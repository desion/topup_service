/*************************************************************************
	> File Name: TopupServer.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Fri 07 Feb 2014 05:45:16 PM CST
 ************************************************************************/

#ifndef __TOPUP_HANDLE_MAIN_H
#define __TOPUP_HANDLE_MAIN_H

#include <iostream>
#include "TopupUtils.h"
#include "ConnectionManager.h"
#include "slog.h"
#include "selog.h"

class TopupHandleMain{
	protected:
		virtual int ParseParam(int argc, char ** argv);
		
		virtual void PrintHelp();

		void PrintVersion();
		
		virtual int InitLog();
		
		virtual void GlobalInit();
		
		int TPServe();
		
	public:
		TopupHandleMain():confPath("../conf"), confName("topup.ini"), logPath("../log"), logName("topup_handle") {
		}
		
		virtual ~TopupHandleMain() {
		}
		
		virtual void GlobalDestroy();
		
		virtual int Serve(int argc, char ** argv);

	public:
		const char *confPath;			//配置文件路径
		const char *confName;			//配置文件名
		const char *logPath;			//日志路径
		const char *logName;			//日志文件名

		char log[1024];					//日志缓冲区
		int	log_len;					//日志位置
		ConnectionManager *conn_manager;	//数据库连接池

};

#endif //__TOPUP_SERVER_H
