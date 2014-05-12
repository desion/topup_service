/*************************************************************************
	> File Name: TopupUtils.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 03:14:09 PM CST
 ************************************************************************/

#include "TopupUtils.h"
#include <sys/stat.h>

LOG_HANDLE g_logHandle = NULL;

int get_file_time(char *filename){
	struct stat st = {0};
	if(stat(filename, &st)){
		return -1;
	}
	return (int)st.st_mtime;
}

//so初始化
int so_init(SoBase *so, char *filename, int (reload)(SoBase *so)){
    so->filename = filename;
    so->reload = reload;
    pthread_rwlock_init(&so->lock, NULL);
    return so->reload(so);
}

//so文件名改变了
int so_change(SoBase *so, char *filename){
    so->filename = filename;
    so->reload(so);
    return 0;
}
//检查so是否需要重新加载
int so_check_reload(SoBase *so){
    int ret;
    int time = get_file_time(so->filename);
    if(time <= 0 || time == so->file_time){
       return 0;
    }
    ret = so->reload(so);
    return ret;
}
//加载so文件
int so_load(SoBase *so){
    so->handle = LoadLibrary(so->filename);
    if(so->handle == NULL){
	   slog_write(LL_FATAL, "customer so load failed handle is NULL path:%s", so->filename);
       return -1;
    }
    return 0;
}
//卸载so
int so_free(SoBase *so){
    FreeLibrary(so->handle);
    so->handle = NULL;
    return 0;
}

int so_topup_reload(SoBase *so){
    pthread_rwlock_wrlock(&so->lock);
    if(so->handle != NULL){
        so_free((SoBase*)so);
    }
    if(so_load(so)){
        pthread_rwlock_unlock(&so->lock);
        return -1;
    }
    so->create = (create_t)GetProcAddress(so->handle, "m_create");
    so->destory = (destroy_t)GetProcAddress(so->handle, "m_destroy");
    if (so->create == NULL || so->destory == NULL){
        so_free(so);
        pthread_rwlock_unlock(&so->lock);
        return -1;
    }
    so->file_time = get_file_time(so->filename);
    pthread_rwlock_unlock(&so->lock);
    return 0;
}

int so_customer_reload(SoBase *so){
    pthread_rwlock_wrlock(&so->lock);
    if(so->handle != NULL){
        so_free((SoBase*)so);
    }
    if(so_load(so)){
        pthread_rwlock_unlock(&so->lock);
		slog_write(LL_FATAL, "customer so load failed where so_load");
        return -1;
    }
    so->create = (create_t)GetProcAddress(so->handle, "customer_create");
    so->destory = (destroy_t)GetProcAddress(so->handle, "customer_destroy");
    if (so->create == NULL || so->destory == NULL){
		slog_write(LL_FATAL, "customer so load failed where so->create or so->destory");
        so_free(so);
        pthread_rwlock_unlock(&so->lock);
        return -1;
    }
    so->file_time = get_file_time(so->filename);
    pthread_rwlock_unlock(&so->lock);
    return 0;
}

//根据手机号码和时间生成order no
uint64_t encode_orderno(string phoneNo){
	uint64_t phone_num, orderno;
	sscanf(phoneNo.c_str(), "%lu", &phone_num);
	uint32_t ttime = time(NULL);
	orderno = ttime - 1392906485;
	orderno = orderno << 36;
	orderno += (phone_num - 10000000000);
	return orderno;	
}

//解析order no从中得到手机号和时间戳
void decode_orderno(const uint64_t orderno, uint64_t *phoneno, uint32_t *ttime){
	*phoneno = (orderno & 0xFFFFFFFFF) + 10000000000;
	*ttime = orderno >> 36;
}

int get_time_now(const char* format, string &time_str){
	time_t t = time(NULL);
	char buf[30];
	int ret = strftime(buf, 30, format, localtime(&t));
	time_str = buf;
	return ret;
}

