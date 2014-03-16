/*************************************************************************
	> File Name: Test_HttpClient.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Wed 22 Jan 2014 03:44:47 PM CST
 ************************************************************************/

#include "HttpClient.h"
#include <assert.h>
#include "GlobalConfig.h"
#include "ConnectionManager.h"

using namespace std;

size_t debug(void *buffer, size_t size, size_t count, void *user_p)
{
	fprintf(stdout, "%s\n", (char*)buffer);
	return size * count;
}

bool TEST_SIGN()
{
	printf("--------------------TEST_SIGN------------------\n");
	map<string, string, cmpKeyAscii> entitys;
	const char * query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&cardNum=1&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	bool parse_ret = parse_params(query, &entitys);
	assert(parse_ret == true);
	char md5[33] = {0};
	char signStr[2048] = {0};
	int len = 0;
	map<string, string>::iterator it = entitys.begin();
	for(;it != entitys.end(); ++it){
		if(strcmp("sign", it->first.c_str()) == 0){
			continue;
		}
		printf("PARAM:%s:%s\n", it->first.c_str(), it->second.c_str());
		len += sprintf(signStr + len, "%s%s", it->first.c_str(), it->second.c_str());
    }
    len += sprintf(signStr + len, "529d9ce791e47401de40233e26d954c6");
	printf("SIGN STRING:%s\n", signStr);
    str2md5(signStr,len, md5);
	assert(strlen(md5) == 32);
	printf("MD5:%s\n", md5);
	return true;
}

bool TEST_NORMAL_CHARGE()
{
	printf("--------------------正常测试------------------\n");
	map<string, string, cmpKeyAscii> entitys;
	const char * query = "coopId=928707139&tbOrderNo=20140303231300&cardId=10001&cardNum=1&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&tbOrderSnap=99.02|101|测试样例|测试加密";
	char buf[2048];
	char inbuf[2048] = {0};
	char param[256] = {0};
	strcpy(inbuf, query);
	size_t insize = strlen(inbuf);
	size_t outsize = 2048;
	change_code("UTF-8", "GBK", inbuf, &insize, buf, &outsize);
	printf("转gbk:%s\n", buf);
	bool parse_ret = parse_params(buf, &entitys);
	assert(parse_ret == true);

	char encode_query[2048] = {0};
	int len = 0;
	
	
	map<string, string>::iterator it = entitys.begin();
	for(;it != entitys.end(); ++it){
		url_encode(it->second.c_str(), strlen(it->second.c_str()), param, 256);
		printf("%s:%s:%s\n", it->first.c_str(),it->second.c_str(), param);
		it->second = string(param);
		len += sprintf(encode_query+len, "%s=%s&", it->first.c_str(), param);
	}
	encode_query[len -1] = '\0';
	printf("encode_query:%s\n", encode_query);

	
	char md5[33] = {0};
	char signStr[2048] = {0};
	len = 0;
	map<string, string, cmpKeyAscii> decode_entitys;
	parse_ret = parse_params(encode_query, &decode_entitys);
	it = decode_entitys.begin();
	for(;it != decode_entitys.end(); ++it){
		if(strcmp("sign", it->first.c_str()) == 0){
			continue;
		}
		printf("PARAM:%s:%s\n", it->first.c_str(), it->second.c_str());
		len += sprintf(signStr + len, "%s%s", it->first.c_str(), it->second.c_str());
    }
    len += sprintf(signStr + len, "529d9ce791e47401de40233e26d954c6");
    printf("Sign String:%s\n", signStr);
	str2md5(signStr,len, md5);
    assert(strlen(md5) == 32);
    printf("MD5:%s\n", md5);

	len = strlen(encode_query);
	len += sprintf(encode_query + len,"&sign=%s", md5);
	encode_query[len] = '\0';
	printf("正确url:%s\n", encode_query);
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", encode_query, &debug);
	
	return true;
}

