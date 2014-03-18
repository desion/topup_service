/*************************************************************************
	> File Name: HttpClient.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Wed 22 Jan 2014 02:49:24 PM CST
 ************************************************************************/

#ifndef __HTTPCLIENT_H_
#define __HTTPCLIENT_H_

#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <map>
#include <iostream>
#include <openssl/md5.h>
#include <stdlib.h>
#include "TopupServer.h"
#include <tinyxml/tinyxml.h>
#include <tinyxml/tinystr.h>
#include "TopupInterface_types.h"
#include "dllcall.h"
#include <iconv.h>
using namespace std;
#define MAX_LOG_LEN 1024

struct cmp_str
{
    bool operator()(char const *a, char const *b)
    {
         return strcmp(a, b) < 0;
    }
};

struct cmpKeyAscii
{
	bool operator()(const string& k1, const string& k2)
	{
		return k1 < k2;
	}
};

typedef size_t (*PARSE_FUNCTION)(void* buffer, size_t size, size_t nmemb, void* user_p);

bool httpclent_perform(const char *url, const char *params, PARSE_FUNCTION, void *wdata);

bool parse_params(const char *query_str, map<std::string, std::string, cmpKeyAscii>*);

int str2md5(const char* src, int len, char *md5str);

bool parse_query(const char *query_str, TopupInfo *topup_info);

int change_code(const char* from, const char* to, char *inbuf, size_t *insize, char *outbuf, size_t *outsize);

int url_encode(const char* str, const int strSize, char* result, const int resultSize);

int url_decode(const char* str, const int strSize, char* result, const int resultSize);

int url_signature(const char* url,const char* private_key, char *md5str);

size_t parse_tmall_response(void *buffer, size_t size, size_t count, void *args);

#endif //__HTTPCLIENT_H_

