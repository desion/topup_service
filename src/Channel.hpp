/*************************************************************************
	> File Name: Channel.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Mon 10 Feb 2014 09:23:05 PM CST
 ************************************************************************/

#include <iostream>
#include "TopupServer.h"
using namespace std;

//充值渠道基类
class Channel{
	protected:
		string m_channel_id;

	public:
		//请求充值
		virtual int Charge(TopupInfo *topup_info, string &result) = 0;
		//查询订单
		virtual int Query(TopupInfo *topup_info, string &result) = 0;
		//查询余额
		virtual int Balance(TopupInfo *topup_info, double &balance) = 0;
		//接受Notify请求
		virtual int AcceptNotify(TopupInfo *topup_info, string &result) = 0;

};

///手拉手充值渠道处理类
class ChannelSLS : public Channel{
	public:
		ChannelSLS()
		{
			m_channel_id = "SLS";
		}
	public:
		int Charge(TopupInfo *topup_info, string &result);

		int Query(TopupInfo *topup_info, string &result);

		int Balance(TopupInfo *topup_info, double &balance);

		int AcceptNotify(TopupInfo *topup_info, string &result);
};
