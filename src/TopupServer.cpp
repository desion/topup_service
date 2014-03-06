/*************************************************************************
	> File Name: TopupServer.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Thu 16 Jan 2014 10:42:58 AM CST
 ************************************************************************/

#include <iostream>
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <concurrency/ThreadManager.h>
#include <concurrency/PosixThreadFactory.h>
#include <server/TNonblockingServer.h>
#include <server/TThreadPoolServer.h>
#include <mkdirs.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "Topup.h"
#include "TopupService.h"
#include "GlobalConfig.h"
#include "BaseBusiness.h"
#include "glog/logging.h"
#include "TopupServer.h"

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace apache::thrift::concurrency;

using namespace  ::topupinterface;

using boost::shared_ptr;

//系统当前所处的状态值
enum ESysStatus{
	Normal = 0,			//正常
	Suspend,			//暂停服务
	Stop,				//停止
	Resume				//恢复，由暂停到正常的中间状态
};

GlobalConfig* gconf = NULL;			//全局配置
//extern TopupServer *P_TPServer;	//充值服务实例
TopupServer *P_TPServer = NULL;		//充值服务实例
//LOG_HANDLE	service_log;			//日志文件句柄
int isDaemon = 0;
volatile int status = Normal;

//读取启动参数
int TopupServer::ParseParam(int argc, char ** argv){
	if(argc <= 1){
		PrintHelp();
	    return 1;
	}
	char c;
	while ((c = getopt(argc, argv, "f:dc:l:h")) != -1) 
	{
		switch (c) 
		{
			case 'f':
				confName = optarg;
				break;
																			
			case 'c':
				confPath = optarg;
				break;

            case 'l':
                logPath = optarg;
                break;
			case 'd':
				isDaemon = 1;
				break;
																														
			case 'h':
				PrintHelp();
				return 1;
																																																					default:
				printf("user input options error! \n");
				PrintHelp();
				exit(EXIT_FAILURE);
				break;
		}
	}

	if (NULL == confPath || NULL == confName) 
	{
		PrintHelp();
		return -1;
	}
	logName = strrchr(argv[0], '/');
	if (logName != NULL)
	{
		++logName;
	}
	else
	{
		logName = argv[0];
	}
	return 0;
}

//帮助信息
void TopupServer::PrintHelp()
{
	printf("Usage:\n");
	printf("    -d  config file path\n");
	printf("    -f  config file name\n");
	printf("    -l  log file dir\n");
	printf("    -h  print this help message\n");
	printf("\n");
}

//初始化log
int TopupServer::InitLog()
{
	//系统日志初始化
	if (!mkdirs((char *) logPath))
    {
		fprintf(stderr, "create logpath[%s/%s] failed\n", logPath, logName);
        exit(EXIT_FAILURE);
    }
	
	if(logPath == NULL || logName == NULL){
		return 1;
	}

	if(logName && *logName){
	   TLogConf log;
	   log.maxLen = 256 * 1024 * 1024;
	   log.minLevel = LL_ALL;
	   log.spec.log2TTY = 1;
	   if(slog_open(logPath, logName, &log)){
	        return 1;
	   }
    }
	//业务日志初始化
	if(seLogInit(&service_log, logPath, gconf->p_tplog_prefix, SLO_SWITCH_BY_DAY | SLO_SWITCH_BY_SIZE, NULL, 1024, 0)){       
		return 1;
	}
	return 0;
}

//全局初始化
void TopupServer::GlobalInit()
{
	gconf = GlobalConfig::Instance();
	char filename[1024];
	snprintf(filename, sizeof(filename), "%s/%s", confPath, confName);
	if(gconf == NULL || !gconf->Init(filename)){
		slog_write(LL_FATAL, "Init conf failed %s", filename);
		exit(EXIT_FAILURE);
	}

	//初始化数据库连接池
	ConnectionManager *conn_manager  = ConnectionManager::Instance();
	printf("user:%s\tpasswd:%s\n", gconf->s_db_userName.c_str(), gconf->s_db_passWord.c_str());
	if(conn_manager == NULL || !conn_manager->Init(gconf->s_db_userName,
				gconf->s_db_passWord, gconf->s_db_connString,
				gconf->n_max_connection, gconf->n_min_connection,
				gconf->n_inc_connection)){
		slog_write(LL_FATAL, "Init database connection pool failed");
		exit(EXIT_FAILURE);
	}
	printf("connManager user:%s\tconnManager passwd:%s\n", conn_manager->m_userName.c_str(), conn_manager->m_passWord.c_str());

	//加载充值服务动态链接库
	if(so_init(&topup_so, gconf->p_so_path, so_topup_reload)){
		slog_write(LL_FATAL, "so load failed %s", gconf->p_so_path);
		exit(EXIT_FAILURE);
	}else{
		slog_write(LL_NOTICE, "so load success %s", gconf->p_so_path);
	}	
}

//回收资源，以及关闭服务相应处理
void TopupServer::GlobalDestroy()
{
	slog_write(LL_NOTICE, "global destory start...");
	//正常关闭日志
	slog_write(LL_NOTICE, "close log...");
	slog_close(0);
}

//启动thrift服务
int TopupServer::TPServe(){
	int port = gconf->n_port;
    shared_ptr<TopupService> handler(new TopupService());
    shared_ptr<TProcessor> processor(new TopupProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(20);
    shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();
	TNonblockingServer server(processor, protocolFactory, port, threadManager);
    //TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);
    server.serve();
    return 0;
}

int TopupServer::Serve(int argc, char ** argv){
	if(ParseParam(argc, argv) != 0){
		return -1;
	}
	//初始化全局配置
	GlobalInit();
	//初始化系统日志
	InitLog();
	//启动thrift服务
	TPServe();
	return 0;
}

int TopupServer::CallLog(){
	if(log_len > 0 && log_len < MAX_LOG_LEN){
		log[log_len] = '\0';
		seLogEx(service_log, "%s", log);
		log_len = 0;
		return log_len;
	}
	return 0;
}

void daemon(void){
	//int fd;
	if(fork() != 0) exit(0);	//parent exit
	setsid();	//crete a new session
	/*
	if((fd = open("/dev/null", O_RDWR)) != -1){
		dup2(fd, STDIN_FILENO);
	    dup2(fd, STDOUT_FILENO);
	    dup2(fd, STDERR_FILENO);
	    if (fd > STDERR_FILENO) close(fd);
	}*/
}

void signalHandler(int sig){
	slog_write(LL_NOTICE, "Received SIGTERM, scheduling shutdown...");
	if(P_TPServer != NULL){
		P_TPServer->GlobalDestroy();
		delete P_TPServer;
	}
	exit(0);
}
//设置信号处理函数
void setupSignalHandlers(void) {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = signalHandler;
	sigaddset(&act.sa_mask, SIGINT);
	sigaddset(&act.sa_mask, SIGTERM);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	return;
}

int main(int argc, char *argv[]){
	//if(isDaemon == 1)
		daemon();
	setupSignalHandlers();
	P_TPServer = new TopupServer();
	return P_TPServer->Serve(argc, argv);
}