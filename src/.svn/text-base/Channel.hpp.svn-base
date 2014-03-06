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
		virtual int Charge(TopupServer *tpServer, map<string, string> &params, string &result) = 0;
		//查询订单
		virtual int Query(TopupServer *tpServer, map<string, string> &params, string &result) = 0;
		//查询余额
		virtual int Balance(map<string, string> &params, double &balance) = 0;
		//接受Notify请求
		virtual int AcceptNotify(TopupServer *tpServer, map<string, string> &params, string &result) = 0;

};

///手拉手充值渠道处理类
class ChannelSLS : public Channel{
	public:
		ChannelSLS()
		{
			m_channel_id = "SLS";
		}
	public:
		int Charge(TopupServer *tpServer, map<string, string> &params, string &result);

		int Query(TopupServer *tpServer, map<string, string> &params, string &result);

		int Balance(map<string, string> &params, double &balance);

		int AcceptNotify(TopupServer *tpServer, map<string, string> &params, string &result);
};
