/*************************************************************************
	> File Name: TopupService.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Mon 20 Jan 2014 03:48:25 PM CST
 ************************************************************************/

#include <iostream>
#include <sstream>
#include "TopupService.h"
#include "TopupUtils.h"
#include "TopupZkcl.h"
#include "TopupServer.h"
#include "GlobalConfig.h"
#include "boost/shared_ptr.hpp"

using namespace boost;
using namespace std;

extern LOG_HANDLE g_logHandle;

//for tmall
#define TMALL_API 		"/tmall/"
//for zkcl internal
#define ZKCL_API 		"/zkcl/"
//for other customer
#define CUSTOMER_API 	"/customer/"
//for customer query result
#define RES_API 		"/result/"

//handle thrift request
void TopupService::SendRequest(std::string& _return,const TopupRequest& request){
	TopupBase *topupBase;
	SoBase *so_base;
	TopupInfo *tpInfo = new TopupInfo();
	assert(tpInfo != NULL);
	//through uri specify interface
	const char* uri = request.uri.c_str();
	P_TPServer->SetSeqId(&tpInfo->seqid); 
	slog_write(LL_TRACE, "#%d\t[%s]\t%s\t%s\t%d",tpInfo->seqid, request.uri.c_str()
            ,request.query.c_str(), request.checksum.c_str(), request.itimestamp);
	if(strncmp(TMALL_API, uri, 7) == 0){		//tmall interface uri
		so_base = &P_TPServer->topup_so;
		//create charge up instance by so
		DLLCALL(so_base, create, topupBase);
		if(topupBase != NULL){
			ConnectionManager *connManager = ConnectionManager::Instance();
			Connection *conn = connManager->CreateConnection();
			if(conn == NULL){
				slog_write(LL_FATAL, "#%d\tcreate connection instance failed!",tpInfo->seqid);
				DLLCALL2(so_base, destory, topupBase);
				delete tpInfo;
				return;
			}
			tpInfo->conn = conn;
			tpInfo->userid = string("tmall");
			topupBase->Init(tpInfo);
			topupBase->HandleRequest(request, _return);
			//recover db connection
			connManager->Recover(conn);
			//write log to log file
			if(tpInfo->log_len > 0 && tpInfo->log_len < MAX_LOG_LEN){
				tpInfo->log[tpInfo->log_len] = '\0';
				seLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->log);
			}
			if(tpInfo->err_log_len > 0 && tpInfo->err_log_len < MAX_LOG_LEN){
				tpInfo->err_log[tpInfo->err_log_len] = '\0';
				seErrLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->err_log);
			}
			//destory charge up instance
			DLLCALL2(so_base, destory, topupBase);
		}else{
			slog_write(LL_FATAL, "#%d\tcreate topup so instance failed!", tpInfo->seqid);
		}
	}else if(strncmp("/zkcl/", uri, 6) == 0){		//call by zkcl self use
		//create charge up instance
		topupBase = new TopupZkcl();
		if(topupBase != NULL){
			ConnectionManager *connManager = ConnectionManager::Instance();
			Connection *conn = connManager->CreateConnection();
			if(conn == NULL){
				slog_write(LL_FATAL, "#%d\tcreate connection instance failed!",tpInfo->seqid);
				delete topupBase;
				delete tpInfo;
				return;
			}
			tpInfo->conn = conn;
			topupBase->Init(tpInfo);
			topupBase->HandleRequest(request, _return);
			//recover db connection
			connManager->Recover(conn);
			//write back log to file
			if(tpInfo->log_len > 0 && tpInfo->log_len < MAX_LOG_LEN){
				tpInfo->log[tpInfo->log_len] = '\0';
				seLogEx(g_logHandle, "[TopupZkcl] %s", tpInfo->log);
			}
			if(tpInfo->err_log_len > 0 && tpInfo->err_log_len < MAX_LOG_LEN){
				tpInfo->err_log[tpInfo->err_log_len] = '\0';
				seErrLogEx(g_logHandle, "[TopupZkcl] %s", tpInfo->err_log);
			}
			//destory charge up instance
			delete topupBase;
		}else{
			slog_write(LL_FATAL, "#%d\tcreate topup so instance failed!", tpInfo->seqid);
		}
	}else if(strncmp("/customer/", uri, 10) == 0){		//interface for customer
		so_base = &P_TPServer->customer_so;
		//create charge up interface
		DLLCALL(so_base, create, topupBase);
		if(topupBase != NULL){
			ConnectionManager *connManager = ConnectionManager::Instance();
			Connection *conn = connManager->CreateConnection();
			if(conn == NULL){
				slog_write(LL_FATAL, "#%d\tcreate connection instance failed!" ,tpInfo->seqid);
				DLLCALL2(so_base, destory, topupBase);
				delete tpInfo;
				return;
			}
			tpInfo->conn = conn;
			topupBase->Init(tpInfo);
			topupBase->HandleRequest(request, _return);
			//recover db connection
			connManager->Recover(conn);
			//write back log to file
			if(tpInfo->log_len > 0 && tpInfo->log_len < MAX_LOG_LEN){
				tpInfo->log[tpInfo->log_len] = '\0';
				seLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->log);
			}
			if(tpInfo->err_log_len > 0 && tpInfo->err_log_len < MAX_LOG_LEN){
				tpInfo->err_log[tpInfo->err_log_len] = '\0';
				seErrLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->err_log);
			}
			//destory charge up instance
			DLLCALL2(so_base, destory, topupBase);
		}else{
			slog_write(LL_FATAL, "#%d\tcreate topup so customer instance failed!", tpInfo->seqid);
		}
	}else if(strncmp("/result/", uri, 8) == 0){		//interface for receive channel notice
		const char* notify_uri = strrchr(uri, '/');
		if(notify_uri == NULL){
			slog_write(LL_FATAL, "wrong uri in notify[%s]!", uri);
		}else{
			//get the interface of notify and call the interface
			if(strcmp(notify_uri, "notify_dingxin.fcg") == 0){
				//调用相应的notify接口
			}else if(strcmp(notify_uri, "notify_llww.fcg") == 0){
			
			}else if(strcmp(notify_uri, "notify_sls.fcg") == 0){

			}else if(strcmp(notify_uri, "notify_sls.fcg") == 0){

			}else if(strcmp(notify_uri, "notify_sls.fcg") == 0){

			}else if(strcmp(notify_uri, "notify_sls.fcg") == 0){

			}else if(strcmp(notify_uri, "notify_sls.fcg") == 0){

			}
		}	
	}
	delete tpInfo;
}
// 发送服务管理等请求
int32_t TopupService::Admin(const ManageRequest& request){
	//TODO 动态链接库重新加载
	
	//TODO 缓存信息重加载
	//verify user for weixin login
	if(request.cmd == 10){
		ConnectionManager *connManager = ConnectionManager::Instance();
	    Connection *conn = connManager->CreateConnection();	
		ChargeBusiness *chargeBusiness = new ChargeBusiness();
	    chargeBusiness->Init(conn);
		int ret = chargeBusiness->VerifyWeixin(request.key, request.value);
		connManager->Recover(conn);
		delete chargeBusiness;
		return ret;
	}
	return 0;
}
