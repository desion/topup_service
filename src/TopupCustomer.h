/*************************************************************************
	> File Name: TopupImpl.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 04:10:52 PM CST
 ************************************************************************/
#ifndef __TOPUP_CUSTOMER_H
#define __TOPUP_CUSTOMER_H

#include <iostream>
#include "TopupInterface_types.h"
#include "TopupBase.h"
#include "ChargeBusiness.h"
#include "TopupUtils.h"
#include "HttpClient.h"
using namespace std;
using namespace  ::topupinterface;


class TopupCustomer: public TopupBase{
	public:
		TopupCustomer();
		~TopupCustomer();
		//对外接口，统一处理FCGI请求
		int HandleRequest(const TopupRequest& request, string& result);

		//连接由主程序分配回收
		int Init(TopupInfo* m_topup_info);

		int Notify();

		void Log();
	
	//上行接口
	protected:
		//针对各个接口的处理函数
		//放货充值接口
		int CustomerCharge(string &response); 
		//放货查询接口
		int CustomerQuery(string &response);
		//放货回调接口，向客户发送回调请求
		int CustomerNotify(string &response);
		//查询余额接口，第三方订购商使用
		int GetBalance(string &response);
	
	protected:
		//返回错误信息
		int MakeErrReplay(const char* errCode,const char* status, string &result);
		//返回成功信息
		int MakeSuccessReplay(const char* status, string &result);

		int MakeBalanceReplay(const char* errCode, double balance, string &result);
		//验证商品信息的正确性
		//包括产品id是否存在，是否有库存，是否下架
		//验证价格是否正确，是否是手机对应省市产品等
		//正确返回0，其他有问题
		int CheckProduct();
		//为对应的产品选择最优的渠道
		//包括充值速度，成功率，对应省市，价格信息，库存信息，优先级等
		//返回对应的渠道信息
		int SelectBestChannel();

		int CheckBalance();

		int CreateTmallOrder();
		//向上游代理商发送充值请求，并取得返回的xml信息
		int SendRequest(string &reponse);

		int QueryOrder();

		bool CheckSign();

		int UpdateStatus();

	private:
		TopupInfo *m_topup_info;		//充值使用的信息
		vector<ChannelInfo> m_channels;
		int m_channel_index;
		map<string, string, cmpKeyAscii> map_entitys;
		Connection *m_conn;
};

//动态链接库调用接口，用于创建相应实例
extern "C" TopupBase* customer_create();

//动态链接库调用接口，用于销毁相应的实例,可不可以通过得到的指针直接销毁
extern "C" void customer_destroy(TopupBase* p);

#endif //__TOPUP_IMPL_H
