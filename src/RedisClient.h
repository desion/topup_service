/*************************************************************************
	> File Name: RedisClient.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Tue 18 Mar 2014 10:17:27 AM CST
 ************************************************************************/

#include <iostream>
#include "hiredis/hiredis.h"
using namespace std;

class RedisClient{
public:
	RedisClient();
	~RedisClient();
	bool connect(string host, int port);
	bool setex(string key, string value, int seconds);
	bool select(int db);
	bool delkey(string key);

private:
	redisContext *redis;
	char *cmd;
};

