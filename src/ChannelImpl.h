/*************************************************************************
	> File Name: TopupImpl.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 04:10:52 PM CST
 ************************************************************************/
#ifndef __CHANNEL_IMPL_H
#define __CHANNEL_IMPL_H

#include <iostream>
#include "ChannelBase.h"
#include "ChargeBusiness.h"
#include "Channel.hpp"
#include "TopupUtils.h"
#include "HttpClient.h"
#include "RedisClient.h"
using namespace std;
using namespace  ::topupinterface;



class ChannelImpl: public ChannelBase{
	public:
		ChannelImpl();
		~ChannelImpl();

		//处理充值请求
		int ChargeRequest(TopupInfo *m_topup_info);
		//处理查询请求
	    int QueryRequest(TopupInfo *m_topup_info);
		//余额查询
	    int BalanceRequest(TopupInfo *m_topup_info, double &balance);
		//接收异步回调
	    int AcceptNotify(TopupInfo *m_topup_info);
	
	protected:
		int SendCharge(string &response); 
		int SendQuery(string &response);
		int AccpetNotify(string &response);
		int GetBalance(string &response);
	

	private:
		TopupInfo *m_topup_info;	
		map<string, string, cmpKeyAscii> map_entitys;
		Connection *m_conn;
};
//动态链接库调用接口，用于创建相应实例
extern "C" ChannelBase* channel_create(); 

//动态链接库调用接口，用于销毁相应的实例,可不可以通过得到的指针直接销毁
extern "C" void channel_destroy(ChannelBase* p);
#endif //__CHANNEL_IMPL_H
