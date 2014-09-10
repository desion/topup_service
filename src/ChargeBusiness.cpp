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
#include <boost/shared_ptr.hpp>
#include "RedisClient.h"
#include "GlobalConfig.h"
using namespace std;
using boost::lexical_cast;
using boost::shared_ptr;

///ret = 2 no such product
///ret = 1 exception not find product
int ChargeBusiness::GetTmallProduct(string productId, Product &product){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        //初始化数据库连接
        stmt = conn->createStatement(SQL_QUERY_PRODUCT);
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
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;
}
//through the product info get the best channel to create order
//@return the channel num
int ChargeBusiness::SelectBestChannel(int value, int province, int op, vector<ChannelInfo>& channels){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        stmt = conn->createStatement(SQL_SELECT_CHANNEL);
        stmt->setInt(1, value);         //product value
        stmt->setInt(2, province);      //provice of product
        stmt->setInt(3, op);            //operator of product
        ResultSet *rs = stmt->executeQuery();
        while(rs->next())
        {
            ChannelInfo channel;
            channel.channelId = rs->getInt(1);               //channel id
            //channel.channelName = rs->getString(2);          //channel name
            channel.sname = rs->getString(2);                //short name
            channel.priority = rs->getInt(3);                //priority
            channel.repeat = rs->getInt(4);                  //repeat times
            channel.discount = rs->getFloat(5);              //discount of product
            channel.interfaceName = rs->getString(6);        //the interface of channel,through it to find the class to handle order
            channel.pid = rs->getString(7);                  //the product id given by channel
            channel.private_key = rs->getString(8);          //the private key given by channel
            channel.query_interval = rs->getInt(9);          //query order interval
            channels.push_back(channel);
            ret++;
        }
        stmt->closeResultSet(rs);
    }catch(std::exception &e){
        HandleException(e);
        ret = -1;
    }
    Finish();
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;
}


int ChargeBusiness::CreateTmallOrder(TopupInfo *topupInfo, ChannelInfo &channelInfo){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        stmt = conn->createStatement(SQL_CREATE_ORDER);
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
        //订单状态
        stmt->setInt(8, 1);
        string time_str;
        int len = get_time_now("%Y/%m/%d %H:%M:%S", time_str);
        if(len < 18){
            conn->terminateStatement(stmt);
            return -3;
        }
        stmt->setString(9, time_str);
        stmt->setString(10, time_str);
        //notify
        stmt->setInt(11, 0);
        //the source of order
        if(topupInfo->userid == "tmall"){
            stmt->setInt(12, 1);
        }else if(topupInfo->userid == "zkcl"){
            //the interface for internal recharge
            stmt->setInt(12, 2);
        }else if(topupInfo->userid == "ybtj"){
            //the interface for ybtj
            stmt->setInt(12, 3);
        }else{
            //customer interface
            //use source field record customer id,the id use for refund
            stmt->setInt(12, topupInfo->qs_info.coopId);
        }
        //operator
        stmt->setInt(13, topupInfo->qs_info.op);
        //price //TODO has question
        stmt->setFloat(14, topupInfo->qs_info.sum / topupInfo->qs_info.cardNum);
        int total_value = topupInfo->qs_info.value * topupInfo->qs_info.cardNum;
        //the sale price
        stmt->setFloat(15, total_value * channelInfo.discount);
        //the profit of this order
        stmt->setFloat(16,topupInfo->qs_info.sum - total_value * channelInfo.discount);
        //the channel used for charge up,this can be change when create order failed
        stmt->setInt(17, channelInfo.channelId);
        stmt->executeUpdate();
        string topup_data;
        //serialize the topup info
        serialize_topupinfo(topupInfo, topup_data);
        //push to underway queue
        shared_ptr<RedisClient> redis(new RedisClient());
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
            //internal order can't summit twice in 30 minutes, check phone and charge price
            if(topupInfo->userid == "zkcl"){
                char buf[256] = {0};
                sprintf(buf, "%s_%d", topupInfo->qs_info.customer.c_str(), topupInfo->qs_info.value);
                if(!redis->setex(buf, buf, 60 * 30)){
                    TP_WRITE_ERR(topupInfo, "#%d [CreateTmallOrder] record internat charge info failed %s\t%d\n",
                        topupInfo->seqid, topupInfo->qs_info.customer.c_str(), topupInfo->qs_info.value);
                    errors.push_back(string("REDIS Exception:record internat charge info failed!"));
                }
            }
        }else{
            TP_WRITE_ERR(topupInfo, "#%d [CreateTmallOrder] enqueue can't connect to redis %s:%d\n",
                    topupInfo->seqid, GlobalConfig::Instance()->s_redis_ip.c_str(), GlobalConfig::Instance()->n_redis_port);
            errors.push_back(string("Exception:enqueue can't connect to redis"));
            ret = -6;
        }
    }catch (SQLException &sqlExcp){
        HandleException(sqlExcp);
        ret = -1;
    }catch(std::exception &e){
        HandleException(e);
        ret = -1;
    }
    Finish();
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;    
}

