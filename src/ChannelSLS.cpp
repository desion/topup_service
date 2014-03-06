/*************************************************************************
	> File Name: ChannelSLS.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 15 Feb 2014 09:57:19 AM CST
 ************************************************************************/

#include <iostream>
#include "Channel.hpp"
using namespace std;

//向手拉手发送充值请求，对结果进行处理
int ChannelSLS::Charge(TopupServer *tpServer, map<string, string> &params, string &result)
{
	TP_WRITE_LOG(tpServer, "[ChannelSLS][Charge] CALL");
	//填充参数
	//发送充值请求
	//解析处理结果
	//是否请求重发
	return 0;
}

//向手拉手发送订单查询请求，并对结果进行处理
int ChannelSLS::Query(TopupServer *tpServer, map<string, string> &params, string &result)
{
	TP_WRITE_LOG(tpServer, "[ChannelSLS][Query] CALL");
	//填充参数
	//发送请求
	//解析处理结果
	return 0;
}

//向手拉手发送余额查询请求
int ChannelSLS::Balance(map<string, string> &params, double &balance)
{
	//填充参数
	//发送请求
	//解析结果
	return 0;
}

//接受手拉手的异步回调请求
int ChannelSLS::AcceptNotify(TopupServer *tpServer, map<string, string> &params, string &result)
{
	TP_WRITE_LOG(tpServer, "[ChannelSLS][AcceptNotify] CALL");
	return 0;
}
