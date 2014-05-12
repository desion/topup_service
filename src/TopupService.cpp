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

// 处理thrift请求
void TopupService::SendRequest(std::string& _return,const TopupRequest& request){
	TopupBase *topupBase;
	SoBase *so_base;
	TopupInfo *tpInfo = new TopupInfo();
	assert(tpInfo != NULL);
	//通过URI目录区分下游用户和TMall
	const char* uri = request.uri.c_str();
	P_TPServer->SetSeqId(&tpInfo->seqid); 
	slog_write(LL_TRACE, "#%d\t[%s]\t%s\t%s\t%d",tpInfo->seqid, request.uri.c_str()
            ,request.query.c_str(), request.checksum.c_str(), request.itimestamp);
	if(strncmp("/tmall/", uri, 7) == 0){		//tmall接口目录
		so_base = &P_TPServer->topup_so;
		//创建充值实例
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
			//回收连接池连接
			connManager->Recover(conn);
			//日志落地
			if(tpInfo->log_len > 0 && tpInfo->log_len < MAX_LOG_LEN){
				tpInfo->log[tpInfo->log_len] = '\0';
				seLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->log);
			}
			if(tpInfo->err_log_len > 0 && tpInfo->err_log_len < MAX_LOG_LEN){
				tpInfo->err_log[tpInfo->err_log_len] = '\0';
				seErrLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->err_log);
			}
			//销毁充值实例
			DLLCALL2(so_base, destory, topupBase);
		}else{
			slog_write(LL_FATAL, "#%d\tcreate topup so instance failed!", tpInfo->seqid);
		}
	}else if(strncmp("/zkcl/", uri, 6) == 0){		//中科补充接口目录
		//创建充值实例
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
			tpInfo->userid = string("zkcl");
			topupBase->Init(tpInfo);
			topupBase->HandleRequest(request, _return);
			//回收连接池连接
			connManager->Recover(conn);
			//日志落地
			if(tpInfo->log_len > 0 && tpInfo->log_len < MAX_LOG_LEN){
				tpInfo->log[tpInfo->log_len] = '\0';
				seLogEx(g_logHandle, "[TopupZkcl] %s", tpInfo->log);
			}
			if(tpInfo->err_log_len > 0 && tpInfo->err_log_len < MAX_LOG_LEN){
				tpInfo->err_log[tpInfo->err_log_len] = '\0';
				seErrLogEx(g_logHandle, "[TopupZkcl] %s", tpInfo->err_log);
			}
			//销毁充值实例
			delete topupBase;
		}else{
			slog_write(LL_FATAL, "#%d\tcreate topup so instance failed!", tpInfo->seqid);
		}
	}else if(strncmp("/customer/", uri, 10) == 0){		//下游放货接口目录
		so_base = &P_TPServer->customer_so;
		//创建充值实例
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
			//回收连接池连接
			connManager->Recover(conn);
			//日志落地
			if(tpInfo->log_len > 0 && tpInfo->log_len < MAX_LOG_LEN){
				tpInfo->log[tpInfo->log_len] = '\0';
				seLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->log);
			}
			if(tpInfo->err_log_len > 0 && tpInfo->err_log_len < MAX_LOG_LEN){
				tpInfo->err_log[tpInfo->err_log_len] = '\0';
				seErrLogEx(g_logHandle, "[TopupImpl] %s", tpInfo->err_log);
			}
			//销毁充值实例
			DLLCALL2(so_base, destory, topupBase);
		}else{
			slog_write(LL_FATAL, "#%d\tcreate topup so customer instance failed!", tpInfo->seqid);
		}
	}else if(strncmp("/result/", uri, 8) == 0){		//接受上游通知接口目录
		const char* notify_uri = strrchr(uri, '/');
		if(notify_uri == NULL){
			slog_write(LL_FATAL, "wrong uri in notify[%s]!", uri);
		}else{
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
	return 0;
}
