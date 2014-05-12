/*************************************************************************
	> File Name: TopupCustomer.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 04:16:03 PM CST
 ************************************************************************/

#include "TopupCustomer.h"
#include "TopupUtils.h"
#include "HttpClient.h"
#include "GlobalConfig.h"
using namespace std;
using namespace  ::topupinterface;

extern LOG_HANDLE g_logHandle;

extern TopupServer *P_TPServer;

TopupCustomer::TopupCustomer(){
}

TopupCustomer::~TopupCustomer(){
//	P_TPServer->CallLog(m_topup_info);
}


/**初始化连接**/
int TopupCustomer::Init(TopupInfo* topup_info)
{
	m_topup_info = topup_info;
	m_conn = topup_info->conn;
	topup_info->log_len = 0;
	topup_info->err_log_len = 0;
	return 0;
}

void TopupCustomer::Log()
{
	//P_TPServer->CallLog(m_topup_info);
}


#define LAKE_PARAM_ERR		"0101"
#define NO_PRODUCT_ERR		"0305"
#define PRODUCT_MAIN_ERR	"0304"
#define SIGN_NOMATCH_ERR	"0102"
#define REQUEST_FAILED_ERR	"0104"
#define NO_SUCH_USER		"0105"
#define BALANCE_FAIL		"0106"
#define BALANCE_NOT_ENOUGH	"0107"

//处理FCGI的请求，根据请求的URI判断如何处理
int TopupCustomer::HandleRequest(const TopupRequest& request, string &result){
	TP_WRITE_LOG(m_topup_info, "#%d\t[%s]\t%s\t%s\t%d",m_topup_info->seqid, request.uri.c_str()
			,request.query.c_str(), request.checksum.c_str(), request.itimestamp);
	const char *params = request.query.c_str();
	const char *uri = request.uri.c_str();
	uint32_t request_time = request.itimestamp;
	const char *md5str = request.checksum.c_str();
	const char *m_interface = NULL;
	//解析post参数
	parse_params(params, &map_entitys);
	//解析query参数，并封装结构
	parse_query(params, m_topup_info);
	if(uri != NULL){
		m_interface = strrchr(uri, '/');
		if(m_interface == NULL){
			//TODO 输出错误
		}
	}
	//根据调用的URI参数判断调用的相应接口
	if(strcmp(m_interface + 1, "pay") == 0){
		//调用充值接口
		CustomerCharge(result);
	}else if(strcmp(m_interface + 1, "query") == 0){
		//调用查询订单查询接口
		CustomerQuery(result);
	}else if(strcmp(m_interface, "balance") == 0){
		//调用余额查询接口
	}
#ifdef DEBUG
	map<string, string>::iterator it = map_entitys.begin();
	for(;it != map_entitys.end(); ++it){
			printf("key:%s\tvalue:%s\n", it->first.c_str(), it->second.c_str());
	}
#endif
	return 0;
}

int TopupCustomer::Notify(){
	return 0;
}

