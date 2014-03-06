/*************************************************************************
	> File Name: GlobalConfig.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Tue 14 Jan 2014 05:19:37 PM CST
	> Singleten Global Configure Class
 ************************************************************************/

#include<iostream>
#include <map>
using namespace std;

#ifndef __GLOBALCONFIG_H__
#define __GLOBALCONFIG_H__

class GlobalConfig{
	enum {MAX_PATH_LEN = 512,  MAX_FILENAME_LEN = 512};
	public:
		static GlobalConfig* Instance(){
			return &m_global_config;
		}

		bool Init(const char* confPath);
	
	public:
		int n_port;					// server port

		///Oracle connection pool params
		string s_db_userName;
		string s_db_passWord;
		string s_db_connString;
		int n_max_connection;
		int n_min_connection;
		int n_inc_connection;
		char *p_tplog_prefix;		//业务日志前缀
		char *p_so_path;			//充值服务动态链接库路径

		const char* private_key;
		const char* coopid;

		map<string, string> errors;

	private:
		static GlobalConfig m_global_config;
};

#endif //__GLOBALCONFIG_H__