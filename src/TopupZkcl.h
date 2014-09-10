/*************************************************************************
    > File Name: TopupZkcl.h
    > Author: desionwang
    > Mail: wdxin1322@qq.com 
    > Created Time: Sat 08 Feb 2014 04:10:52 PM CST
 ************************************************************************/
#ifndef __TOPUP_ZKCL_H
#define __TOPUP_ZKCL_H

#include <iostream>
#include "TopupInterface_types.h"
#include "TopupBase.h"
#include "ChargeBusiness.h"
#include "TopupUtils.h"
#include "HttpClient.h"
#include "RedisClient.h"
using namespace std;
using namespace  ::topupinterface;


class TopupZkcl: public TopupBase{
    public:
        TopupZkcl();
        ~TopupZkcl();
        //对外接口，统一处理FCGI请求
        int HandleRequest(const TopupRequest& request, string& result);

        //连接由主程序分配回收
        int Init(TopupInfo* m_topup_info);

        int Notify();
        //日志落地，在对象要销毁时调用
        void Log();
    
    //上行接口
    protected:
        //充值接口
        int ZkclCharge(string &response); 
    
    protected:
        //返回错误信息
        int MakeErrReplay(const char* errCode,const char* status, string &result);
        //返回成功信息
        int MakeSuccessReplay(const char* status, string &result);
        //正确返回0，其他有问题
        int CheckProduct();
        //为对应的产品选择最优的渠道
        //包括充值速度，成功率，对应省市，价格信息，库存信息，优先级等
        //返回对应的渠道信息
        int SelectBestChannel();

        int CreateTmallOrder();
        //向上游代理商发送充值请求，并取得返回的xml信息
        int SendRequest(string &reponse);
        
        int UpdateStatus();

        bool CheckSign();


    private:
        TopupInfo *m_topup_info;        //充值使用的信息
        vector<ChannelInfo> m_channels;
        map<string, string, cmpKeyAscii> map_entitys;
        Connection *m_conn;
};

#endif //__TOPUP_ZKCL_H
