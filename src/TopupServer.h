/*************************************************************************
	> File Name: TopupServer.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Fri 07 Feb 2014 05:45:16 PM CST
 ************************************************************************/

#ifndef __TOPUP_SERVER_H
#define __TOPUP_SERVER_H

#include <iostream>
#include "TopupUtils.h"
#include "ConnectionManager.h"
#include "slog.h"
#include "selog.h"

using namespace std;

class TopupServer{
	protected:
		virtual int ParseParam(int argc, char ** argv);
		
		virtual void PrintHelp();

		void PrintVersion();
		
		virtual int InitLog();
		
		virtual void GlobalInit();
		
		int TPServe();
		
	public:
		TopupServer():confPath("../conf"), confName("topup.ini"), logPath("../log"), logName("topup") {
			pthread_mutex_init(&seq_lock, NULL);
			seq_id = 0;
		}
		
		virtual ~TopupServer() {
			pthread_mutex_destroy(&seq_lock);
		}
		
		virtual void GlobalDestroy();
		
		virtual int Serve(int argc, char ** argv);

		virtual int CallLog(TopupInfo* topup_info); 

		virtual void SetSeqId(uint32_t *seq_id);

	public:
		const char *confPath;			//配置文件路径
		const char *confName;			//配置文件名
		const char *logPath;			//日志路径
		const char *logName;			//日志文件名

		char log[1024];					//日志缓冲区
		int	log_len;					//日志位置
		SoBase topup_so;				//TMall充值服务动态链接库
		SoBase customer_so;				//放货动态链接库
		SoBase channel_so;				//渠道充值动态链接库
		ConnectionManager *conn_manager;	//数据库连接池
		uint32_t seq_id;				//系统序列号，用于标记请求
		pthread_mutex_t seq_lock;
		LOG_HANDLE  service_log;		//日志文件句柄
};

extern TopupServer *P_TPServer;


#endif //__TOPUP_SERVER_H
