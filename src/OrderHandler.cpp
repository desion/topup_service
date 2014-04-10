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
#include "OrderHandler.h"

extern LOG_HANDLE g_logHandle; 

void *charge(void *arg){
	int sleep_time = 10;//ms	
	int fail_time = 0;
	RedisClient *redis = new RedisClient();
	if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
		seErrLogEx(g_logHandle, "[charge] start thread can't connect to redis %s:%d", GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
		return NULL;
	}
	while(1){
		string value;
		redis->select(1);
		if(!redis->dequeue("underway", value)){
			fail_time++;
			if(fail_time > 5){
				if(redis->ping()){
					fail_time = 0;
				}else{
					if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
						seErrLogEx(g_logHandle, "[charge] reconnect can't connect to redis %s:%d", GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
						return NULL;
					}
				}
			}
			if(sleep_time < 2000)
				sleep_time += 10;
		}else{
			if(sleep_time > 0)
				sleep_time -= 10;
			TopupInfo *topup_info = new TopupInfo;
			deserialize_topupinfo(value, topup_info);
			ChannelImpl *channel_handler = new ChannelImpl;
			if(0 == channel_handler->ChargeRequest(topup_info)){
				string success_data;
				serialize_topupinfo(topup_info, success_data);
				redis->enqueue("query", success_data.c_str());		
			}else{
				//创建失败,不再返回队列，直接重试
			}
		}
		if(sleep_time > 0)
			usleep(sleep_time);
	}
}

void *query(void *arg){
	int sleep_time = 10;//ms	
	int fail_time = 0;
	RedisClient *redis = new RedisClient();
	if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
		seErrLogEx(g_logHandle, "[query] start thread can't connect to redis %s:%d", GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
		return NULL;
	}

	while(1){
		string value;
		redis->select(1);
		if(!redis->dequeue("query", value)){
			fail_time++;
			if(fail_time > 5){
				if(redis->ping()){
					fail_time = 0;
				}else{
					if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
					}
				}
			}
			if(sleep_time < 2000)
				sleep_time += 10;
		}else{
			if(sleep_time > 0)
				sleep_time -= 10;
			TopupInfo *topup_info = new TopupInfo;
			deserialize_topupinfo(value, topup_info);
			ChannelImpl *channel_handler = new ChannelImpl;
			if(0 == channel_handler->QueryRequest(topup_info)){
				string success_data;
				serialize_topupinfo(topup_info, success_data);
				redis->enqueue("notify", success_data.c_str());		
			}else{
				//将要超时的直接更新为成功
			}
		}
		if(sleep_time > 0)
			usleep(sleep_time);
	}
}

void *notify(void *arg){
	int sleep_time = 10;//ms	
	int fail_time = 0;
	RedisClient *redis = new RedisClient();
	if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
		seErrLogEx(g_logHandle, "[notify] start thread can't connect to redis %s:%d", GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
		return NULL;
	}

	while(1){
		string value;
		redis->select(1);
		if(!redis->dequeue("notify", value)){
			fail_time++;
			if(fail_time > 5){
				if(redis->ping()){
					fail_time = 0;
				}else{
					if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
					}
				}
			}
			if(sleep_time < 2000)
				sleep_time += 10;
		}else{
			if(sleep_time > 0)
				sleep_time -= 10;
			TopupInfo *topup_info = new TopupInfo;
			deserialize_topupinfo(value, topup_info);
			ChannelImpl *channel_handler = new ChannelImpl;
			//notify失败5次，直接丢弃
			if(0 == channel_handler->ChargeRequest(topup_info)){
				string success_data;
				serialize_topupinfo(topup_info, success_data);
				redis->enqueue("query", success_data.c_str());		
			}else{
			}
		}

		if(sleep_time > 0)
			usleep(sleep_time);
	}
}
