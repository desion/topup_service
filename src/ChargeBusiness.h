/*************************************************************************
	> File Name: ChargeBusiness.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Fri 21 Feb 2014 05:24:29 PM CST
 ************************************************************************/
#ifndef __CHARGE_BUSINESS_H
#define __CHARGE_BUSINESS_H

#include <iostream>
#include "BaseBusiness.h"
#include "TopupUtils.h"
#include <vector>


using namespace std;
//验证商品信息
#define SQL_QUERY_PRODUCT "SELECT ID,ZONE,VALUE,OPERATOR FROM PRODUCT_TBL WHERE ID = :1 AND STATUS != 3"
//选取渠道
#define SQL_SELECT_CHANNEL "SELECT CHANNEL_ID,NAME,SNAME,PRIORITY,REPEAT,DISCOUNT,INTERFACE,PID FROM CHANNEL_DETAIL_TBL WHERE VALUE = :1 AND PROVINCE = :2 AND OPERATOR = :3 AND STATUS = 1"
//创建订单信息
#define SQL_CREATE_ORDER "INSERT INTO TOPUP_ORDER_TBL (PHONE_NO, TOPUP_PHONE, PROVINCE, SUM, SYSTEM_NO, TOPUP_NO, REQUEST_NO, STATUS, CREATE_TIME, UPDATE_TIME, NOTIFY, SOURCE, OPERATOR, SALE_PRICE, IN_PRICE, PROFIT, PROXY) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14,:15,:16,:17)"

//订单查询
#define SQL_QUERY_ORDER "SELECT REQUEST_NO, SYSTEM_NO,STATUS, UPDATE_TIME FROM TOPUP_ORDER_TBL WHERE REQUEST_NO = :1"
//更新订单状态信息
#define SQL_UPDATE_STATUS "UPDATE TOPUP_ORDER_TBL set STATUS = :1 ,NOTIFY = :2, UPDATE_TIME = :3 WHERE SYSTEM_NO = :4"
//更新状态信息
#define SQL_UPDATE_NOTIFY "UPDATE TOPUP_ORDER_TBL set NOTIFY = :1, UPDATE_TIME = :2 WHERE SYSTEM_NO = :3"
//更新是否notify
#define SQL_QUERY_NOTIFY "SELECT NOTIFY FROM TOPUP_ORDER_TBL WHERE SYSTEM_NO = :1"
//更新channel信息
#define SQL_UPDATE_CHANNEL "UPDATE TOPUP_ORDER_TBL set PROXY = :1, INPRICE = :2, PROFIT = :3 WHERE SYSTEM_NO = :4"
//查询余额并加锁
#define SQL_BALANCE_UPDATE "SELECT ID, BALANCE FROM CUSTOMER_TBL WHERE USER_NAME = :1 FOR UPDATE NOWAIT"
//更新下游余额信息
#define SQL_UPDATE_CUSTOMER_BALANCE "UPDATE CUSTOMER_TBL SET BALANCE = :1 WHERE USER_NAME = :2"

//(13693555577, 13693555577, 2, 100, '1234567891231', '1234567891231', '1234567891231', 1, '2013/12/20 12:25:22', '2013/12/20 12:28:22', 0, 2, 2, 100.12, 98.95, 1.02, 101);"

typedef struct Product{
	string productId;		//商品id
	int price;				//商品价格
	double discount;		//折扣信息
	int provinceId;			//省市信息
	int op;					//运营商
} Product;


class ChargeBusiness: public BaseBusiness
{
public:
	//获取商品信息
	int GetTmallProduct(string productId, Product &product);
	//获取可用渠道信息
	int SelectBestChannel(int value, int province, int op, vector<ChannelInfo>& channels);
	//创建订单信息
	int CreateTmallOrder(TopupInfo *topupInfo, ChannelInfo &channelInfo);
	//更新订单信息
	int ChargeTmallOrder(TopupInfo *topupInfo, ChannelInfo &channelInfo);
	//查询订单信息
	int QueryOrder(TopupInfo *topupInfo);
	//更新状态信息
	int UpdateOrderStatus(TopupInfo *topupInfo);
	//更新notify信息
	int NotifyOrder(TopupInfo *topupInfo);
	//更新channel信息
	int UpdateChannel(TopupInfo *topupInfo);
	//获取notify信息
	int GetNotifyStatus(string &sysNo);
	//查询和更新下游余额信息
	int CheckAndBalance(TopupInfo *topupInfo);

	int GetBalance(string userid, double &balance);
	
	int SystemMessage(const string& message, int type);
};


#endif //__CHARGE_BUSINESS_H
