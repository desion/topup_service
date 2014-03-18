/*************************************************************************
	> File Name: RedisClient.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Tue 18 Mar 2014 10:26:26 AM CST
 ************************************************************************/

#include "RedisClient.h"
#include "stdlib.h"
#include "string.h"
using namespace std;

#define CMD_LENGTH 2048

RedisClient::RedisClient(){
	cmd = (char *)malloc(CMD_LENGTH * sizeof(char));
	if(cmd == NULL){
		fprintf(stderr, "[RedisClient] cmd can't alloc");
	}	
}

RedisClient::~RedisClient(){
	if(redis){
		redisFree(redis);
	}
	if(cmd){
		free(cmd);
	}
}

bool RedisClient::connect(string host, int port){
	redis = redisConnect(host.c_str(), port);
	if(redis->err){
		fprintf(stderr, "connect to %s:%d failed %d", host.c_str(), port, redis->err);
		return false;
	}
	return true;
}

bool RedisClient::setex(string key, string value, int seconds){
	memset(cmd, 0, CMD_LENGTH);
	snprintf(cmd, CMD_LENGTH, "SETEX %s %d %s", key.c_str(), seconds, value.c_str());
	redisReply *reply = (redisReply*)redisCommand(redis, cmd);
	if(reply == NULL){
		fprintf(stderr, "reply is NULL");
		return false;
	}
	if( !(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK")==0))  
    {  
        fprintf(stderr, "Failed to execute command[%s]\n",cmd);  
        freeReplyObject(reply);
		return false;
	}
	freeReplyObject(reply);
	return true;
}

bool RedisClient::select(int db){
	int len = 0;
	len = snprintf(cmd, CMD_LENGTH, "SELECT %d", db);
	cmd[len] = '\0';
	redisReply *reply = (redisReply*)redisCommand(redis, cmd);
    if(reply == NULL){
	    fprintf(stderr, "reply is NULL");
		return false;
    }
    if( !(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK")==0))  
    {  
        fprintf(stderr, "Failed to execute command[%s]\n",cmd);  
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;	
}

bool RedisClient::delkey(string key){
	int len = 0;
	len = snprintf(cmd, CMD_LENGTH, "DEL %s", key.c_str());
	cmd[len] = '\0';
	redisReply *reply = (redisReply*)redisCommand(redis, cmd);
    if(reply == NULL){
	    fprintf(stderr, "reply is NULL");
		return false;
    }
    if( !(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str,"OK")==0))  
    {  
        fprintf(stderr, "Failed to execute command[%s]\n",cmd);  
        freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;	
}