int get_strtime(const uint32_t ts, const char* format, string &time_str){
	char buf[30];
	time_t t = (time_t)ts;
	int ret = strftime(buf, 30, format, localtime(&t));
	time_str = buf;
	return ret;
}
//将其它格式的时间转换为yyyymmddHHmmSS
void trans_time(string &from, string &to){
	char buf[30] = {0};
	strcpy(buf, from.c_str());
	char *p1 = buf;
	char *p2 = buf;
	char ch;
	while(*p1){
		if(isdigit(*p1)){
			ch = *p1;
			*p2 = ch;
			p1++;
			p2++;	
		}else{
			p1++;
		}
	}
	*p2 = '\0';
	to = buf;
}

int calc_time_span(struct timeval *start_time){                                                                                                                                                         
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - start_time->tv_sec) * 1000000 + (now.tv_usec - start_time->tv_usec);
}

void write_err_msg(TopupInfo *topupInfo, vector<string>& errors){
    vector<string>::iterator it = errors.begin();
	for(;it != errors.end();++it){	
		TP_WRITE_ERR(topupInfo, "seqid:%d\t(SelectBestChannel)\t%s", topupInfo->seqid, it->c_str());
	}
}

/***
 *  * 对字符串做MD5加密
 *   * */
/*
int str2md5(const char* src, int len,char *md5str){
    if(src == NULL)
        return 1;
    if(len <= 0)
        return 2;
    if(md5str == NULL)
        return 3;
    unsigned char md[16];
    unsigned char* tmp = (unsigned char*)malloc(len * sizeof(unsigned char));
    if(tmp == NULL)
        return 4;
	memcpy(tmp, src, len);
	int i;
	MD5(tmp ,len ,md);
	for (i = 0; i < 16; i++){
	    sprintf(md5str + i*2,"%2.2x",md[i]);
	}
	free(tmp);
	return 0;
}*/
void serialize_topupinfo(TopupInfo* topup_info, string &strout){
	if(topup_info == NULL)
		return;
	Json::Value root;
	Json::Value channel_array;
	root["coopId"] = topup_info->qs_info.coopId;						//商家编号
	root["tbOrderNo"] = topup_info->qs_info.tbOrderNo;					//淘宝的订单号
	root["coopOrderNo"] = topup_info->qs_info.coopOrderNo;				//系统生成订单号
	root["cardId"] = topup_info->qs_info.cardId;						//充值卡商品编号
	root["cardNum"] = topup_info->qs_info.cardNum;						//充值卡数量
	root["customer"] = topup_info->qs_info.customer;					//手机号码
	root["sum"] = topup_info->qs_info.sum;								//本次充值总金额
	root["tbOrderSnap"] = topup_info->qs_info.tbOrderSnap;				//商品信息快照
	root["notifyUrl"] = topup_info->qs_info.notifyUrl;					//异同通知地址
	root["price"] = topup_info->qs_info.price;
	root["value"] = topup_info->qs_info.value;
	root["op"] = topup_info->qs_info.op;
	root["province"] = topup_info->qs_info.province;
	root["status"] = topup_info->status;
	root["interfaceName"] = topup_info->interfaceName;
	root["channelId"] = topup_info->channelId;
	root["channel_discount"] = topup_info->channel_discount;
	root["creteTime"] = (uint32_t)time(NULL);
	root["updateTime"] = (uint32_t)time(NULL);
	root["last_op_time"] = topup_info->last_op_time;
	root["userid"] = topup_info->userid;
	//strout = root.toStyledString();
	vector<ChannelInfo>::iterator iter = topup_info->channels.begin();
	for(; iter != topup_info->channels.end(); ++iter){
		Json::Value channel_entity;
		//渠道ID
		channel_entity["channelId"] = iter->channelId;
		//渠道名称
		channel_entity["channelName"] = iter->channelName;
		//渠道简称
		channel_entity["sname"] = iter->sname;
		//折扣
		channel_entity["discount"] = iter->discount;
		//接口标识
		channel_entity["interfaceName"] = iter->interfaceName;
		//优先级
		channel_entity["priority"] = iter->priority;
		//重试次数
		channel_entity["repeat"] = iter->repeat;
		channel_entity["pid"] = iter->pid;
		channel_array.append(channel_entity);
	}
	root["channels"] = channel_array;
	Json::FastWriter writer;
	strout = writer.write(root);
}

