/************************************************************
 * Functions used in creating CGI, see CGIParser.c for detail
 * Author: Lubing
 * Date: May 19, 1999
 * 2003.4�����˶�Apacheģ���֧�֣�����ʱ����-DAPACHE_MODULE
 * �Ϳ����ˡ�
 ************************************************************/

#ifndef _CGI_PARSER_H
#define _CGI_PARSER_H

/* CGI�����ṹ���壬�ڲ��洢ʹ�� */
typedef struct {
	char *szName;   /* �������� */
	char *szValue;  /* ������ֵ */
	char *filename; /* �ϴ��ļ���������Multipartʱʹ�� */
	char *type;     /* �ϴ��ļ�MIME���ͣ�����Multipartʱʹ�� */
	int len;        /* �ϴ��ļ��ĳ��ȣ�����Multipartʱʹ�� */
} QUERY_ITEM;

/* ����CGI��������Ľṹ����Ҫ�ڸ�����������Ϊ�������� */
typedef struct {
	char method[10];          /* ���󷽷� */
	int  content_length;      /* ������ĳ��ȣ�POST��ʽ */
	char* content_type;       /* �����MIME���� */
	QUERY_ITEM* pParsedQuery; /* for storing parsed name/value pairs */
	int pairNums;             /* number of name/value pairs */
	char* pQueryString;       /* ��ʱ�ڴ� */
} CGIParser;

#ifdef APACHE_MODULE
#include "httpd.h"
#endif
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ����CGI������GET��ʽ����QUERY_STRING��POST��ʽ����POST������
 * IN:
 *  parser: CGIParser�ṹ������ǰ�Ƚ��г�ʼ��[memset(&parser, 0, sizeof(CGIParser);]
 *          (Ϊ��֧�ֶ�ε��ñ��������ѳ�ʼ�������˵���������)
 *  r:      apacheģ��ר�ã���apacheģ�������ṹ����apacheԴ���http.h
 * OUT:
 *  CGI�����ĸ���
 * ��ע�⡿�����CGI����������������ReleaseQueryBuffer�ͷ���ʱ������ڴ�
 */
#ifndef APACHE_MODULE
int ParseQuery(CGIParser* parser);
#else
int ParseQuery(CGIParser* parser, request_rec* r);
#endif

/**
 * ���ַ�����ΪQUERY_STRING����CGI����������������Ϊ���Ի��߷�����Ӧ���
 * IN:
 *  parser: CGIParser�ṹ������ǰ�Ƚ��г�ʼ��[memset(&parser, 0, sizeof(CGIParser);]
 *          (Ϊ��֧�ֶ�ε��ñ��������ѳ�ʼ�������˵���������)
 *  str:    �ַ�������ע�⡿�ַ��������ݻᱻ�޸ģ���������ã������Ᵽ��
 * OUT:
 *  CGI�����ĸ���
 * ��ע�⡿�����CGI����������������ReleaseQueryBuffer�ͷ���ʱ������ڴ�
 */
int ParseString(CGIParser* parser, char* str);

/**
 * ȡCGI������
 * IN:
 *  parser: CGIParser�ṹ���ǵ���ParseQuery����ParseStringʱʹ�ù��ġ�
 *  szName: ��������
 * OUT:
 *  NULL: û�ж�Ӧ�Ĳ���ֵ
 *  ��NULL: ������Ӧ��ֵ
 */
char* Param(const CGIParser* parser, const char *szName);

/**
 * ȡCGI��������CGI�����а������ͬ������ʱ�ɵ��ñ�������
 * IN:
 *  parser: CGIParser�ṹ���ǵ���ParseQuery����ParseStringʱʹ�ù��ġ�
 *  szName: ��������
 *  result: ��һ��ָ�����飬���ڴ�Ŷ�����
 *  size:   result���������
 * OUT:
 *  ʵ�ʴ����result�е�����
 */
int   Params(const CGIParser* parser, const char* szName, char* result[], int size);

/**
 * ȡ�ϴ��ļ�����Ҫ��POST������Multipart��ʽ���͡�
 * IN:
 *  parser(IN):    CGIParser�ṹ���ǵ���ParseQueryʹ�ù��ġ�
 *  szName(IN):    ��������
 *  filename(OUT): ����ϴ��ļ��ڿͻ��˵��ļ��������û���ϴ��ļ������ú���NULL
 *  result(OUT):   ����ϴ��ļ����ݵ���ʼ��ַ�����û���ϴ��ļ������ú���NULL
 *  type(OUT):     ����ϴ��ļ���MIME���ͣ����û���ϴ��ļ������ú���NULL
 *  size(OUT):     ����ϴ��ļ��Ĵ�С�����û���ϴ��ļ������ú���0
 * OUT:
 *  0: û��ָ������
 *  ��0: �ҵ���ָ���Ĳ���
 */
int   GetFile(const CGIParser* parser, const char* szName, char** filename, char** result, char** type, int* size);

/**
 * �����Ƿ����ĳ������
 * IN:
 *  parser:    CGIParser�ṹ���ǵ���ParseQuery����ParseStringʱʹ�ù��ġ�
 *  szName:    ��������
 * OUT:
 *  0: û��ָ������
 *  ��0: ������ָ���Ĳ���
 */
int   HasParam(const CGIParser* parser, const char *szParam);

/**
 * ������еĲ�������ֵ�������ڲ���
 * IN:
 *  parser: CGIParser�ṹ���ǵ���ParseQuery����ParseStringʱʹ�ù��ġ�
 *  out:    ����������ļ�ָ�룬��stdout
 *  deli:   ��������ķָ�������"\n"��"<br>"�ȣ����Ը�NULL
 */
void  DumpParams(const CGIParser* parser, FILE* out, const char* deli);

/**
 * �ͷŵ���ParseQuery��ParseStringʱ������ڴ�ռ�
 * IN:
 *  parser: CGIParser�ṹ���ǵ���ParseQuery����ParseStringʱʹ�ù��ġ�
 */
void  ReleaseQueryBuffer(CGIParser* parser);

/**
 * ȡ���󷽷�
 * IN:
 *  parser: CGIParser�ṹ���ǵ���ParseQuery����ParseStringʱʹ�ù��ġ�
 * OUT:
 *  ������������ΪNULL
 */
char* GetMethod(const CGIParser* parser);

#ifdef __cplusplus
}
#endif

#endif
