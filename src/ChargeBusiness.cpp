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
#include "RedisClient.h"
#include "GlobalConfig.h"
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
			channels.push_back(channel);
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
		stmt->setAutoCommit(false);
		stmt->setString(1, topupInfo->qs_info.customer);
		stmt->setString(2, topupInfo->qs_info.customer);
		stmt->setInt(3, topupInfo->qs_info.province);
		stmt->setInt(4, topupInfo->qs_info.value);
		uint64_t sysNo = encode_orderno(topupInfo->qs_info.customer);
		if(sysNo <= 0){
			conn->terminateStatement(stmt);
			return -2;
		}
		string systemNo = lexical_cast<string>(sysNo);
		topupInfo->qs_info.coopOrderNo = systemNo;
		stmt->setString(5, systemNo);
		stmt->setString(6, systemNo);
		stmt->setString(7, topupInfo->qs_info.tbOrderNo);
		stmt->setInt(8, 1);
		string time_str;
		int ret = get_time_now("%Y/%m/%d %H:%M:%S", time_str);
		if(ret < 18){
			conn->terminateStatement(stmt);
			return -3;
		}
		stmt->setString(9, time_str);
		stmt->setString(10, time_str);
		stmt->setInt(11, 0);
		stmt->setInt(12, 1);
		stmt->setInt(13, topupInfo->qs_info.op);
		stmt->setFloat(14, topupInfo->qs_info.sum / topupInfo->qs_info.cardNum);
		stmt->setFloat(15, topupInfo->qs_info.price);
		stmt->setFloat(16,topupInfo->qs_info.price - topupInfo->qs_info.sum / topupInfo->qs_info.cardNum);
		stmt->setInt(17, channelInfo.channelId);
		stmt->executeUpdate();
		conn->terminateStatement(stmt);
		//加入处理队列
		string topup_data;
		//序列化充值信息
	    serialize_topupinfo(topupInfo, topup_data);
	    //充值信息同步到redis,进入处理队列
	    RedisClient *redis = new RedisClient();
	    if(redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
	        redis->select(1);
	        if(!redis->setex(topupInfo->qs_info.tbOrderNo, topup_data, 3600)){
	             TP_WRITE_ERR(topupInfo, "#%d [CreateTmallOrder] setex %s failed\n", topupInfo->seqid, 
						 topupInfo->qs_info.tbOrderNo.c_str());
				 errors.push_back(string("Exception:cache order failed!"));
				 ret = -4;
			}
		    if(!redis->enqueue("underway", topup_data.c_str())){
			    TP_WRITE_ERR(topupInfo, "#%d [CreateTmallOrder] enqueue %s failed\n",
						topupInfo->seqid, topupInfo->qs_info.tbOrderNo.c_str());
				errors.push_back(string("Exception:enqueue underway failed!"));
				ret = -5;
			}
		}else{
			TP_WRITE_ERR(topupInfo, "#%d [CreateTmallOrder] enqueue can't connect to redis %s:%d\n",
					topupInfo->seqid, GlobalConfig::Instance()->s_redis_ip.c_str(), GlobalConfig::Instance()->n_redis_port);
			errors.push_back(string("Exception:enqueue can't connect to redis"));
			ret = -6;
		}
	    delete redis;
	}catch (SQLException &sqlExcp){
		HandleException(sqlExcp);
		ret = -1;
	}catch(std::exception &e){
		HandleException(e);
		ret = -1;
	}
	Finish();
	return ret;	
}

int ChargeBusiness::QueryOrder(TopupInfo *topupInfo){
	int ret = 0;
	try{
		Statement *stmt = conn->createStatement(SQL_CREATE_ORDER);
		string tbOrderNo = topupInfo->qs_info.tbOrderNo;
		stmt->setString(1, tbOrderNo);
		ResultSet *rs = stmt->executeQuery();
		while(rs->next())
		{
			topupInfo->qs_info.tbOrderNo = rs->getString(1);
			topupInfo->qs_info.coopOrderNo = rs->getString(2);
			topupInfo->status = (OrderStatus)rs->getInt(3);
			string ts = rs->getString(4);
			trans_time(ts, topupInfo->update_time);
			ret++;
		}
	}catch(SQLException &sqlExcp){
		HandleException(sqlExcp);
		ret = -1;
	}catch(std::exception &e){
		HandleException(e);
		ret = -1;
	}
	Finish();
	return ret;
}

int ChargeBusiness::UpdateOrderStatus(TopupInfo *topupInfo){
	int ret = 0;
	try{
		int status = 0;
		if(topupInfo->status == SUCCESS){
			status = 1;
		}else if(topupInfo->status == FAILED){
			status = 2;
		}
		string ts;
		get_time_now("%Y/%m/%d %H:%M:%S", ts);
		int notify = 1;
		Statement *stmt = conn->createStatement(SQL_UPDATE_STATUS);
		stmt->setAutoCommit(false);
		string tbOrderNo = topupInfo->qs_info.tbOrderNo;
		stmt->setInt(1, status);
		stmt->setInt(2, notify);
		stmt->setString(3, ts);
		stmt->setString(4, tbOrderNo);
		stmt->executeUpdate();
		conn->terminateStatement(stmt);
	}catch(SQLException &sqlExcp){
		HandleException(sqlExcp);
		ret = -1;
	}catch(std::exception &e){
		HandleException(e);
		ret = -1;
	}
	Finish();
	return ret;
}