int TopupCustomer::CustomerCharge(string &response){
	//TODO 验证参数的正确性
	
	map<string, string>::iterator it;

	if(m_topup_info->qs_info.coopId.empty()){
		MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) NO coopId %s", LAKE_PARAM_ERR);
		return 1;
	}
	if(m_topup_info->qs_info.tbOrderNo.empty()){
		MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) NO tbOrderNo %s", LAKE_PARAM_ERR);
		return 1;
	}
	if(m_topup_info->qs_info.cardId.empty()){
		MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) NO cardId %s", LAKE_PARAM_ERR);
		return 1;
	}
	if(m_topup_info->qs_info.cardNum == 0){
		MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) NO cardNum %s", LAKE_PARAM_ERR);
		return 1;
	}
	if(m_topup_info->qs_info.customer.empty()){
		MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) NO customer %s", LAKE_PARAM_ERR);
		return 1;
	}
	if(m_topup_info->qs_info.sum == 0.){
		MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) NO sum %s", LAKE_PARAM_ERR);
		return 1;
	}
	if(m_topup_info->qs_info.sign.empty()){
		MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) NO sign %s", LAKE_PARAM_ERR);
		return 1;
	}
	//验证签名
	if(!CheckSign()){
		MakeErrReplay(SIGN_NOMATCH_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) sign error %s", SIGN_NOMATCH_ERR);
		return 4;
	}
	//选择正确的产品，所有产品信息加入缓存，商品更新发送通知，重新加载缓存
	int check_product = CheckProduct();
	TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) PRODUCT:%d v:%d o:%d p:%d",check_product, m_topup_info->qs_info.value,
			        m_topup_info->qs_info.op ,m_topup_info->qs_info.province);
	if(check_product == 2){
		MakeErrReplay(NO_PRODUCT_ERR, SORDER_FAILED, response);
		return 2;
	}else if(check_product == 1){
		MakeErrReplay(PRODUCT_MAIN_ERR, SORDER_FAILED, response);
		return 3;
	}
	//下游用户充值，需要检查余额信息
	int check_balance = CheckBalance();
	if(check_balance == 1){
		//余额不足
		MakeErrReplay(BALANCE_NOT_ENOUGH, SORDER_FAILED, response);
	}else if(check_balance == 2){
		//没找到相应的用户信息
		MakeErrReplay(NO_SUCH_USER, SORDER_FAILED, response);
	}else if(check_balance < 0){
		//数据库操作错误
		MakeErrReplay(BALANCE_FAIL, SORDER_FAILED, response);
	}
	//TODO 选择最优的渠道，渠道信息同样加入缓存，信息更新，重新加载
	int selectChannel = SelectBestChannel();
	if(selectChannel <= 0){
		MakeErrReplay(PRODUCT_MAIN_ERR, SORDER_FAILED, response);
		return 3;
	}
	TP_WRITE_LOG(m_topup_info, "\tCHANNEL:%d id:%d name:%s dis:%f pri:%d int:%s",selectChannel,
		    m_topup_info->channels[0].channelId,m_topup_info->channels[0].sname.c_str(),m_topup_info->channels[0].discount,
			m_topup_info->channels[0].priority, m_topup_info->channels[0].interfaceName.c_str());
	//建立订单，订单创建采用append模式，快速，采用按天分表模式，保留一个月的数据
	int create_status = CreateTmallOrder();
	if(create_status < 0){
		MakeErrReplay(PRODUCT_MAIN_ERR, SORDER_FAILED, response);
		TP_WRITE_LOG(m_topup_info, "\t(TmallCharge) fail to create order %s\terr code:%d", PRODUCT_MAIN_ERR, create_status);
		return 5;	
	}
	//TODO 返回结果
	TP_WRITE_LOG(m_topup_info, "\t(TmallCharge) success create order");
	MakeSuccessReplay(SUNDERWAY, response);
	
	//设置通知状态
	m_topup_info->notify = 0;
	//返回结果
	TP_WRITE_LOG(m_topup_info, "\t(CustomerCharge) success create order");
	MakeSuccessReplay(SUNDERWAY, response);
	//设置通知状态
	m_topup_info->notify = 0;
	//*注.至此只是在系统创建订单，并没有先上游发送请求
	return 0;
}