void deserialize_topupinfo(const string& json, TopupInfo* topup_info){
	if(topup_info == NULL)
		return;
	Json::Value root;
	Json::Reader reader;
	if(reader.parse(json, root)){
		if(!root["coopId"].isNull())
            topup_info->qs_info.coopId = root["coopId"].asString();							//商家编号
        if(!root["tbOrderNo"].isNull())
            topup_info->qs_info.tbOrderNo = root["tbOrderNo"].asString();                  //淘宝的订单号
        if(!root["coopOrderNo"].isNull())
            topup_info->qs_info.coopOrderNo = root["coopOrderNo"].asString();              //系统生成订单号
        if(!root["cardId"].isNull())
            topup_info->qs_info.cardId = root["cardId"].asString();                        //充值卡商品编号
        if(!root["cardNum"].isNull())
            topup_info->qs_info.cardNum = root["cardNum"].asInt();						   //充值卡数量
        if(!root["customer"].isNull())
            topup_info->qs_info.customer = root["customer"].asString();                    //手机号码
        if(!root["sum"].isNull())
            topup_info->qs_info.sum = root["sum"].asDouble();                              //本次充值总金额
        if(!root["tbOrderSnap"].isNull())
            topup_info->qs_info.tbOrderSnap = root["tbOrderSnap"].asString();              //商品信息快照
        if(!root["notifyUrl"].isNull())
            topup_info->qs_info.notifyUrl = root["notifyUrl"].asString();                   //异同通知地址
        if(!root["price"].isNull())
            topup_info->qs_info.price = root["price"].asDouble(); 
        if(!root["value"].isNull())
            topup_info->qs_info.value = root["value"].asInt(); 
        if(!root["op"].isNull())
            topup_info->qs_info.op = root["op"].asInt(); 
        if(!root["province"].isNull())
            topup_info->qs_info.province = root["province"].asInt(); 
        if(!root["status"].isNull())
            topup_info->status = (OrderStatus)root["status"].asInt(); 
        if(!root["creteTime"].isNull())
            topup_info->create_time = root["creteTime"].asInt(); 
        if(!root["interfaceName"].isNull())
            topup_info->interfaceName = root["interfaceName"].asString(); 
        if(!root["channelId"].isNull())
            topup_info->channelId = root["channelId"].asInt(); 
        if(!root["channel_discount"].isNull())
            topup_info->channel_discount = root["channel_discount"].asDouble(); 
        if(!root["last_op_time"].isNull())
            topup_info->last_op_time = root["last_op_time"].asInt(); 
        if(!root["userid"].isNull())
            topup_info->userid = root["userid"].asString(); 
		if(!root["channels"].isNull()){
			Json::Value channelArray = root["channels"];
			for(int i = 0; i < channelArray.size(); i++){
				ChannelInfo channel_info;
				//渠道ID
				if(channelArray[i].isMember("channelId"))
					channel_info.channelId = channelArray[i]["channelId"].asInt();
				//渠道名称
				if(channelArray[i].isMember("channelName"))
					channel_info.channelName = channelArray[i]["channelName"].asString();
				//渠道简称
				if(channelArray[i].isMember("sname"))
					channel_info.sname = channelArray[i]["sname"].asString();
				//折扣
				if(channelArray[i].isMember("discount"))
					channel_info.discount = channelArray[i]["discount"].asDouble();
				//接口标识
				if(channelArray[i].isMember("interfaceName"))
					channel_info.interfaceName = channelArray[i]["interfaceName"].asString();
				//优先级
				if(channelArray[i].isMember("priority"))
					channel_info.priority = channelArray[i]["priority"].asInt();
				//重试次数
				if(channelArray[i].isMember("repeat"))
					channel_info.repeat = channelArray[i]["repeat"].asInt();
				if(channelArray[i].isMember("pid"))
					channel_info.pid = channelArray[i]["pid"].asString();

				topup_info->channels.push_back(channel_info);
			}
		}
	}
}
