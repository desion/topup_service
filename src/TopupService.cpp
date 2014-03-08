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


// 发送充值，查询订单，查询余额，回调，取消订单等请求
void TopupService::SendRequest(std::string& _return,const TopupRequest& request){
	TopupBase *topupBase;
	SoBase *so_base = &P_TPServer->topup_so;
	//创建充值实例
	DLLCALL(so_base, create, topupBase);
	if(topupBase != NULL){
		TopupInfo *tpInfo = new TopupInfo();
		ConnectionManager *connManager = ConnectionManager::Instance();
		Connection *conn = connManager->CreateConnection();
		if(conn == NULL){
			slog_write(LL_FATAL, "create connection instance failed!");
			DLLCALL2(so_base, destory, topupBase);
			return;
		}
		tpInfo->conn = conn;
		topupBase->Init(conn);
		topupBase->HandleRequest(tpInfo, request, _return);
		connManager->Recover(conn);
	}else{
		slog_write(LL_FATAL, "create topup so instance failed!");
	}
	//销毁充值实例
	DLLCALL2(so_base, destory, topupBase);
}
// 发送服务管理等请求
int32_t TopupService::Admin(const ManageRequest& request){
	//TODO 动态链接库重新加载
	
	//TODO 缓存信息重加载
	return 0;
}