///查询接口，用于查询订单
int TopupCustomer::CustomerQuery(string &response){
	//验证参数的正确性
	if(m_topup_info->qs_info.coopId.empty()){
        MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
        TP_WRITE_LOG(m_topup_info, "\t(CustomerQuery) NO coopId %s", LAKE_PARAM_ERR);
        return 1;
    }
    if(m_topup_info->qs_info.tbOrderNo.empty()){
        MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
        TP_WRITE_LOG(m_topup_info, "\t(CustomerQuery) NO tbOrderNo %s", LAKE_PARAM_ERR);
        return 1;
    }
	if(m_topup_info->qs_info.sign.empty()){
        MakeErrReplay(LAKE_PARAM_ERR, SORDER_FAILED, response);
        TP_WRITE_LOG(m_topup_info, "\t(CustomerQuery) NO sign %s", LAKE_PARAM_ERR);
        return 1;
    }
	//验证签名
	if(!CheckSign()){
        MakeErrReplay(SIGN_NOMATCH_ERR, SORDER_FAILED, response);
        TP_WRITE_LOG(m_topup_info, "\t(CustomerQuery) sign error %s", SIGN_NOMATCH_ERR);
        return 4;
    }		
	//查询订单
	int ret = QueryOrder();
   	if(ret == 1){
		switch(m_topup_info->status){
			case UNDERWAY:
		        MakeSuccessReplay(SUNDERWAY, response); 
		        break;
		    case FAILED:
		        MakeSuccessReplay(SFAILED, response);   
		        break;
		    case SUCCESS:
		        MakeSuccessReplay(SSUCCESS, response);
				break;
		    default:
		        MakeErrReplay(REQUEST_FAILED_ERR, SREQUEST_FAILED, response);
		        break;
		}
	}else{
		MakeErrReplay(REQUEST_FAILED_ERR, SREQUEST_FAILED, response);
	}
	//返回结果
	return 0;
}

