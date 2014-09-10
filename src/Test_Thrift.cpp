/*************************************************************************
 *
 * 充值用FCGI接口
 * author:desionwang
 * time:2014-01-28
 * FCGI直接通过thrift调用充值服务
 *
 *************************************************************************/

#include "fcgi_config.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

extern char **environ;		//FCGI所有环境变量

#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <concurrency/ThreadManager.h>
#include <concurrency/PosixThreadFactory.h>
#include <server/TNonblockingServer.h>
#include <server/TThreadPoolServer.h>
#include "HttpClient.h"
#include "Topup.h"
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace apache::thrift::concurrency;

using namespace  ::topupinterface;
using boost::shared_ptr;


#define MAX_QUERY_LENGTH	2048	//POST参数最大长度
#define NOPARAM_ERR			101		//没有post参数
#define POST_ERR			102		//content length和读到的psot参数不能对应
#define POST_TOOLONG_ERR	103		//post参数过长
#define TOPUP_VERSION				"V1.0@21040129"


vector<pair<string, int> > serviceList;		//充值服务配置列表

TopupClient *m_topupClient;					//thrift client handler
shared_ptr<TTransport> sp_transport;		//thrift连接句柄

//CGI使用配置文件加载，格式：ip\tport
int loadServiceList(const char *path, vector<pair<string, int> > &serviceList)
{
	if(path == NULL)
		return -1;
    int fp = open(path, O_RDONLY);
    if (fp < 0) {
        #ifdef DEBUG
        cerr << "[Error] can not open server IP address file!\n";
        #endif
		return -2;
    }
	char buf[1024] = {0};

	int size = read(fp,buf,1024);
	if(size <= 0)
		return -3;

    char ip[32];
    int port;
	char *p = buf;
    char *s= buf;
	while((p = strstr(p, "\n")) != NULL){
		*p = '\0';
		sscanf(s, "%s\t%d", ip, &port);
		p += 1;
		s = p;
		serviceList.push_back(pair<string, int>(ip, port));
	}
    #ifdef DEBUG
    cout << "[DEBUG] service number: " << serviceList.size() << '\n';
    #endif
	return 0;
}


//打开thrift连接
bool OpenProtocol(const string& host,const int port)
{
	shared_ptr<TSocket> tsocket(new TSocket(host, port));
	tsocket->setConnTimeout(1000);
	tsocket->setRecvTimeout(2000000);
	tsocket->setSendTimeout(2000000);
    //shared_ptr<TTransport> socket(new TSocket(host, port));	
    shared_ptr<TTransport> socket(tsocket);
	//sp_transport =  shared_ptr<TTransport>(new TBufferedTransport(socket));
	sp_transport =  shared_ptr<TTransport>(new TFramedTransport(socket));
    shared_ptr<TProtocol> protocol(new TBinaryProtocol(sp_transport));
	m_topupClient = new TopupClient(protocol);
    try {
        sp_transport->open();
        return true;
    } catch (TException &tx) {
		cout << tx.what() << endl;
        //sp_transport->close();
        return false;
    }
}

//关闭thrift连接
bool CloseProtocol(){
	if(sp_transport){
		sp_transport->close();
		return true;
	}
	return false;
}

// 输出xml结果，FCGI端处理错误使用
static void EchoXML(int status){
	printf("<?xml version=\"1.0\" encoding=\"GBK\"?>\n");
	printf("<topupResult>\n");
	printf("<status>%d</status>", status);
	printf("</topupResult>\n");
}


bool TEST_NORMAL_CHARGE(int num, char* req_buffer)
{
	printf("--------------------正常测试------------------\n");
	map<string, string, cmpKeyAscii> entitys;
	char query[2048] = {0};
	uint64_t orderNO = num;
	orderNO = (orderNO << 32);
	orderNO = orderNO + (int)time(NULL);
	uint64_t phone_no = 13693111111 + num;
	sprintf(query, "coopId=928707139&tbOrderNo=%lu&cardId=10001&cardNum=1&customer=%lu&sum=99.02&notifyUrl=http://123.126.54.32/notify.do&tbOrderSnap=99.02|101|测试样例|测试加密",orderNO, phone_no);
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

	strcpy(req_buffer, encode_query);
	return true;
}

int main (int argc, char* argv[])
{
	// 读取请求的post参数缓存
    char buffer[MAX_QUERY_LENGTH];
	char md5str[33] = {0};
	// 加载服务列表信息
	loadServiceList("./service.ini", serviceList);
	if(serviceList.size() <= 0){
		cerr << "can't load serviceList\n";
		return -1;
	}
	for(int i = 0; i < 10000; i++){
		time_t time_now;
		time(&time_now);
		TEST_NORMAL_CHARGE(i, buffer);
		OpenProtocol(serviceList[0].first, serviceList[0].second);
		TopupRequest req;
		req.query = string(buffer);
		int read_len = strlen(buffer);
		if(str2md5(buffer, read_len, md5str)){
			EchoXML(POST_ERR);
			CloseProtocol();
			continue;
		}
		req.checksum = string(md5str);
		req.version = string(TOPUP_VERSION);
		req.ip = string("127.0.0.1");
		req.uri = string("/tmall/pay");
		req.itimestamp = (uint32_t)time_now;
		string ret;
		m_topupClient ->SendRequest(ret, req);
		CloseProtocol();
		if(ret.empty()){
			EchoXML(POST_ERR);
		}
		//直接输出后台返回的处理结果（XML）
		printf("%s\n", ret.c_str());
	}
    return 0;
}
