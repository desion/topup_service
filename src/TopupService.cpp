/*************************************************************************
	> File Name: TopupService.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Mon 20 Jan 2014 03:48:25 PM CST
 ************************************************************************/

#include <iostream>
#include <sstream>
#include "TopupService.h"
#include "TopupServer.h"
#include "GlobalConfig.h"
using namespace std;

//extern TopupServer *P_TPServer;

// 发送充值，查询订单，查询余额，回调，取消订单等请求
void TopupService::SendRequest(std::string& _return,const TopupRequest& request){
	TopupBase *topupBase;
	SoBase *so_base = &P_TPServer->topup_so;
	//创建充值实例
	DLLCALL(so_base, create, topupBase);
	if(topupBase != NULL){
		ConnectionManager *connManager = ConnectionManager::Instance();
		Connection *conn = connManager->CreateConnection();
		topupBase->Init(conn);
		topupBase->HandleRequest(P_TPServer, request, _return);
		connManager->Recover(conn);
	}else{
		slog_write(LL_FATAL, "create topup so instance failed!");
	}
	//销毁充值实例
	DLLCALL2(so_base, destory, topupBase);
	//日志落地
	P_TPServer->CallLog();
}
// 发送服务管理等请求
int32_t TopupService::Admin(const ManageRequest& request){
	//TODO 动态链接库重新加载
	
	//TODO 缓存信息重加载
	return 0;
}
