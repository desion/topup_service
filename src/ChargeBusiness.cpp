/*************************************************************************
	> File Name: ChargeBusiness.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 22 Feb 2014 11:16:53 PM CST
 ************************************************************************/

#include<iostream>
#include "ChargeBusiness.h"
#include "ConnectionManager.h"
#include <boost/lexical_cast.hpp>
using namespace std;
using boost::lexical_cast;

///ret = 2 没有指定的商品
///ret = 1 商品维护中
int ChargeBusiness::GetTmallProduct(string productId, Product &product){
	int ret = 0;
	try{
		//初始化数据库连接
		Statement *stmt = conn->createStatement(SQL_QUERY_PRODUCT);
		stmt->setString(1, productId);
		ResultSet *rs = stmt->executeQuery();
		while (rs->next())
		{
			product.productId = rs->getString(1);
			product.provinceId = rs->getInt(2);
			product.price = rs->getInt(3);
			product.op = rs->getInt(4);	
		}
		stmt->closeResultSet(rs);
		conn->terminateStatement(stmt);
		if(product.productId.empty() || product.provinceId == 0){
			ret = 2;
		}
	}
	catch(std::exception &e)
	{
		HandleException(e);
		ret = 1;
	}
	Finish();
	return ret;
}
//根据天猫的商品信息，选取最优的渠道
//@return 筛选出来的渠道数量
int ChargeBusiness::SelectBestChannel(int value, int province, int op, vector<ChannelInfo>& channels){
	int ret = 0;
	try{
		Statement *stmt = conn->createStatement(SQL_SELECT_CHANNEL);
		stmt->setInt(1, value);
		stmt->setInt(2, province);
		stmt->setInt(3, op);
		ResultSet *rs = stmt->executeQuery();
		while(rs->next())
		{
			ChannelInfo channel;
			channel.channelId = rs->getInt(1);
			channel.channelName = rs->getString(2);
			channel.sname = rs->getString(3);
			channel.priority = rs->getInt(4);
			channel.repeat = rs->getInt(5);
			channel.discount = rs->getFloat(6);
			channel.interfaceName = rs->getString(7);
			ret++;
		}
		stmt->closeResultSet(rs);
		conn->terminateStatement(stmt);
	}catch(std::exception &e){
		HandleException(e);
		ret = -1;
	}
	Finish();
	return ret;
}

int ChargeBusiness::CreateTmallOrder(TopupInfo *topupInfo, ChannelInfo &channelInfo){
	int ret = 0;
	try{
		Statement *stmt = conn->createStatement(SQL_CREATE_ORDER);
		stmt->setString(1, topupInfo->customer);
		stmt->setString(2, topupInfo->customer);
		stmt->setInt(3, topupInfo->province);
		stmt->setInt(4, topupInfo->value);
		uint64_t sysNo = encode_orderno(topupInfo->customer);
		if(sysNo <= 0){
			return -2;
		}
		string systemNo = lexical_cast<string>(sysNo);
		stmt->setString(5, systemNo);
		stmt->setString(6, systemNo);
		stmt->setString(7, topupInfo->tbOrderNo);
		stmt->setInt(8, 1);
		string time_str;
		int ret = get_time_now("%Y/%m/%d %H:%M:%S", time_str);
		if(ret < 18){
			return -3;
		}
		stmt->setString(9, time_str);
		stmt->setString(10, time_str);
		stmt->setInt(11, 0);
		stmt->setInt(12, 1);
		stmt->setInt(13, topupInfo->op);
		stmt->setFloat(14, topupInfo->sum / topupInfo->cardNum);
		stmt->setFloat(15, topupInfo->price);
		stmt->setFloat(16,topupInfo->price - topupInfo->sum / topupInfo->cardNum);
		stmt->setInt(17, channelInfo.channelId);
	}catch(std::exception &e){
		HandleException(e);
		ret = -1;
	}
	Finish();
	return ret;	
}
