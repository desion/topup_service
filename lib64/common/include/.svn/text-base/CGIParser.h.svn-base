/************************************************************
 * Functions used in creating CGI, see CGIParser.c for detail
 * Author: Lubing
 * Date: May 19, 1999
 * 2003.4增加了对Apache模块的支持，编译时增加-DAPACHE_MODULE
 * 就可以了。
 ************************************************************/

#ifndef _CGI_PARSER_H
#define _CGI_PARSER_H

/* CGI参数结构定义，内部存储使用 */
typedef struct {
	char *szName;   /* 参数名称 */
	char *szValue;  /* 参数的值 */
	char *filename; /* 上传文件名，分析Multipart时使用 */
	char *type;     /* 上传文件MIME类型，分析Multipart时使用 */
	int len;        /* 上传文件的长度，分析Multipart时使用 */
} QUERY_ITEM;

/* 分析CGI参数保存的结构，需要在各个函数中作为参数传递 */
typedef struct {
	char method[10];          /* 请求方法 */
	int  content_length;      /* 请求体的长度，POST方式 */
	char* content_type;       /* 请求的MIME类型 */
	QUERY_ITEM* pParsedQuery; /* for storing parsed name/value pairs */
	int pairNums;             /* number of name/value pairs */
	char* pQueryString;       /* 临时内存 */
} CGIParser;

#ifdef APACHE_MODULE
#include "httpd.h"
#endif
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 分析CGI参数：GET方式分析QUERY_STRING，POST方式分析POST请求体
 * IN:
 *  parser: CGIParser结构；传递前先进行初始化[memset(&parser, 0, sizeof(CGIParser);]
 *          (为了支持多次调用本函数，把初始化放在了调用外面做)
 *  r:      apache模块专用，是apache模块的请求结构，见apache源码的http.h
 * OUT:
 *  CGI参数的个数
 * 【注意】如果有CGI参数，在最后请调用ReleaseQueryBuffer释放临时申请的内存
 */
#ifndef APACHE_MODULE
int ParseQuery(CGIParser* parser);
#else
int ParseQuery(CGIParser* parser, request_rec* r);
#endif

/**
 * 把字符串作为QUERY_STRING进行CGI参数分析，可以作为调试或者分析响应结果
 * IN:
 *  parser: CGIParser结构；传递前先进行初始化[memset(&parser, 0, sizeof(CGIParser);]
 *          (为了支持多次调用本函数，把初始化放在了调用外面做)
 *  str:    字符串。【注意】字符串的内容会被修改，如果有它用，请另外保存
 * OUT:
 *  CGI参数的个数
 * 【注意】如果有CGI参数，在最后请调用ReleaseQueryBuffer释放临时申请的内存
 */
int ParseString(CGIParser* parser, char* str);

/**
 * 取CGI参数。
 * IN:
 *  parser: CGIParser结构；是调用ParseQuery或者ParseString时使用过的。
 *  szName: 参数名称
 * OUT:
 *  NULL: 没有对应的参数值
 *  非NULL: 参数对应的值
 */
char* Param(const CGIParser* parser, const char *szName);

/**
 * 取CGI参数。当CGI参数中包含多个同名参数时可调用本函数。
 * IN:
 *  parser: CGIParser结构；是调用ParseQuery或者ParseString时使用过的。
 *  szName: 参数名称
 *  result: 是一个指针数组，用于存放多个结果
 *  size:   result数组的项数
 * OUT:
 *  实际存放在result中的项数
 */
int   Params(const CGIParser* parser, const char* szName, char* result[], int size);

/**
 * 取上传文件。需要用POST方法的Multipart方式传送。
 * IN:
 *  parser(IN):    CGIParser结构；是调用ParseQuery使用过的。
 *  szName(IN):    参数名称
 *  filename(OUT): 存放上传文件在客户端的文件名，如果没有上传文件，调用后存放NULL
 *  result(OUT):   存放上传文件内容的起始地址，如果没有上传文件，调用后存放NULL
 *  type(OUT):     存放上传文件的MIME类型，如果没有上传文件，调用后存放NULL
 *  size(OUT):     存放上传文件的大小，如果没有上传文件，调用后存放0
 * OUT:
 *  0: 没有指定参数
 *  非0: 找到所指定的参数
 */
int   GetFile(const CGIParser* parser, const char* szName, char** filename, char** result, char** type, int* size);

/**
 * 测试是否包含某个参数
 * IN:
 *  parser:    CGIParser结构；是调用ParseQuery或者ParseString时使用过的。
 *  szName:    参数名称
 * OUT:
 *  0: 没有指定参数
 *  非0: 存在所指定的参数
 */
int   HasParam(const CGIParser* parser, const char *szParam);

/**
 * 输出所有的参数及其值，仅用于测试
 * IN:
 *  parser: CGIParser结构；是调用ParseQuery或者ParseString时使用过的。
 *  out:    用于输出的文件指针，如stdout
 *  deli:   各参数间的分隔符，如"\n"、"<br>"等，可以给NULL
 */
void  DumpParams(const CGIParser* parser, FILE* out, const char* deli);

/**
 * 释放调用ParseQuery或ParseString时申请的内存空间
 * IN:
 *  parser: CGIParser结构；是调用ParseQuery或者ParseString时使用过的。
 */
void  ReleaseQueryBuffer(CGIParser* parser);

/**
 * 取请求方法
 * IN:
 *  parser: CGIParser结构；是调用ParseQuery或者ParseString时使用过的。
 * OUT:
 *  方法名，可能为NULL
 */
char* GetMethod(const CGIParser* parser);

#ifdef __cplusplus
}
#endif

#endif
