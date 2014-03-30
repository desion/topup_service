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
		char *p_tmall_path;			//充值服务动态链接库路径
		char *p_customer_path;		//放货用动态链接库路径
		char *p_channel_path;		//充值渠道动态链接库

		const char* private_key;
		const char* coopid;	

		//手拉手
		char *p_sls_interface;
		char *p_sls_query_url;
		char *p_sls_charge_url;
		char *p_sls_balance_url;

		//来来往往
		char *p_llww_interface;
		char *p_llww_query_url;
		char *p_llww_charge_url;
		char *p_llww_balance_url;

		//易宝
		char *p_yeepay_interface;
		char *p_yeepay_query_url;
		char *p_yeepay_charge_url;
		char *p_yeepay_balance_url;

		string s_redis_ip;
		int n_redis_port;
		int n_redis_timeout;

		map<string, string> errors;

		int n_charge_thread;
		int n_query_thread;
		int n_notify_thread;

	private:
		static GlobalConfig m_global_config;
};

#endif //__GLOBALCONFIG_H__