int ChargeBusiness::QueryOrder(TopupInfo *topupInfo){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        stmt = conn->createStatement(SQL_CREATE_ORDER);
        string tbOrderNo = topupInfo->qs_info.tbOrderNo;
    fprintf(stderr, "ChargeBusiness::QueryOrder %s\n", tbOrderNo.c_str());
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
        conn->terminateStatement(stmt);
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
        ret = -1;
    }catch(std::exception &e){
        HandleException(e);
        ret = -1;
    }
    Finish();
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;
}

int ChargeBusiness::UpdateOrderStatus(TopupInfo *topupInfo){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        int status = 0;
        if(topupInfo->status == SUCCESS){
            status = 1;
        }else if(topupInfo->status == FAILED){
            status = 2;
        }
        string ts;
        get_time_now("%Y/%m/%d %H:%M:%S", ts);
        int notify = topupInfo->notify;
        stmt = conn->createStatement(SQL_UPDATE_STATUS);
        stmt->setAutoCommit(false);
        string tbOrderNo = topupInfo->qs_info.coopOrderNo;
        stmt->setInt(1, status);
        stmt->setInt(2, notify);
        stmt->setString(3, ts);
        stmt->setString(4, tbOrderNo);
        stmt->executeUpdate();
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
        ret = -1;
    }catch(std::exception &e){
        HandleException(e);
        ret = -1;
    }
    Finish();
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;
}

int ChargeBusiness::UpdateChannel(TopupInfo *topupInfo){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        stmt = conn->createStatement(SQL_UPDATE_CHANNEL);
        stmt->setAutoCommit(false);
        string tbOrderNo = topupInfo->qs_info.coopOrderNo;
        stmt->setInt(1, topupInfo->channelId);
        int total_value = topupInfo->qs_info.value * topupInfo->qs_info.cardNum;
        stmt->setFloat(2, total_value * topupInfo->channel_discount);
        stmt->setFloat(3, topupInfo->qs_info.sum - total_value * topupInfo->channel_discount);
        stmt->setString(4, tbOrderNo);
        stmt->executeUpdate();
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
        ret = -1;
    }catch(std::exception &e){
        HandleException(e);
        ret = -1;
    }
    Finish();
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;
}
/***
 * get the balance of customer and get the passwd for charge
 */
