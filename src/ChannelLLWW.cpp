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

const char* ChannelLLWW::userid = "1001092";
const char* ChannelLLWW::pwd = "zkcl20128848";

///解析订单查询返回的xml
size_t parse_llww_response(void *buffer, size_t size, size_t count, void *args){
#ifdef DEBUG	
	fprintf(stdout, "%s\n", (char*)buffer);
#endif
	TiXmlDocument doc;
	req_result *ret_info = (req_result*)args;
    if(!doc.Parse((const char*)buffer)){
		seErrLogEx(g_logHandle, "[ChannelLLWW#%lu] [Query] failed@can't parse xml format:%s",pthread_self(), (char*)buffer);
		ret_info->status = -1;
		return size * count;
    }
    TiXmlHandle docHandle(&doc);
    TiXmlElement* status_ele = docHandle.FirstChild("root").FirstChild("result").ToElement();
	if(status_ele == NULL){
		seErrLogEx(g_logHandle, "[ChannelLLWW#%lu] [Query] failed@xml formar err:%s",pthread_self(), (char*)buffer);
		ret_info->status = -1;
		return size * count;
	}
    const char *status = status_ele->GetText();
	int result = atoi(status);
	if(result == 1 || result == 2 || result == 3 || result == 5){
		ret_info->status = result;
	}else{
		status_ele = docHandle.FirstChild("root").FirstChild("errMsg").ToElement();
		if(status_ele == NULL){
			seErrLogEx(g_logHandle, "[ChannelLLWW#%lu] [Query] failed@xml formar err:%s",pthread_self(), (char*)buffer);
			ret_info->status = -1;
			return size * count;
		}
		ret_info->status = result;
		status = status_ele->GetText();
		strncpy(ret_info->msg, status, 256);
	}
	return size * count;
}
///创建订单返回值解析
size_t parse_llww_charge_response(void *buffer, size_t size, size_t count, void *args){
#ifdef DEBUG
	fprintf(stdout, "%s\n", (char*)buffer);
#endif
	TiXmlDocument doc;
	req_result *ret_info = (req_result*)args;
    if(!doc.Parse((const char*)buffer)){
		seErrLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] failed@can't parse xml format:%s",pthread_self(), (char*)buffer);
		ret_info->status = 2;
		return size * count;
    }
    TiXmlHandle docHandle(&doc);
    TiXmlElement* status_ele = docHandle.FirstChild("root").FirstChild("result").ToElement();
	if(status_ele == NULL){
		seErrLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] failed@xml formar err:%s",pthread_self(), (char*)buffer);
		ret_info->status = 2;
		return size * count;
	}
    const char *status = status_ele->GetText();
	int result = atoi(status);
	if(result == 1){
		ret_info->status = 0;
		status_ele = docHandle.FirstChild("root").FirstChild("orderNO").ToElement();
		if(status_ele == NULL){
			seErrLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] failed@xml formar err:%s",pthread_self(), (char*)buffer);
			ret_info->status = 2;
			return size * count;
		}
		status = status_ele->GetText();
		strncpy(ret_info->order_no, status, 128);
	}else{
		status_ele = docHandle.FirstChild("root").FirstChild("errMsg").ToElement();
		if(status_ele == NULL){
			seErrLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] failed@xml formar err:%s",pthread_self(), (char*)buffer);
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
int ChannelLLWW::Charge(TopupInfo *topup_info, string &result)
{
	int ret = 0;
	int retry = 0;
	req_result req_ret;
	int len = 0, siglen = 0;
	char buf[2048] = {0};
	char sigbuf[2048] = {0};
	char md5str[33] = {0};
	char *charge_interface = GlobalConfig::Instance()->p_llww_charge_url;
	str2md5(pwd, strlen(pwd), md5str);
	len += sprintf(buf, "userid=%s&pid=%s&gameaccount=%s&proquantity=1&price=%d&orderno=%s",
			userid, topup_info->pid.c_str(), topup_info->qs_info.customer.c_str(), topup_info->qs_info.value,
			topup_info->qs_info.coopOrderNo.c_str());
	//gameaccount,orderno,pid,price,proquantity,userid,md5(pwd)
	siglen += sprintf(sigbuf, "%s%s%s%d%d%s%s", topup_info->qs_info.customer.c_str(), topup_info->qs_info.coopOrderNo.c_str()
			, topup_info->pid.c_str(), topup_info->qs_info.value, 1,userid, md5str);
	str2md5(sigbuf, siglen, md5str);
	len += sprintf(buf + len, "&sign=%s", md5str);
	buf[len] = '\0';
	seLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] CALL:%s",pthread_self(), buf);
	while(retry <= 5 && ret != 0){
		retry++;
		if(httpclent_perform(charge_interface, buf, parse_llww_charge_response, (void*)&req_ret)){
			if(req_ret.status == 0){
				ret = 0;
				seLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] success@%d:%s",pthread_self(), retry, buf);
				break;
			}else{
				ret = 1;
				seLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] failed@%d:%s-%s",pthread_self(), retry, buf, req_ret.msg);
				continue;
			}
		}else{
			ret = 1;
			seLogEx(g_logHandle, "[ChannelLLWW#%lu] [Charge] failed@%d:%s-httpclent_perform",pthread_self(), retry, buf);
			continue;
		}
	}
	//稳妥起见订单失败后，查询订单是否真的失败
	int query_status = Query(topup_info, result);
	if(query_status == 0 || query_status == 1 || query_status == 2)
		ret = 0;
	return ret;
}

// ret < 0 exception
// ret = 0 success
// ret = 1 underway
// ret = 2 failed
// ret = 3 no such order
int ChannelLLWW::Query(TopupInfo *topup_info, string &result)
{
	int len = 0;
	int siglen = 0;
	char buf[2048] = {0};
	char sigbuf[2048] = {0};
	char md5str[33] = {0};
	req_result req_ret;;
	int ret = 0;
	char* query_interface = GlobalConfig::Instance()->p_llww_query_url;
	///param make
	len += sprintf(buf, "userid=%s&orderno=%s",userid,topup_info->qs_info.coopOrderNo.c_str());
	siglen += sprintf(sigbuf, "%s%s", topup_info->qs_info.coopOrderNo.c_str(), userid);
	str2md5(sigbuf, siglen, md5str);
	len += sprintf(buf + len, "&sign=%s", md5str);
	buf[len] = '\0';
	httpclent_perform(query_interface, buf, parse_llww_response, (void*)&req_ret);
	if(req_ret.status == 2){
		ret = 0;	
	}else if(req_ret.status == 1){
		ret = 1;	
	}else if(req_ret.status == 3){
		ret = 2;
	}else if(req_ret.status == 5){
		ret = 3;
	}else{
		ret = -1;
	}
	return ret;
}

int ChannelLLWW::Balance(TopupInfo *topup_info, double &balance)
{
	seLogEx(g_logHandle, "[ChannelLLWW][Balance] CALL");
	return 0;
}

int ChannelLLWW::AcceptNotify(TopupInfo *topup_info, string &result)
{
	seLogEx(g_logHandle, "[ChannelLLWW][AcceptNotify] CALL");
	return 0;
}
