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
#include "TopupServer.h"
#include "GlobalConfig.h"
using namespace std;


// 处理thrift请求
void TopupService::SendRequest(std::string& _return,const TopupRequest& request){
	TopupBase *topupBase;
	SoBase *so_base;
	TopupInfo *tpInfo = new TopupInfo();
	//通过URI目录区分下游用户和TMall
	const char* uri = request.uri.c_str();
	if(strncmp("/tmall/", uri, 7) == 0){
		so_base = &P_TPServer->topup_so;
		//创建充值实例
		DLLCALL(so_base, create, topupBase);
		if(topupBase != NULL){
			P_TPServer->SetSeqId(&tpInfo->seqid); 
			ConnectionManager *connManager = ConnectionManager::Instance();
			Connection *conn = connManager->CreateConnection();
			if(conn == NULL){
				slog_write(LL_FATAL, "create connection instance failed!");
				DLLCALL2(so_base, destory, topupBase);
				return;
			}
			tpInfo->conn = conn;
			topupBase->Init(tpInfo);
			topupBase->HandleRequest(request, _return);
			connManager->Recover(conn);
		}else{
			slog_write(LL_FATAL, "create topup so instance failed!");
		}
		//销毁充值实例
		DLLCALL2(so_base, destory, topupBase);
	}else if(strncmp("/customer/", uri, 10) == 0){
		so_base = &P_TPServer->customer_so;
		//创建充值实例
		DLLCALL(so_base, create, topupBase);
		if(topupBase != NULL){
			P_TPServer->SetSeqId(&tpInfo->seqid); 
			ConnectionManager *connManager = ConnectionManager::Instance();
			Connection *conn = connManager->CreateConnection();
			if(conn == NULL){
				slog_write(LL_FATAL, "create connection instance failed!");
				DLLCALL2(so_base, destory, topupBase);
				return;
			}
			tpInfo->conn = conn;
			topupBase->Init(tpInfo);
			topupBase->HandleRequest(request, _return);
			connManager->Recover(conn);
		}else{
			slog_write(LL_FATAL, "create topup so instance failed!");
		}
		//销毁充值实例
		DLLCALL2(so_base, destory, topupBase);
	}else if(strncmp("/result/", uri, 8) == 0){
		const char* notify_uri = strrchr(uri, '/');
		if(notify_uri == NULL){
			slog_write(LL_FATAL, "wrong uri in notify[%s]!", uri);
		}else{
			if(strcmp(notify_uri, "notify_sls.fcg") == 0){
				//调用相应的notify接口
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
