/*************************************************************************
	> File Name: GlobalConfig.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Tue 14 Jan 2014 06:52:43 PM CST
 ************************************************************************/

#include <iostream>
#include <assert.h>
#include "GlobalConfig.h"
#include "iniparser/iniparser.h"
#include "iniparser/dictionary.h"
#include "TopupUtils.h"
#include "slog.h"

using namespace std;

GlobalConfig *global_conf = NULL;				//配置的全局变量

GlobalConfig GlobalConfig::m_global_config;

bool GlobalConfig::Init(const char* confPath){
	dictionary* ini_dict;
	ini_dict = iniparser_load(confPath);
	assert(ini_dict != NULL);
	
	char *db_userName = iniparser_getstring(ini_dict, "DATABASE:USER_NAME",NULL);
	CHECKNULL(db_userName, "DATABASE:USER_NAME IS NULL");
	s_db_userName = db_userName;
	
	char *db_passWord = iniparser_getstring(ini_dict, "DATABASE:PASS_WORD",NULL);
	CHECKNULL(db_passWord, "DATABASE:PASS_WORD IS NULL");
	s_db_passWord = db_passWord;

	char *db_connString = iniparser_getstring(ini_dict, "DATABASE:CONN_STRING",NULL);
	CHECKNULL(db_connString, "DATABASE:CONN_STRING IS NULL");
	s_db_connString = db_connString;

	p_tplog_prefix = iniparser_getstring(ini_dict, "COMMON:TPLOG_PREFIX",NULL);
	CHECKNULL(p_tplog_prefix, "COMMON:TPLOG_PREFIX IS NULL");
	
	p_tmall_path = iniparser_getstring(ini_dict, "COMMON:TMALL_SO",NULL);
	CHECKNULL(p_tmall_path, "COMMON:TMALL_SO IS NULL");
	
	p_customer_path = iniparser_getstring(ini_dict, "COMMON:CUSTOMER_SO",NULL);
	CHECKNULL(p_customer_path, "COMMON:CHANNEL_SO IS NULL");
	
	p_channel_path = iniparser_getstring(ini_dict, "COMMON:CHANNEL_SO",NULL);
	CHECKNULL(p_channel_path, "COMMON:CHANNEL_SO IS NULL");
	
	char *p_error_path = iniparser_getstring(ini_dict, "COMMON:ERRORS",NULL);
	CHECKNULL(p_error_path, "COMMON:ERRORS IS NULL");
	
	FILE *fp = fopen(p_error_path, "r");
	if(fp == NULL){
		return false;
	}
	char buf[1024] = {0};
	vector<char*> field_vec;
	while(fgets(buf, 1023, fp) != NULL){
		buf[strlen(buf)-1] = '\0';
		if(2 != split_string(buf, "\t", field_vec))
	         continue;
		errors.insert(make_pair(string(field_vec[0]), string(field_vec[1])));
	}
	fclose(fp);

	n_port = iniparser_getint(ini_dict, "COMMON:PORT",0);

	n_max_connection = iniparser_getint(ini_dict, "DATABASE:MAX_CONNECTION", 0);
	n_min_connection = iniparser_getint(ini_dict, "DATABASE:MIN_CONNECTION", 0);
	n_inc_connection = iniparser_getint(ini_dict, "DATABASE:INCR_CONNECTION", 0);

	private_key = "529d9ce791e47401de40233e26d954c6";
	coopid="928707139";
	return true;
}
