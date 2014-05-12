/*************************************************************************
	> File Name: ChannelSLS.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 15 Feb 2014 09:57:19 AM CST
 ************************************************************************/

#include <iostream>
#include "Channel.hpp"
#include "HttpClient.h"
#include "GlobalConfig.h"
#include <tinyxml/tinyxml.h>
#include <tinyxml/tinystr.h>

using namespace std;
extern LOG_HANDLE g_logHandle;

const char* ChannelSLS::key = "shoulashou0571";                                                                                                                                                      
const char* ChannelSLS::userid = "84001";
const char* ChannelSLS::pwd = "123456";
string ChannelSLS::GetErrMsg(string code){
	map<string, string>::iterator iter;
	if((iter = errors.find(code)) != errors.end()){
		return iter->second;
	}else{
		return "no such code:" + code;
	}
}


size_t parse_response(void *buffer, size_t size, size_t count, void *args){
#ifdef DEBUG	
	fprintf(stdout, "%s\n", (char*)buffer);
#endif
	char *code = (char*)args;
	strncpy(code, (char*)buffer, 3);
	return size * count;
}

size_t parse_sls_charge_response(void *buffer, size_t size, size_t count, void *args){
#ifdef DEBUG
	fprintf(stdout, "%s\n", (char*)buffer);
#endif
	TiXmlDocument doc;
	req_result *ret_info = (req_result*)args;
    if(!doc.Parse((const char*)buffer)){
		seErrLogEx(g_logHandle, "[ChanneSLS#%lu] [Charge] failed@can't parse xml format:%s",pthread_self(), (char*)buffer);
		ret_info->status = 2;
		return size * count;
    }
    TiXmlHandle docHandle(&doc);
    TiXmlElement* status_ele = docHandle.FirstChild("Response").FirstChild("Result").ToElement();
	if(status_ele == NULL){
		seErrLogEx(g_logHandle, "[ChannelSLS#%lu] [Charge] failed@xml formar err:%s",pthread_self(), (char*)buffer);
		ret_info->status = 2;
		return size * count;
	}
    const char *status = status_ele->GetText();
	int result = atoi(status);
	//result==1提交成功
	if(result == 1){
		ret_info->status = 0;
		seErrLogEx(g_logHandle, "[ChannelSLS#%lu] [Charge] sucess xml:%s",pthread_self(), (char*)buffer);
	}else{
		status_ele = docHandle.FirstChild("Response").FirstChild("ErrCode").ToElement();
		if(status_ele == NULL){
			seErrLogEx(g_logHandle, "[ChannelSLS#%lu] [Charge] failed@xml formar err:%s",pthread_self(), (char*)buffer);
			ret_info->status = 2;
			return size * count;
		}
		ret_info->status = 1;
		status = status_ele->GetText();
		strncpy(ret_info->msg, status, 256);
	}
	return size * count;
}

//ret = 0 success
//ret = 1 failed
int ChannelSLS::Charge(TopupInfo *topup_info, string &result)
{
	int ret = 1;
	int retry = 0;
	req_result req_ret;
	int len = 0, siglen = 0;
	char buf[2048] = {0};
	char sigbuf[2048] = {0};
	char md5str[33] = {0};
	char *charge_interface = GlobalConfig::Instance()->p_sls_charge_url;
	str2md5(pwd, strlen(pwd), md5str);
	len += sprintf(buf, "UserID=%s&Phone=%s&Money=%d&OrderNo=%s&IsLocalPay=%d&PayPwd=%s",
			userid, topup_info->qs_info.customer.c_str(), topup_info->qs_info.value, topup_info->qs_info.coopOrderNo.c_str(),
			0, md5str);
	siglen += sprintf(sigbuf, "%s%s%s%d%s%d%s%s", key, userid, topup_info->qs_info.customer.c_str(), 
			topup_info->qs_info.value, topup_info->qs_info.coopOrderNo.c_str(), 0, md5str, key);
	str2md5(sigbuf, siglen, md5str);
	len += sprintf(buf + len, "&Sign=%s", md5str);
	buf[len] = '\0';
	seLogEx(g_logHandle, "[ChannelSLS#%lu] [Charge] CALL:%s",pthread_self(), buf);
	while(retry <= 5 && ret != 0){
		retry++;
		if(httpclent_perform(charge_interface, buf, parse_sls_charge_response, (void*)&req_ret)){
			if(req_ret.status == 0){
				ret = 0;
				seLogEx(g_logHandle, "[ChannelSLS#%lu] [Charge] success@%d:%s", pthread_self(), retry, buf);
				break;
			}else{
				ret = 1;
				seLogEx(g_logHandle, "[ChannelSLS#%lu] [Charge] failed@%d:%s-%s",pthread_self(), retry, buf, req_ret.msg);
				continue;
			}
		}else{
			ret = 1;
			seLogEx(g_logHandle, "[ChannelSLS#%lu] [Charge] failed@%d:%s-httpclent_perform",pthread_self(), retry, buf);
			continue;
		}
	}
	return ret;
}

// ret < 0 exception
// ret = 0 success
// ret = 1 underway
// ret = 2 failed
// ret = 3 no such order
int ChannelSLS::Query(TopupInfo *topup_info, string &result)
{
	TP_WRITE_LOG(topup_info, "[ChannelSLS][Query] CALL");
	int len = 0;
	int siglen = 0;
	char buf[2048] = {0};
	char sigbuf[2048] = {0};
	char md5str[33] = {0};
	char ret_code[5] = {0};
	int ret = 0;
	char* query_interface = GlobalConfig::Instance()->p_sls_query_url;
	///param make
	len += sprintf(buf,"UserID=%s&OrderNo=%s",userid,topup_info->qs_info.tbOrderNo.c_str());
	siglen += sprintf(sigbuf, "%s%s%s%s", key, userid, topup_info->qs_info.tbOrderNo.c_str(), key);
	str2md5(sigbuf, siglen, md5str);
	len += sprintf(buf + len, "&Sign=%s", md5str);
	buf[len] = '\0';
	TP_WRITE_LOG(topup_info, "\t%s", buf);
	httpclent_perform(query_interface, buf, parse_response, (void*)&ret_code);
	if(strcmp(ret_code, "601") == 0){
		ret = 0;	
	}else if(strcmp(ret_code, "602") == 0){
		ret = 2;	
	}else if(strcmp(ret_code, "603") == 0){
		ret = 1;
	}else if(strcmp(ret_code, "600") == 0){
		ret = 3;
	}else{
		ret = -1;
	}
	return ret;
}

int ChannelSLS::Balance(TopupInfo *topup_info, double &balance)
{
	return 0;
}

int ChannelSLS::AcceptNotify(TopupInfo *topup_info, string &result)
{
	TP_WRITE_LOG(topup_info, "[ChannelSLS][AcceptNotify] CALL");
	return 0;
}
