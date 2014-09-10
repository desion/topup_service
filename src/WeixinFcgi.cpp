/*************************************************************************
 *
 * 微信公众平台接口
 * author:desionwang
 * time:2014-01-28
 * 主要用于微信接口
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
#include "RedisClient.h"

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


const string userName = "zkcl";
const string password = "zkcl";
unsigned int maxConn=5;
unsigned int minConn=1;
unsigned int incrConn=2;
						 
//oracle::occi::Environment *env = oracle::occi::Environment::createEnvironment("ZHS16GBK","UTF8");
									         
/*
shared_ptr<RedisClient> redis(new RedisClient());
if(!redis->connect(GlobalConfig::Instance()->s_redis_ip, GlobalConfig::Instance()->n_redis_port)){
	        seErrLogEx(g_logHandle, "[notify#%lu] start thread can't connect to redis %s:%d"
					                ,pthread_self(), GlobalConfig::Instance()->s_redis_ip.c_str(),GlobalConfig::Instance()->n_redis_port);
			        return NULL;
					    }
	                    redis->select(1);
*/
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
	shared_ptr<TSocket> tsocket(new TSocket(host, port));
	tsocket->setConnTimeout(1000);
	tsocket->setRecvTimeout(2000);
	tsocket->setSendTimeout(2000);
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
	printf("<status>%d</status>\n", status);
	printf("</topupResult>");
}

static void EchoResponse(const char *to_user,const char* from_user,const char *content){
	const char* msg_type = "text";
	printf("<xml>\
			<ToUserName><![CDATA[%s]]></ToUserName>\
			<FromUserName><![CDATA[%s]]></FromUserName>\
			<CreateTime>%d</CreateTime>\
			<MsgType><![CDATA[%s]]></MsgType>\
			<Content><![CDATA[%s]]></Content>\
			<FuncFlag>0</FuncFlag> \
			</xml>",to_user, from_user, time(NULL), msg_type, content);
}

#define VERIFY_SQL "UPDATE USER_TBL SET OPEN_ID = ? WHERE NAME = ?"

int main ()
{
	setenv("ORACLE_BASE", "/data/db/oracle", 1);
	setenv("ORACLE_HOME", "$ORACLE_BASE/ora11", 1);
	setenv("ORACLE_SID", "zkcl", 1);
	// 读取请求的post参数缓存
    char buffer[MAX_QUERY_LENGTH];
	char md5str[33] = {0};
	char randbuf[7] = {0};
	char connBuf[128] = {0};
	// 加载服务列表信息
	loadServiceList("./service.ini", serviceList);
	if(serviceList.size() <= 0){
		cerr << "can't load serviceList\n";
		return -1;
	}

	string db_host = serviceList[1].first;
	int db_port = serviceList[1].second;
	string redis_host = serviceList[2].first;
	int redis_port = serviceList[2].second;

	sprintf(connBuf, "//%s:%d/zkcl", db_host.c_str(), db_port);

	//建立连接池
	//ConnectionPool *connPool=env->createConnectionPool(userName, password, connBuf, minConn, incrConn);
	
    while (FCGI_Accept() >= 0) {
		// 获取content长度，以此来决定读取的参数长度
        char *contentLength = getenv("CONTENT_LENGTH");
		char *ipstr = getenv("REMOTE_ADDR");
		char *request_method = getenv("REQUEST_METHOD");
		// 获取请求的URI信息，用来区分request的接口信息
		char *uri = getenv("REQUEST_URI");
		char *query_string = getenv("QUERY_STRING");
		map<string,string, cmpKeyAscii> formData;
		map<string,string, cmpKeyAscii>::iterator iter;

		if(strcmp(request_method, "GET") == 0 && query_string != NULL){
			printf("Content-type: text/plain\r\nStatus: 200 OK\r\n\r\n");
			parse_params(query_string, &formData);
			if((iter = formData.find("echostr")) != formData.end()){
				string echoStr = iter->second;	
				printf("%s", echoStr.c_str());
			}
		}else{
			printf("Content-type: text/xml\r\nStatus: 200 OK\r\n\r\n");
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
				TiXmlDocument doc;
			    //if(!doc.Parse((const char*)buffer)){
				//	continue;
			    //}
			    doc.Parse((const char*)buffer);
			    TiXmlHandle docHandle(&doc);
			    TiXmlElement* from_user = docHandle.FirstChild("xml").FirstChild("FromUserName").ToElement();
			    TiXmlElement* to_user = docHandle.FirstChild("xml").FirstChild("ToUserName").ToElement();
			    TiXmlElement* key_word = docHandle.FirstChild("xml").FirstChild("Content").ToElement();
				const char *str_from_user, *str_to_user, *str_key_word;

			    if(from_user != NULL && to_user != NULL && key_word != NULL){
					str_from_user = from_user->GetText();
					str_to_user = to_user->GetText();
					str_key_word = key_word->GetText();
					/*
					if(strstr(str_key_word,"verify ") != NULL){
						const char *user_str = str_key_word + 7;
						if(strlen(user_str) <= 5){
							str_key_word = "注册验证失败！";
							EchoResponse(str_from_user, str_to_user, str_key_word);
							continue;
						}
						//从连接池获取连接
						Connection *conn=connPool->createConnection(userName,password);
						Statement *stmt = conn->createStatement(VERIFY_SQL);
						stmt->executeUpdate();
						conn->terminateStatement(stmt);
						connPool->terminateConnection(conn);	
						str_key_word = "注册验证失败！";
						EchoResponse(str_from_user, str_to_user, str_key_word);
						continue;
					}
					*/
					if(strcmp(str_key_word,"login") == 0){
						shared_ptr<RedisClient> redis(new RedisClient());
						if(!redis->connect(redis_host, redis_port)){
							str_key_word = "注册验证失败！";
							EchoResponse(str_from_user, str_to_user, str_key_word);
							continue;
						}
						redis->select(2);
						srand((unsigned)time(NULL));
						uint32_t rand_num = rand() % (899999) + 1000000;
						snprintf(randbuf, 7 , "%d", rand_num);
						redis->setex(str_from_user, randbuf, 60);
						str_key_word = randbuf;
						EchoResponse(str_from_user, str_to_user, str_key_word);
						continue;
					}else if(strstr(str_key_word,"verify ") != NULL){
						OpenProtocol(serviceList[0].first, serviceList[0].second);
						ManageRequest mreq;
						mreq.cmd = 10;
						const char* userId = str_key_word + 7;
						if(strlen(userId) < 2){
							str_key_word = "注册验证失败！";
						    EchoResponse(str_from_user, str_to_user, str_key_word);
						    continue;
						}
						mreq.key = string(userId);
						mreq.value = str_from_user;
						int ret = m_topupClient->Admin(mreq);
						if(ret == 1){
							str_key_word = "没有此用户！";
						    EchoResponse(str_from_user, str_to_user, str_key_word);
						    continue;
						}else if(ret == 2){
							str_key_word = "该用户已经验证，请联系管理员！";
						    EchoResponse(str_from_user, str_to_user, str_key_word);
						    continue;
						}else{
							str_key_word = "用户验证成功，请联系管理员解锁！";
						    EchoResponse(str_from_user, str_to_user, str_key_word);
						    continue;
						}
					}
				}
			}
		}
    } /* while */
    return 0;
}