int ChargeBusiness::CheckAndBalance(TopupInfo *topupInfo){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        stmt = conn->createStatement(SQL_BALANCE_UPDATE);
        stmt->setAutoCommit(false);
        string tbOrderNo = topupInfo->qs_info.coopOrderNo;
        stmt->setString(1, topupInfo->qs_info.coopId);
        ResultSet *rs = stmt->executeQuery();
        double banlance = 0.0;
        bool has_result = false;
        while(rs->next())
        {
            banlance = rs->getDouble(2);
            has_result = true;
        }
        if(!has_result){
            errors.push_back(string("Exception:can't find customer info!"));
            ret = 2;        
        }
        if(topupInfo->qs_info.sum > banlance)
        {
            //余额不足
            ret =  1;
        }
        else
        {
            stmt = conn->createStatement(SQL_UPDATE_CUSTOMER_BALANCE);
            stmt->setAutoCommit(false);
            banlance = banlance - topupInfo->qs_info.sum;
            stmt->setDouble(1, banlance);
            stmt->setString(1, topupInfo->qs_info.coopId);
            stmt->executeUpdate();
        }
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
        ret = -1;
    }catch(std::exception &e){
        HandleException(e);
        ret = -1;
    }
    Finish();
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;    
}
    
int ChargeBusiness::GetBalance(string userid, double &balance){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        stmt = conn->createStatement(SQL_BALANCE_UPDATE);
        stmt->setAutoCommit(false);
        stmt->setString(1, userid);
        ResultSet *rs = stmt->executeQuery();
        bool has_result = false;
        while(rs->next())
        {
            balance = rs->getDouble(2);
            has_result = true;
        }
        if(!has_result){
            errors.push_back(string("Exception:can't find customer info!"));
            ret = 1;        
        }
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
        ret = -1;
    }catch(std::exception &e){
        HandleException(e);
        ret = -1;
    }
    Finish();
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;    
}

int ChargeBusiness::NotifyOrder(TopupInfo *topupInfo){
    int ret = 0;
    Statement *stmt = NULL;
    try{
        string ts;
        get_time_now("%Y/%m/%d %H:%M:%S", ts);
        int notify = 1;
        stmt = conn->createStatement(SQL_UPDATE_NOTIFY);
        stmt->setAutoCommit(false);
        string sysNo = topupInfo->qs_info.coopOrderNo;
        stmt->setInt(1, topupInfo->notify);
        stmt->setString(2, ts);
        stmt->setString(3, sysNo);
        stmt->executeUpdate();
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
        ret = -1;
    }catch(std::exception &e){
        HandleException(e);
    }
    if(stmt)
        conn->terminateStatement(stmt);
    Finish();
    return ret;
}

int ChargeBusiness::GetNotifyStatus(string &sysNo){
    int notify = 0;
    Statement *stmt = NULL;
    try{
        stmt = conn->createStatement(SQL_QUERY_NOTIFY);
        stmt->setString(1, sysNo);
        ResultSet *rs = stmt->executeQuery();
        while(rs->next())
        {
            notify = rs->getInt(1);
        }
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
    }catch(std::exception &e){
        HandleException(e);
    }
    if(stmt)
        conn->terminateStatement(stmt);
    return notify;
}

//验证用户微信号
int ChargeBusiness::VerifyWeixin(string userId, string openId){
    Statement *stmt = NULL;
    int ret = 0;
    try{
        stmt = conn->createStatement(QUERY_USER_SQL);
        stmt->setString(1, userId);
        ResultSet *rs = stmt->executeQuery();
        string user_open_id;
        int id = -1;
        while(rs->next())
        {
            id = rs->getInt(1);
            user_open_id = rs->getString(2);
        }
        if(id == -1){
            ret = 1;
        }else if(user_open_id.empty()){
            conn->terminateStatement(stmt);
            stmt = conn->createStatement(VERIFY_SQL);
            stmt->setString(1, openId);
            stmt->setString(2, userId);
            stmt->executeUpdate();
        }else{
            ret = 2;
        }
        
    }catch(SQLException &sqlExcp){
        HandleException(sqlExcp);
    }catch(std::exception &e){
        HandleException(e);
    }   
    if(stmt)
        conn->terminateStatement(stmt);
    return ret;
}