//回调接口，发送回调请求，下游用户实现，该方法只发送回调请求
int TopupCustomer::CustomerNotify(string &response){
	int retry = 5;				//重试次数
	int notify_status = 0;		//通知状态
	while(retry > 0 && notify_status == 0){
		retry--;
		if(m_topup_info->qs_info.coopId.empty()){
			string json_data;
			serialize_topupinfo(m_topup_info, json_data);
			seLogEx(g_logHandle,"%s\t(CustomerNotify) NO coopId %s",json_data.c_str(), LAKE_PARAM_ERR);
			return 1;
		}
		if(m_topup_info->qs_info.tbOrderNo.empty()){
			string json_data;
			serialize_topupinfo(m_topup_info, json_data);
			seLogEx(g_logHandle,"%s\t(CustomerNotify) NO tbOrderNo %s",json_data.c_str(), LAKE_PARAM_ERR);
			return 1;
		}
		if(m_topup_info->qs_info.coopOrderNo.empty()){
			string json_data;
			serialize_topupinfo(m_topup_info, json_data);
			seLogEx(g_logHandle,"%s\t(CustomerNotify) no coopOrderNo %s",json_data.c_str(), LAKE_PARAM_ERR);
			return 1;
		}
		/*
		if(m_topup_info->qs_info.coopOrderStatus.empty()){
			TP_WRITE_LOG(m_topup_info, "\t(CustomerNotify) NO tbOrderNo %s", LAKE_PARAM_ERR);
			return 1;
		}
		*/
		if(m_topup_info->qs_info.notifyUrl.empty()){
			string json_data;
			serialize_topupinfo(m_topup_info, json_data);
			seLogEx(g_logHandle,"%s\t(CustomerNotify) no notifyUrl %s",json_data.c_str(), LAKE_PARAM_ERR);
			return 1;
		}
		//向天猫或下游订购用户发送回调请求
		char buf[2048] = {0};
		char snap_encode[256] = {0};
		char md5str[33] = {0};
		int len = 0;
		url_encode(m_topup_info->qs_info.tbOrderSnap.c_str(), m_topup_info->qs_info.tbOrderSnap.length(), snap_encode, 256);
		//订单为成功状态的情况
		if(m_topup_info->status == SUCCESS){
			string ts;
			len += sprintf(buf,
					"coopId=%s&tbOrderNo=%s&coopOrderNo=%s&coopOrderStatus=SUCCESS&coopOrderSnap=%s&coopOrderSuccessTime=%s",
					m_topup_info->qs_info.coopId.c_str(),m_topup_info->qs_info.tbOrderNo.c_str(),
				   	m_topup_info->qs_info.coopOrderNo.c_str(), m_topup_info->qs_info.tbOrderSnap.c_str(),ts.c_str());
			buf[len] = '\0';
			if(url_signature(buf ,GlobalConfig::Instance()->private_key , md5str) != 0){
				string json_data;
				serialize_topupinfo(m_topup_info, json_data);
				seLogEx(g_logHandle,"%s\t(CustomerNotify) url_signature failed",json_data.c_str());
				continue;
			}
			len = 0;
			len += sprintf(buf,
				"coopId=%s&tbOrderNo=%s&coopOrderNo=%s&coopOrderStatus=SUCCESS&coopOrderSnap=%s&coopOrderSuccessTime=%s&sign=%s",
				m_topup_info->qs_info.coopId.c_str(), m_topup_info->qs_info.tbOrderNo.c_str(),
				m_topup_info->qs_info.coopOrderNo.c_str(), snap_encode, ts.c_str(), md5str);
			buf[len] = '\0';
			if(!httpclent_perform(m_topup_info->qs_info.notifyUrl.c_str(), buf, &parse_tmall_response, (void*)(&notify_status))){
				//log
				string json_data;
				serialize_topupinfo(m_topup_info, json_data);
				seLogEx(g_logHandle,"%s\t(CustomerNotify) httpclent_perform failed",json_data.c_str());
				notify_status = 0;
				continue;
			}
		//订单状态为失败的情况下
		}else if(m_topup_info->status == FAILED){
			len += sprintf(buf,
				"coopId=%s&tbOrderNo=%s&coopOrderNo=%s&coopOrderStatus=FAILED&failedCode=%s",
				m_topup_info->qs_info.coopId.c_str(),m_topup_info->qs_info.tbOrderNo.c_str(),
			   	m_topup_info->qs_info.coopOrderNo.c_str(), "0501");
			buf[len] = '\0';
			if(url_signature(buf ,GlobalConfig::Instance()->private_key , md5str) != 0){
				TP_WRITE_LOG(m_topup_info, "\t(TmallNotify) url_signature failed");
				continue;	
			}
			len = 0;
			len += sprintf(buf,
					"coopId=%s&tbOrderNo=%s&coopOrderNo=%s&coopOrderStatus=FAILED&failedCode=%s&sign=%s",
					m_topup_info->qs_info.coopId.c_str(),m_topup_info->qs_info.tbOrderNo.c_str(),
				   	m_topup_info->qs_info.coopOrderNo.c_str(),  "0501", md5str);
			buf[len] = '\0';
			if(!httpclent_perform(m_topup_info->qs_info.notifyUrl.c_str(), buf, &parse_tmall_response, (void*)(&notify_status))){
				//log
				string json_data;
				serialize_topupinfo(m_topup_info, json_data);
				seLogEx(g_logHandle,"%s\t(TmallNotify) httpclent_perform failed",json_data.c_str());
				notify_status = 0;
				continue;
			}
		}else{
			string json_data;
			serialize_topupinfo(m_topup_info, json_data);
			seLogEx(g_logHandle,"%s\t(CustomerNotify) the order status is not success nor fail",json_data.c_str());
			return 0;
		}
	}
	//超过5次没有通知成功，直接放弃通知
	if(notify_status == 0){
		//更新通知状态为通知失败
		m_topup_info->notify = NOTIFY_FAIL;
	}else{
		m_topup_info->notify = notify_status;
	}
	//更新数据库通知状态
	UpdateStatus();
	return 0;
}


