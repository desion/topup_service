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
#define SQL_QUERY_PRODUCT "SELECT ID,ZONE,VALUE,OPERATOR FROM PRODUCT_TBL WHERE ID = :1 AND STATUS != 3"
#define SQL_SELECT_CHANNEL "SELECT CHANNEL_ID,NAME,SNAME,PRIORITY,REPEAT,DISCOUNT,INTERFACE FROM CHANNEL_DETAIL_TBL WHERE VALUE = :1 AND PROVINCE = :2 AND OPERATOR = :3 AND STATUS = 1"
#define SQL_CREATE_ORDER "INSERT INTO TOPUP_ORDER_TBL (PHONE_NO, TOPUP_PHONE, PROVINCE, SUM, SYSTEM_NO, TOPUP_NO, REQUEST_NO, STATUS, CREATE_TIME, UPDATE_TIME, NOTIFY, SOURCE, OPERATOR, SALE_PRICE, IN_PRICE, PROFIT, PROXY) VALUES (:1,:2,:3,:4,:5,:6,:7,:8,:9,:10,:11,:12,:13,:14,:15,:16,:17)"

//订单查询
#define SQL_QUERY_ORDER "SELECT REQUEST_NO, SYSTEM_NO,STATUS, UPDATE_TIME FROM TOPUP_ORDER_TBL WHERE REQUEST_NO = :1"

#define SQL_UPDATE_STATUS "UPDATE TOPUP_ORDER_TBL set STATUS = :1 ,NOTIFY = :2, UPDATE_TIME = :3 WHERE SYSTEM_NO = :4"

#define SQL_UPDATE_CHANNEL "UPDATE TOPUP_ORDER_TBL set PROXY = :1, INPRICE = :2, PROFIT = :3 WHERE SYSTEM_NO = :4"

//(13693555577, 13693555577, 2, 100, '1234567891231', '1234567891231', '1234567891231', 1, '2013/12/20 12:25:22', '2013/12/20 12:28:22', 0, 2, 2, 100.12, 98.95, 1.02, 101);"

typedef struct Product{
	string productId;
	int price;
	double discount;
	int provinceId;
	int op;
} Product;


class ChargeBusiness: public BaseBusiness
{
public:
	int GetTmallProduct(string productId, Product &product);

	int SelectBestChannel(int value, int province, int op, vector<ChannelInfo>& channels);

	int CreateTmallOrder(TopupInfo *topupInfo, ChannelInfo &channelInfo);

	int ChargeTmallOrder(TopupInfo *topupInfo, ChannelInfo &channelInfo);

	int QueryOrder(TopupInfo *topupInfo);

	int UpdateOrderStatus(TopupInfo *topupInfo);

	int UpdateChannel(TopupInfo *topupInfo);
};


#endif //__CHARGE_BUSINESS_H
