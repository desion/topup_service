/*************************************************************************
	> File Name: ChannelBase.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 03:21:15 PM CST
 ************************************************************************/
#ifndef __CHANNEL_BASE_H
#define __CHANNEL_BASE_H

#include <iostream>
#include "occi.h"
#include "TopupUtils.h"

using namespace std;
using namespace  ::topupinterface;  
using namespace oracle::occi;

struct TopupInfo;

class ChannelBase{
	public:
		virtual ~ChannelBase(){};
		virtual int ChargeRequest(TopupInfo *m_topup_info) = 0;
		virtual int QueryRequest(TopupInfo *m_topup_info) = 0;
		virtual int BalanceRequest(TopupInfo *m_topup_info, double &balance) = 0;
		virtual int AcceptNotify(TopupInfo *m_topup_info) = 0;
};


typedef ChannelBase* (*create_channel)();
typedef void (*destroy_channel)(ChannelBase*);

#endif //__CHANNEL_BASE_H