///用于处理下游订购用户的查询余额请求
int TopupCustomer::GetBalance(string &response){
	if(m_topup_info->qs_info.coopId.empty()){
		MakeBalanceReplay(LAKE_PARAM_ERR, 0.0, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerBalance) NO coopId %s", LAKE_PARAM_ERR);
		return 1;
	}
	if(m_topup_info->qs_info.tbOrderNo.empty()){
		MakeBalanceReplay(LAKE_PARAM_ERR, 0.0, response);
		TP_WRITE_LOG(m_topup_info, "\t(CustomerBalance) NO tbOrderNo %s", LAKE_PARAM_ERR);
		return 1;
	}
	//验证签名
	if(!CheckSign()){
        MakeBalanceReplay(SIGN_NOMATCH_ERR, 0.0, response);
        TP_WRITE_LOG(m_topup_info, "\t(CustomerBalance) sign error %s", SIGN_NOMATCH_ERR);
        return 4;
	}
	//查询数据库，取得余额信息
	ChargeBusiness *chargeBusiness = new ChargeBusiness();
    if(!chargeBusiness){
		MakeBalanceReplay(BALANCE_FAIL, 0.0, response);	
		return -1;
    }
    chargeBusiness->Init(m_conn);
	double balance = 0.0;
    int ret = chargeBusiness->GetBalance(m_topup_info->qs_info.coopId, balance);
	if(ret == 0){
        MakeBalanceReplay(NULL, balance, response);
	}else if(ret == 1){
		MakeBalanceReplay(NO_SUCH_USER, 0.0, response);	
	}else{
		MakeBalanceReplay(BALANCE_FAIL, 0.0, response);	
	}
    delete chargeBusiness;
	//返回余额信息
	return 0;
}


int TopupCustomer::UpdateStatus(){
	ChargeBusiness *chargeBusiness = new ChargeBusiness();
    if(!chargeBusiness){
        return -1;
    }
    chargeBusiness->Init(m_conn);
    int ret = chargeBusiness->UpdateOrderStatus(m_topup_info);
    delete chargeBusiness;
    return ret;	
}

