/*************************************************************************
	> File Name: OrderHandler.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Wed 02 Apr 2014 11:07:29 PM CST
 ************************************************************************/

#include "GlobalConfig.h"
#include "TopupUtils.h"
#include "RedisClient.h"
#include "ChannelImpl.h"
#include "TopupBase.h"
#include "TopupImpl.h"
#include "TopupCustomer.h"
#include "OrderHandler.h"
#include "boost/shared_ptr.hpp"

using namespace boost;

extern LOG_HANDLE g_logHandle; 
#define INITIAL_SLEEP_TIME 10000
#define MAX_SLEEP_TIME 200000
#define TIMEOUT_TIME 1800

void *charge(void *arg){
	int sleep_time = INITIAL_SLEEP_TIME;//ms	
	int fail_time = 0;
	int retry_time = 5;
	int charge_status = 1;
	shared_ptr<RedisClient> redis(new RedisClient());
	//连接redis
	if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
		seErrLogEx(g_logHandle, "[charge#%lu] start thread can't connect to redis %s:%d"
				,pthread_self(), GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
		return NULL;
	}
	//循环出来队列数据
	while(1){
		string value;
		//默认使用1库
		redis->select(1);
		if(!redis->dequeue(CHARGEQUEUE, value)){
			fail_time++;
			//出队列失败次数大于5次，直接ping，如果失败线程退出，发送信息
			if(fail_time > 5){
				if(redis->ping()){
					fail_time = 0;
				}else{
					if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
						seErrLogEx(g_logHandle, "[charge#%lu] reconnect can't connect to redis %s:%d thread exit"
								,pthread_self(), GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
						return NULL;
					}
				}
			}
			//没有新数据增加sleep时间
			if(sleep_time < MAX_SLEEP_TIME)
				sleep_time += 10;
		}else{
			//有新数据立即处理，并减少sleep时间
			if(sleep_time > 0)
				sleep_time -= 10;
			TopupInfo *topup_info = new TopupInfo;
			deserialize_topupinfo(value, topup_info);
			shared_ptr<ChannelImpl> channel_handler(new ChannelImpl);
			//尝试5次到代理商建立订单，失败认为订单失败，并放入通知队列
			while(retry_time >=0 && charge_status != 0){
				charge_status = channel_handler->ChargeRequest(topup_info);
				retry_time--;
			}
			if(charge_status != 0){
				//创建失败,不再返回队列，直接作为失败处理，放入通知队列
				seErrLogEx(g_logHandle, "[charge#%lu] order:%s try 5 times and failed!",pthread_self(), value.c_str());
				//更新订单状态为失败，并通知
				shared_ptr<ChargeBusiness> chargeBusiness(new ChargeBusiness());
				ConnectionManager *connManager = ConnectionManager::Instance();
				Connection *conn = connManager->CreateConnection();
				chargeBusiness->Init(conn);
				topup_info->status = FAILED;
				chargeBusiness->UpdateOrderStatus(topup_info);
				connManager->Recover(conn);
				//通知失败
				string fail_data;
				serialize_topupinfo(topup_info, fail_data);
				redis->enqueue(NOTIFYQUEUE, fail_data.c_str());		
				seLogEx(g_logHandle, "[charge#%lu] create order failed push to notify:%s",pthread_self(), fail_data.c_str());
			}else{
				//成功放入查询队列
				string success_data;
				serialize_topupinfo(topup_info, success_data);
				redis->enqueue(QUERYQUEUE, success_data.c_str());		
				printf("%s\n", success_data.c_str());
				seLogEx(g_logHandle, "[charge#%lu] create order success push to query:%s",pthread_self(), success_data.c_str());
			}
			delete topup_info;
		}
		if(sleep_time > 0)
			usleep(sleep_time);
		sleep(1);
	}
}