bool TEST_LAKEPARAM_CHARGE()
{
	printf("--------------------缺少参数测试------------------\n");
	string query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&cardNum=0&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "tbOrderNo=20140303231300&cardId=101&cardNum=1&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&cardId=101&cardNum=1&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&tbOrderNo=20140303231300&cardNum=1&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&cardNum=1&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&cardNum=1&customer=13693555577&notifyUrl=http://123.126.54.32/notify.do&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&cardNum=1&customer=13693555577&sum=99.02&sign=123456789&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&cardNum=1&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&tbOrderSnap=99.02|101|测试样例|测试加密";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	query = "coopId=928707139&tbOrderNo=20140303231300&cardId=101&cardNum=1&customer=13693555577&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&sign=123456789";
	httpclent_perform("http://127.0.0.1/fcgi-bin/topup.fcg", query.c_str(), &debug);
	return true;
}

bool TEST_ERR_CHARGE()
{
	return true;
}

bool TEST_SIGNNOMATCH_CHARGE()
{
	return true;
}

bool TEST_QUERY_ORDER(){
	printf("--------------------订单查询测试------------------\n");
	map<string, string, cmpKeyAscii> entitys;
	char md5[33] = {0};
	char signStr[2048] = {0};
	const char *query_ori = "coopId=928707139&tbOrderNo=20140303231300";
	const char *query = "coopId=928707139&tbOrderNo=20140303231300&sign=111";
	bool parse_ret = parse_params(query, &entitys);
	map<string, string>::iterator it = entitys.begin();
	int len = 0;
    for(;it != entitys.end(); ++it){
        if(strcmp("sign", it->first.c_str()) == 0){
            continue;
        }
        printf("PARAM:%s:%s\n", it->first.c_str(), it->second.c_str());
        len += sprintf(signStr + len, "%s%s", it->first.c_str(), it->second.c_str());
    }
    len += sprintf(signStr + len, "529d9ce791e47401de40233e26d954c6");
    printf("Sign String:%s\n", signStr);
    str2md5(signStr,len, md5);
	len = 0;
	len += sprintf(signStr + len,"%s", query_ori);
	len += sprintf(signStr + len,"&sign=%s", md5);
	httpclent_perform("http://127.0.0.1/fcgi-bin/query.fcg", signStr, &debug);
	return true;
}

bool TEST_CONNECTION(){
	GlobalConfig *gconf = GlobalConfig::Instance();
    if(gconf == NULL || !gconf->Init("../conf/topup.ini")){
	       exit(EXIT_FAILURE);
    }
	ConnectionManager* conn_manager  = ConnectionManager::Instance();
    printf("user:%s\tpasswd:%s\n", gconf->s_db_userName.c_str(), gconf->s_db_passWord.c_str());
    if(conn_manager == NULL || !conn_manager->Init(gconf->s_db_userName,                                                                                                                    
	             gconf->s_db_passWord, gconf->s_db_connString,
	             gconf->n_max_connection, gconf->n_min_connection,
	               gconf->n_inc_connection)){
	       exit(EXIT_FAILURE);
    }	
	Connection *conn = conn_manager->CreateConnection();
	Statement *stmt = conn->createStatement("SELECT ID,ZONE,VALUE,OPERATOR FROM PRODUCT_TBL WHERE ID = :1 AND STATUS != 3");
	stmt->setString(1, "10001");
    ResultSet *rs = stmt->executeQuery();
    while (rs->next())
   {
		string id = rs->getString(1);
		cout << id << endl;
    }
    stmt->closeResultSet(rs);
    conn->terminateStatement(stmt);
	return true;
}

int main(int argc, char *argv[]){
	/*
	map<string, string, cmpKeyAscii> entitys;
	if(argc > 1){
		char *url = argv[1];
		for(int i = 0; i < 10; i++)
			httpclent_perform(url, "a=1&b=2&c=3", &debug);
	}

	parse_params("a=1&b=2&c=3", &entitys);
	char md5str[33] = {0};
	str2md5("a=1&b=2&c=3", 11,md5str);
	printf("MD5:%s\n", md5str);
	map<string, string>::iterator it = entitys.begin();
	for(;it != entitys.end(); ++it){
		printf("key:%s\tvalue:%s\n", it->first.c_str(), it->second.c_str());
	}
	*/
	TEST_CONNECTION();
	TEST_SIGN();
	TEST_NORMAL_CHARGE();
	TEST_QUERY_ORDER();
	//TEST_LAKEPARAM_CHARGE();
	return 0;
}

