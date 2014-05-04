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

#include "fcgi_stdio.h"
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <concurrency/ThreadManager.h>
#include <concurrency/PosixThreadFactory.h>
#include <server/TNonblockingServer.h>
#include <server/TThreadPoolServer.h>
#include "Topup.h"
#include "HttpClient.h"
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
/*
// 切分字符串函数
inline int split_string (char * s, const char * seperator, std::vector<char *> & field_vec)
{
      field_vec.clear();
      if (s == NULL) return -1;
      if (seperator == NULL) return -1;

      int sep_len = strlen(seperator);
      char * p = s;
      field_vec.push_back(p);
      while ((p = strstr(p, seperator)) != NULL)
     {
          *p = '\0';
           p += sep_len;
           field_vec.push_back(p);
      }
	  return field_vec.size();
}*/

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

//打印FCGI所有变量
inline void PrintEnv(char *label, char **envp)
{
    printf("%s:<br>\n<pre>\n", label);
    for ( ; *envp != NULL; envp++) {
        printf("%s\n", *envp);
    }
    printf("</pre><p>\n");
}

//打开thrift连接
bool OpenProtocol(const string& host,const int port)
{
    shared_ptr<TTransport> socket(new TSocket(host, port));
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

int main ()
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
	
    while (FCGI_Accept() >= 0) {
		// 非常重要必须人工输出header信息
		printf("Content-type: text/xml\r\nStatus: 200 OK\r\n\r\n");
		// 获取content长度，以此来决定读取的参数长度
        char *contentLength = getenv("CONTENT_LENGTH");
		char *ipstr = getenv("REMOTE_ADDR");
		// 获取请求的URI信息，用来区分request的接口信息
		char *uri = getenv("REQUEST_URI");
        int len;

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
			EchoXML(NOPARAM_ERR);
			continue;
        }
        else {
			if(len >= MAX_QUERY_LENGTH){
				EchoXML(POST_TOOLONG_ERR);
				continue;
			}
			//读取post参数，最大post参数长度MAX_QUERY_LENGTH
			int read_len = fread(buffer, 1, MAX_QUERY_LENGTH, stdin);
			if (read_len != len){
				EchoXML(POST_ERR);
				continue;
			}
			buffer[read_len] = '\0';
			time_t time_now;
			time(&time_now);
			OpenProtocol(serviceList[0].first, serviceList[0].second);
			TopupRequest req;
			req.query = string(buffer);
			if(str2md5(buffer, read_len, md5str)){
				EchoXML(POST_ERR);
				CloseProtocol();
				continue;
			}
			req.checksum = string(md5str);
			req.version = TOPUP_VERSION;
			req.ip = ipstr;
			req.uri = uri;
			req.itimestamp = (uint32_t)time_now;
			string ret;
			m_topupClient->SendRequest(ret, req);
			CloseProtocol();
			//直接输出后台返回的处理结果（XML）
			printf("%s\n", ret.c_str());
        }

    } /* while */
	delete m_topupClient;
    return 0;
}