void *query(void *arg){
	int sleep_time = INITIAL_SLEEP_TIME;//ms	
	int fail_time = 0;
	shared_ptr<RedisClient> redis(new RedisClient());
	if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
		seErrLogEx(g_logHandle, "[query#%lu] start thread can't connect to redis %s:%d"
				,pthread_self(), GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
		return NULL;
	}

	while(1){
		string value;
		redis->select(1);
		if(!redis->dequeue(QUERYQUEUE, value)){
			fail_time++;
			if(fail_time > 5){
				if(redis->ping()){
					fail_time = 0;
				}else{
					if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
						seErrLogEx(g_logHandle, "[query#%lu] reconnect can't connect to redis %s:%d thread exit"
								,pthread_self(), GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
						return NULL;
					}
				}
			}
			if(sleep_time < MAX_SLEEP_TIME)
				sleep_time += 10;
		}else{
			if(sleep_time > 0)
				sleep_time -= 10;
			TopupInfo *topup_info = new TopupInfo;
			deserialize_topupinfo(value, topup_info);
			shared_ptr<ChannelImpl> channel_handler(new ChannelImpl);
			//查询的订单已经成功
			if(0 == channel_handler->QueryRequest(topup_info)){
				//更新订单状态为成功，并通知
				shared_ptr<ChargeBusiness> chargeBusiness(new ChargeBusiness());
				ConnectionManager *connManager = ConnectionManager::Instance();
				Connection *conn = connManager->CreateConnection();
				chargeBusiness->Init(conn);
				topup_info->status = SUCCESS;
				chargeBusiness->UpdateOrderStatus(topup_info);
				connManager->Recover(conn);
				//加入通知队列
				string success_data;
				serialize_topupinfo(topup_info, success_data);
				redis->enqueue(NOTIFYQUEUE, success_data.c_str());		
				seLogEx(g_logHandle, "[query#%lu] order charge succes push to notify:%s",pthread_self(), success_data.c_str());
			}else{   ///订单状态时失败的订单
				//将要超时的直接更新为成功
				time_t time_now = time(NULL);
				if(time_now - topup_info->create_time >= TIMEOUT_TIME){
					topup_info->status = SUCCESS;
					//更新订单状态为成功，并通知
					shared_ptr<ChargeBusiness> chargeBusiness(new ChargeBusiness());
					ConnectionManager *connManager = ConnectionManager::Instance();
					Connection *conn = connManager->CreateConnection();
					chargeBusiness->Init(conn);
					chargeBusiness->UpdateOrderStatus(topup_info);
					connManager->Recover(conn);
					//加入通知队列
					string success_data;
					serialize_topupinfo(topup_info, success_data);
					redis->enqueue(NOTIFYQUEUE, success_data.c_str());		
					seLogEx(g_logHandle, "[query#%lu] order charge timeout push to notify:%s",pthread_self(), success_data.c_str());
				}else{
					//没有成功的任务重新入队列	
					string success_data;
					serialize_topupinfo(topup_info, success_data);
					redis->enqueue(QUERYQUEUE, success_data.c_str());		
					seLogEx(g_logHandle, "[query#%lu] order charge no finish push back:%s",pthread_self(), success_data.c_str());
				}
			}
			delete topup_info;
		}
		if(sleep_time > 0)
			usleep(sleep_time);
		sleep(1);
	}
}

void *notify(void *arg){
	int sleep_time = INITIAL_SLEEP_TIME;//ms	
	int fail_time = 0;
	int retry_times = 5;
	int notify_status = -1;
	shared_ptr<RedisClient> redis(new RedisClient());
	//连接redis
	if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
		seErrLogEx(g_logHandle, "[notify#%lu] start thread can't connect to redis %s:%d"
				,pthread_self(), GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
		return NULL;
	}
	//循环处理队列
	while(1){
		string value;
		redis->select(1);
		if(!redis->dequeue(NOTIFYQUEUE, value)){
			//5次出队列失败要检查redis的状态，如果ping不通，直接重连
			fail_time++;
			if(fail_time > 5){
				if(redis->ping()){
					fail_time = 0;
				}else{
					if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
						seErrLogEx(g_logHandle, "[notify#%lu] reconnect can't connect to redis %s:%d thread exit"
								,pthread_self(), GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
						return NULL;
					}
				}
			}
			if(sleep_time < MAX_SLEEP_TIME)
				sleep_time += 10;
		}else{
			if(sleep_time > 0)
				sleep_time -= 10;
			TopupInfo *topup_info = new TopupInfo;
			deserialize_topupinfo(value, topup_info);
			shared_ptr<ChannelImpl> channel_handler(new ChannelImpl);
			shared_ptr<ChargeBusiness> chargeBusiness(new ChargeBusiness());
		    ConnectionManager *connManager = ConnectionManager::Instance();
		    Connection *conn = connManager->CreateConnection();
		    chargeBusiness->Init(conn);
		    //topup_info->status = FAILED;
			//检查是否已经通知，有可能重复通知
		    int notify = chargeBusiness->GetNotifyStatus(topup_info->qs_info.coopOrderNo);
		    connManager->Recover(conn);

			//notify失败5次，直接丢弃
			//判断订单的来源，调用相应的接口进行通知,分为天猫和订购用户
			if(notify == 0){
				if(topup_info->interfaceName == "tmall"){
					TopupBase* topupBase = new TopupImpl();
					ConnectionManager *connManager = ConnectionManager::Instance();
					Connection *conn = connManager->CreateConnection();
					if(conn == NULL){
						slog_write(LL_FATAL, "create connection instance failed!");
					}
					topup_info->conn = conn;
					topupBase->Init(topup_info);
					while(notify == 0 && retry_times >= 0 && notify_status != 0){
						notify_status = topupBase->Notify();
					}
					connManager->Recover(conn);
					//销毁充值实例
					delete topupBase;
				}else{
					TopupBase* topupBase = new TopupCustomer();
					ConnectionManager *connManager = ConnectionManager::Instance();
					Connection *conn = connManager->CreateConnection();
					if(conn == NULL){
						slog_write(LL_FATAL, "create connection instance failed!");
					}
					topup_info->conn = conn;
					topupBase->Init(topup_info);
					//重试
					while(notify == 0 && retry_times >= 0 && notify_status != 0){
						notify_status = topupBase->Notify();
					}
					connManager->Recover(conn);
					//销毁充值实例
					delete topupBase;
				}
			}
			delete topup_info;
		}

		if(sleep_time > 0)
			usleep(sleep_time);
		sleep(1);
	}
}
