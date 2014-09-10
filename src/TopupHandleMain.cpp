/*************************************************************************
	> File Name: TopupServer.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Thu 16 Jan 2014 10:42:58 AM CST
 ************************************************************************/

#include <iostream>
#include <mkdirs.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "Topup.h"
#include "TopupService.h"
#include "GlobalConfig.h"
#include "BaseBusiness.h"
#include "TopupHandleMain.h"
#include "TopupUtils.h"
#include "RedisClient.h"
#include "OrderHandler.h"
#include "ConnectionManager.h"

using namespace std;

using boost::shared_ptr;


GlobalConfig* gconf = NULL;			//全局配置
extern LOG_HANDLE g_logHandle;		//日志句柄
int isDaemon = 0;					//是否使用后台模式启动
extern int handler_status = Normal;		//服务当前状态

//读取启动参数
int TopupHandleMain::ParseParam(int argc, char ** argv){
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
void TopupHandleMain::PrintHelp()
{
	printf("Usage:\n");
	printf("    -c  config file path\n");
	printf("    -f  config file name\n");
	printf("    -l  log file dir\n");
	printf("    -d  daemon\n");
	printf("    -h  print this help message\n");
	printf("\n");
}

//初始化log
int TopupHandleMain::InitLog()
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
	    log.spec.log2TTY = 0;
	    if(slog_open(logPath, logName, &log)){																																										return 1;
		}
	}

	//业务日志初始化
	if(seLogInit(&g_logHandle, logPath, logName, SLO_SWITCH_BY_DAY | SLO_SWITCH_BY_SIZE, NULL, 1024, 0)){       
		return 1;
	}
	return 0;
}

//全局初始化
//初始化配置，连接池
void TopupHandleMain::GlobalInit()
{
	gconf = GlobalConfig::Instance();
	char filename[1024];
	snprintf(filename, sizeof(filename), "%s/%s", confPath, confName);
	if(gconf == NULL || !gconf->Init(filename)){
		fprintf(stderr, "Init conf failed %s", filename);
		exit(EXIT_FAILURE);
	}

	//初始化数据库连接池
	ConnectionManager *conn_manager  = ConnectionManager::Instance();
	if(conn_manager == NULL || !conn_manager->Init(gconf->s_db_userName,
				gconf->s_db_passWord, gconf->s_db_connString,
				gconf->n_max_connection, gconf->n_min_connection,
				gconf->n_inc_connection)){
		fprintf(stderr, "Init database connection pool failed");
		exit(EXIT_FAILURE);
	}
}

//回收资源，以及关闭服务相应处理
void TopupHandleMain::GlobalDestroy()
{
}


/**启动工作流线程**/
int Run(){
	int num = GlobalConfig::Instance()->n_charge_thread;
	assert(num > 0);
	pthread_t *charge_threads = (pthread_t *)malloc(num * sizeof(pthread_t));
	assert(charge_threads != NULL);
	for(int i = 0; i < num; i++){
		pthread_create(&charge_threads[i], NULL, charge, NULL);
	}
	num = GlobalConfig::Instance()->n_query_thread;
	assert(num > 0);
	pthread_t *query_threads = (pthread_t *)malloc(num * sizeof(pthread_t));
	assert(query_threads != NULL);
	for(int i = 0; i < num; i++){
		pthread_create(&query_threads[i], NULL, query, NULL);
	}

	num = GlobalConfig::Instance()->n_notify_thread;
	assert(num > 0);
	pthread_t *notify_threads = (pthread_t *)malloc(num * sizeof(pthread_t));
	assert(notify_threads != NULL);
	for(int i = 0; i < num; i++){
		pthread_create(&notify_threads[i], NULL, notify, NULL);
	}
	num = GlobalConfig::Instance()->n_charge_thread;
	for(int i = 0; i < num; i++){
		pthread_join(charge_threads[i], NULL);
	}
	num = GlobalConfig::Instance()->n_query_thread;
	for(int i = 0; i < num; i++){
		pthread_join(query_threads[i], NULL);
	}
	num = GlobalConfig::Instance()->n_notify_thread;
	for(int i = 0; i < num; i++){
		pthread_join(notify_threads[i], NULL);
	}
	delete charge_threads;
	delete query_threads;
	delete notify_threads;
	return 0;
} 

///启动服务
int TopupHandleMain::Serve(int argc, char ** argv){
	if(ParseParam(argc, argv) != 0){
		return -1;
	}
	//初始化系统日志
	InitLog();
	//初始化全局配置
	GlobalInit();
	//启动处理线程	
	Run();
	return 0;
}

///启动后台模式
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

///kill信号处理函数
void signalHandler(int sig){
	handler_status = Stop;
	fprintf(stderr, "Received SIGTERM, scheduling shutdown...");
	//exit(0);
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
	daemon();
	setupSignalHandlers();
	TopupHandleMain topup_handle;
	return topup_handle.Serve(argc, argv);
}