//返回错误信息
int TopupCustomer::MakeErrReplay(const char* errCode,const char* status, string &result){
	char buf[2048] = {0};
	int len = 0;
	len += sprintf(buf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	len += sprintf(buf + len, "<response>");
	len += sprintf(buf + len, "<tbOrderNo>%s</tbOrderNo>", m_topup_info->qs_info.tbOrderNo.c_str());
	len += sprintf(buf + len, "<coopOrderNo>%s</coopOrderNo>", m_topup_info->qs_info.coopOrderNo.c_str());
	len += sprintf(buf + len, "%s", status);
	len += sprintf(buf + len, "<coopOrderSnap>%s</coopOrderSnap>", m_topup_info->qs_info.tbOrderSnap.c_str());
	len += sprintf(buf + len, "<failedCode>%s</failedCode>", errCode);
	len += sprintf(buf + len, "<failedReason>%s</failedReason>", GlobalConfig::Instance()->errors[string(errCode)].c_str());
	len += sprintf(buf + len, "</response>");
	result = string(buf);
	return len;
}
//返回正确的信息
int TopupCustomer::MakeSuccessReplay(const char* status, string &result){
	char buf[2048] = {0};
	int len = 0;
	len += sprintf(buf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	len += sprintf(buf + len, "<response>");
	len += sprintf(buf + len, "<tbOrderNo>%s</tbOrderNo>", m_topup_info->qs_info.tbOrderNo.c_str());
	len += sprintf(buf + len, "<coopOrderNo>%s</coopOrderNo>", m_topup_info->qs_info.coopOrderNo.c_str());
	len += sprintf(buf + len, "%s", status);
	len += sprintf(buf + len, "<coopOrderSnap>%s</coopOrderSnap>", m_topup_info->qs_info.tbOrderSnap.c_str());
	len += sprintf(buf + len, "<coopOrderSuccessTime>xxx</coopOrderSuccessTime>");
	len += sprintf(buf + len, "</response>");
	result = buf;
	return len;
}

int TopupCustomer::MakeBalanceReplay(const char* errCode, double balance, string &result){
	char buf[2048] = {0};
	int len = 0;
	len += sprintf(buf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
	len += sprintf(buf + len, "<response>");
	if(errCode == NULL){
		len += sprintf(buf + len, "<userId>%s</userId>", m_topup_info->qs_info.coopId.c_str());
		len += sprintf(buf + len, "<balance>%f</balance>", balance);
	}else{
		len += sprintf(buf + len, "<failedCode>%s</failedCode>", errCode);
	    len += sprintf(buf + len, "<failedReason>%s</failedReason>", GlobalConfig::Instance()->errors[string(errCode)].c_str());
	}
	len += sprintf(buf + len, "</response>");
	result = buf;
	return len;
}

int TopupCustomer::CheckProduct(){

	ChargeBusiness *chargeBusiness = new ChargeBusiness();
	chargeBusiness->Init(m_conn);
	Product m_product;
	int ret =  chargeBusiness->GetTmallProduct(m_topup_info->qs_info.cardId, m_product);
	if(ret == 0){
		m_topup_info->qs_info.value = m_product.price;
		m_topup_info->qs_info.op = m_product.op;
		m_topup_info->qs_info.province = m_product.provinceId;
	}
	delete chargeBusiness;
	return ret;
}

int TopupCustomer::CheckBalance(){
	ChargeBusiness *chargeBusiness = new ChargeBusiness();
	chargeBusiness->Init(m_conn);
	Product m_product;
	int ret =  chargeBusiness->CheckAndBalance(m_topup_info);
	delete chargeBusiness;
	return ret;
}

int TopupCustomer::SelectBestChannel(){
	ChargeBusiness *chargeBusiness = new ChargeBusiness();
	chargeBusiness->Init(m_conn);
	int channel_num = chargeBusiness->SelectBestChannel(m_topup_info->qs_info.value, m_topup_info->qs_info.province,
		   	m_topup_info->qs_info.op,m_topup_info->channels);	
#ifdef DEBUG
	printf("SelectBestChannel.....num:%d\n", channel_num);
#endif
	if(chargeBusiness->HasError()){
		vector<string> errors = chargeBusiness->GetErrors();
		write_err_msg(m_topup_info, errors);
	}
	delete chargeBusiness;
	if(channel_num == 0){
		return 0;	
	}else if(channel_num == 1){
		return 1;	
	}else{
		sort(m_channels.begin(), m_channels.end(), ChannelRank);	
		return channel_num;
	}
	return 0;
}

int TopupCustomer::CreateTmallOrder(){
	printf("CreateTmallOrder.....\n");
	ChargeBusiness *chargeBusiness = new ChargeBusiness();
	chargeBusiness->Init(m_conn);
	int ret = chargeBusiness->CreateTmallOrder(m_topup_info, m_topup_info->channels[0]);
	delete chargeBusiness;
	return ret;
}

int TopupCustomer::QueryOrder(){
	ChargeBusiness *chargeBusiness = new ChargeBusiness();
	chargeBusiness->Init(m_conn);
	int ret = chargeBusiness->QueryOrder(m_topup_info);
	delete chargeBusiness;
	return ret;
}

bool TopupCustomer::CheckSign(){
	char md5[33] = {0};
    char signStr[2048] = {0};
    int len = 0;
    map<string, string>::iterator it = map_entitys.begin();
	string sign_str;
    for(;it != map_entitys.end(); ++it){
	    if(strcmp("sign", it->first.c_str()) == 0){
			sign_str = it->second;
			continue;
		}
		len += sprintf(signStr + len, "%s%s", it->first.c_str(), it->second.c_str());
	}
    len += sprintf(signStr + len, "%s", GlobalConfig::Instance()->private_key);
    str2md5(signStr,len, md5);
	if(sign_str.empty()){
		return false;
	}
	
	if(strcmp(sign_str.c_str(), md5) != 0){
		return false;
	}
	return true;	
}

//动态链接库调用接口，用于创建相应实例
extern "C" TopupBase* customer_create() {
	    return new TopupCustomer;
}

//动态链接库调用接口，用于销毁相应的实例,可不可以通过得到的指针直接销毁
extern "C" void customer_destroy(TopupBase* p) {
	    delete p;
}
