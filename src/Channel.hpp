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
			errors.insert(make_pair("001", "用户不存在"));
			errors.insert(make_pair("002", "交易密码错误"));
			errors.insert(make_pair("003", "用户已禁用"));
			errors.insert(make_pair("004", "余额不足"));
			errors.insert(make_pair("005", "订单号已存在"));
			errors.insert(make_pair("006", "系统异常"));
			errors.insert(make_pair("101", "数据库错误"));
			errors.insert(make_pair("201", "IP拒绝访问"));
			errors.insert(make_pair("202", "订单号格式错误"));
			errors.insert(make_pair("203", "通知URL错误"));
			errors.insert(make_pair("301", "暂无此面额缴费产品"));
			errors.insert(make_pair("401", "缺少参数"));
			errors.insert(make_pair("402", "参数错误"));
			errors.insert(make_pair("403", "充值金额错误"));
			errors.insert(make_pair("404", "充值号码错误"));
			errors.insert(make_pair("405", "签名错误"));
			errors.insert(make_pair("501", "余额查询失败"));
			errors.insert(make_pair("555", "余额查询成功"));
			errors.insert(make_pair("600", "订单号不存在"));
			errors.insert(make_pair("601", "充值成功"));
			errors.insert(make_pair("602", "充值失败"));
			errors.insert(make_pair("603", "订单正在处理"));
		}
	public:
		int Charge(TopupInfo *topup_info, string &result);

		int Query(TopupInfo *topup_info, string &result);

		int Balance(TopupInfo *topup_info, double &balance);

		int AcceptNotify(TopupInfo *topup_info, string &result);

		//static bool Init();
		string GetErrMsg(string code);
	private:
		static const char* key;
		static const char* userid;
		static const char* pwd;
		map<string, string> errors;
};
