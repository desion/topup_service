/************************************************************
 * Implementation of functions used in creating CGI, see
 * "CGIParser.h"
 *
 * Author: Lubing
 * Date: May 19, 1999
 ************************************************************/

#include "CGIParser.h"
#ifdef APACHE_MODULE
#if APACHE_RELEASE > 10312100
#include "ap_alloc.h"
#else
#include "alloc.h"
#endif
#include "http_protocol.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef APACHE_MODULE
#define CGI_PARSED_FLAG "cgi_parsed_flag"
#endif
#define NOT_MULTIPART -1

#ifdef WIN32
#define strncasecmp _strnicmp
#endif

#define REQUEST_METHOD	"REQUEST_METHOD"
#define GET				"GET"
#define POST			"POST"
#define QUERY_STRING	"QUERY_STRING"
#define CONTENT_LENGTH	"CONTENT_LENGTH"
#define CONTENT_TYPE    "CONTENT_TYPE"
#define MULTIPART       "multipart/form-data; boundary="
#define DISPOSITION     "Content-Disposition: form-data; name="
#define MULTIPART_TYPE  "Content-Type: "

#ifndef APACHE_MODULE
/*************************************************
 GetQueryString:
  Get Query String of "GET" or "POST".
  the data is fetched from enviroment.
 NOTE: this function used privately in this module
 *************************************************/
static int GetQueryString(CGIParser* parser){
	char *method;
	char *s;
	int totalRead, read, len;

	parser->content_type = getenv(CONTENT_TYPE);
	if((method=getenv(REQUEST_METHOD)) == NULL)return 0;
	if(!strcmp(method, GET)){
		strcpy(parser->method, GET);
		s = getenv(QUERY_STRING);
		if(!s)return 0;
		parser->pQueryString = (char*)malloc(strlen(s)+1);
		if(parser->pQueryString != NULL) {
			strcpy(parser->pQueryString, s);
			return 1;
		}
		return 0;
	}

	if(!strcmp(method, POST)){
		strcpy(parser->method, POST);
		s = getenv(CONTENT_LENGTH);
		if(s == NULL)return 0;
		
		len = atoi(s);
		if(len <= 0)return 0;

		parser->pQueryString = (char*)malloc(len+1);
		if(parser->pQueryString == NULL)return 0;
		totalRead = 0;
		while(totalRead<len){
			read = fread((parser->pQueryString)+totalRead, 1, len-totalRead, stdin);
			if (read == 0 && ferror(stdin)) {
				break;
			}
			totalRead += read;
		}
		parser->pQueryString[totalRead] = '\0';
		parser->content_length = totalRead;
		return 1;
	}

	return 0;
}
#else
/*************************************************
 GetQueryString:
  Get Query String of "GET" or "POST"
  the data is fetched from request_rec
 NOTE: this function used privately in this module
 *************************************************/
static int GetQueryString(request_rec* r, CGIParser* parser){
	char *method;
	char *s;
	int len;
	
	if (r == NULL)return 0;
	/* generate enviroments used by CGI-function module */
	ap_add_cgi_vars(r);
	ap_add_common_vars(r);

	parser->content_type = (char*)ap_table_get(r->subprocess_env, CONTENT_TYPE);
	
	if(r->method_number == M_GET){
		strcpy(parser->method, GET);
		s = (char*)ap_table_get(r->subprocess_env, QUERY_STRING);
		if(!s)return 0;
		parser->pQueryString = (char*)malloc(strlen(s)+1);
		if(parser->pQueryString != NULL) {
			strcpy(parser->pQueryString, s);
			return 1;
		}
		return 0;
	}

	if(r->method_number == M_POST){
		strcpy(parser->method, POST);
		s = (char*)ap_table_get(r->subprocess_env, CONTENT_LENGTH);
		if(s == NULL)return 0;
		
		len = atoi(s);
		r->remaining = (long)len;
		r->read_length = 0;
		if(len <= 0)return 0;

		parser->pQueryString = (char*)malloc(len+1);
		if(parser->pQueryString == NULL)return 0;
		ap_get_client_block(r, parser->pQueryString, len+1);
		parser->pQueryString[len] = '\0';
		parser->content_length = len;
		return 1;
	}

	return 0;
}
#endif

/*************************************************
 CharToNum:
  convert the number character to digital.
 NOTE: private in this module.s
 *************************************************/
#define NON_NUM  'n'
static char CharToNum(char ch){
	if(ch >= '0' && ch <= '9')return (char)(ch-'0');
	if(ch >= 'a' && ch <= 'f')return (char)(ch-'a'+10);
	if(ch >= 'A' && ch <= 'F')return (char)(ch-'A'+10);
	return NON_NUM;
}

/****************************************
 ParseString:
  parse the QUERY_STRING from a string
 Parameter:
  str: a string as QUERY_STRING
 Return:
  >0: the number of name/value pairs
  0:  no name/pair or any error occurrs
 ****************************************/
int ParseString(CGIParser* parser, char* str){
	char *szQuery;
	int iNumOfQueryItems = 0;
	char ch, ch1, ch2;
	char *pHead, *pTail;
	int bName;

	if(str == NULL || str[0] == 0)
		return 0;

	if(parser->pParsedQuery)
		ReleaseQueryBuffer(parser);

	/*first scan to count the number of (name=value) pairs.
	 *(name=value) pairs use '&' as the delimeter.*/
	pHead = szQuery = str;
	parser->pairNums = 1;
	while((ch = *pHead) != '\0'){
		if(ch == '&')parser->pairNums ++;/*encounter '&', increase the number*/
		pHead ++;
	}
	if(*(pHead - 1) == '&'){
		parser->pairNums --;
		if(parser->pairNums == 0)return 0;
	}

	/* allocate buffer to store name/value pairs */
	parser->pParsedQuery = (QUERY_ITEM*)calloc(parser->pairNums, sizeof(QUERY_ITEM));
	if(parser->pParsedQuery == NULL){
		parser->pairNums = 0;
		return 0;
	}
	pHead = pTail = str;

	/* parse name/value pairs */
	iNumOfQueryItems = 0;
	bName = 1;

	while((ch = *szQuery) != '\0'){/*for all the char in the query string*/
		switch(ch){
		case '+':/* '+' is used to stand for blank space. */
			*pTail ++ = ' ';
			szQuery ++;
			break;

		case '%':/* '%' indicates an escape char is followed. */
			if((ch1=*(szQuery+1)) && (ch2=*(szQuery+2))){
				ch1 = CharToNum(ch1);
				ch2 = CharToNum(ch2);
				if((ch1 != NON_NUM) && (ch2 != NON_NUM)){
					*pTail++ = (char)(((ch1 << 4) & 0xf0) | ch2);
					szQuery += 3;
					break;
				}
			}
			*pTail++ = ch;
			szQuery ++;
			break;

		case '=':/*'=' is the delimeter of name and value.*/
			if(pTail > pHead){
				*pTail++ = 0;
				parser->pParsedQuery[iNumOfQueryItems].szName = pHead;
			}

			szQuery ++;
			bName = 0;
			pHead = pTail;
			break;

		case '&':/*'&' is the delimeter of (name=value) pairs.*/
			if(pTail > pHead){
				*pTail ++ = 0;
				szQuery ++;
				
				if(!bName){
					parser->pParsedQuery[iNumOfQueryItems].szValue = pHead;
					parser->pParsedQuery[iNumOfQueryItems].len = pTail - pHead;
				}
				else{
					parser->pParsedQuery[iNumOfQueryItems].szName = pHead;
				}
			}else{
				*pTail ++ = 0;
				szQuery ++;
				if(!bName){
					parser->pParsedQuery[iNumOfQueryItems].szValue = pHead;
					parser->pParsedQuery[iNumOfQueryItems].len = 0;
				}
			}

			iNumOfQueryItems ++;
			bName = 1;
			pHead = pTail;
			break;

		default:/*other chars*/
			*pTail ++ = ch;
			szQuery ++;
		}
	}

	/* copy the remaining name or value */
	if(pTail > pHead){
		*pTail ++ = 0;
		if(!bName){
			parser->pParsedQuery[iNumOfQueryItems].szValue = pHead;
			parser->pParsedQuery[iNumOfQueryItems].len = pTail - pHead;
		}else{
			parser->pParsedQuery[iNumOfQueryItems].szName = pHead;
		}
		iNumOfQueryItems ++;
	}else if((iNumOfQueryItems < parser->pairNums) && ((parser->pParsedQuery[iNumOfQueryItems].szName && parser->pParsedQuery[iNumOfQueryItems].szName[0]) || (parser->pParsedQuery[iNumOfQueryItems].szValue && parser->pParsedQuery[iNumOfQueryItems].szValue[0]))){
		*pTail = 0;
		if(!bName){
			parser->pParsedQuery[iNumOfQueryItems].szValue = pHead;
			parser->pParsedQuery[iNumOfQueryItems].len = 0;
		}
		iNumOfQueryItems ++;
	}
	return iNumOfQueryItems;
}


/****************************************
 ParseMultiPart:
  parse the QUERY_STRING from a string
 Parameter:
  str: a string as QUERY_STRING
 Return:
  -1: not a muilt-part string
  >0: the number of name/value pairs
  0:  no name/pair or any error occurrs
 ****************************************/
static int ParseMultiPart(CGIParser* parser, char* str){
	/* char *ct = getenv(CONTENT_TYPE); */
	int lenBeforeBoundary = strlen(MULTIPART);
	int lenOfDisposition = strlen(DISPOSITION);
	char *boundary;
	int lenOfBoundary;
	int lenOfType;
	char ch;
	
	char *szQuery;
	int iNumOfQueryItems = 0;
	char *pHead, *pTail;
	int bName;
	int i;
	
	if(str == NULL || str[0] == 0)
		return 0;
	
	if(parser->pParsedQuery)
		return parser->pairNums;
		
	/* get multipart boundary, only POST has such string */
	if(strcmp(parser->method, POST))return NOT_MULTIPART;
	if(parser->content_type == NULL)return NOT_MULTIPART;
	if(strncasecmp(parser->content_type, MULTIPART, lenBeforeBoundary))return NOT_MULTIPART;
	
	/*
	boundary = ct + lenBeforeBoundary;
	*/
	boundary = (char*)malloc(strlen(parser->content_type+lenBeforeBoundary) + 3);
	if(!boundary){
		return 0;
	}
	sprintf(boundary, "--%s", parser->content_type+lenBeforeBoundary);
	lenOfBoundary = strlen(boundary);
	lenOfType = strlen(MULTIPART_TYPE);
	
	/*first scan to count the number of (name=value) pairs.
	 *(name=value) pairs use $boundary as the delimeter.*/
	pHead = str;
	parser->pairNums = 0;
	for(i=0; i<parser->content_length; ){
		if(strncmp(pHead, boundary, lenOfBoundary) == 0){
			pHead += lenOfBoundary;
			i += lenOfBoundary;
			if(strncmp(pHead, "--", 2) == 0)break;
			
			while(*pHead && (*pHead == '\r' || *pHead == '\n')){
				pHead ++;
				i ++;
			}
			parser->pairNums ++;
		}
		else{
			pHead ++;
			i ++;
		}
	}
	if(parser->pairNums == 0){
		free(boundary);
		return 0;
	}
	
	/* allocate buffer to store name/value pairs */
	parser->pParsedQuery = (QUERY_ITEM*)calloc(parser->pairNums, sizeof(QUERY_ITEM));
	if(parser->pParsedQuery == NULL){
		parser->pairNums = 0;
		free(boundary);
		return 0;
	}
	
	/* parse name/value pairs */
	iNumOfQueryItems = 0;
	bName = 1; /* get boundary */
	szQuery = str;

	for(i=0; i<parser->content_length;){/*for all the char in the query string*/
		ch = *szQuery;
		if(bName){
			if(ch == '\r' || ch == '\n'){
				szQuery ++;
				i ++;
				continue;
			}
			
			if(strncmp(szQuery, boundary, lenOfBoundary) == 0){
				szQuery += lenOfBoundary;
				i += lenOfBoundary;
				if(strncmp(szQuery, "--", 2) == 0){//the end
					break;
				}
				/* get name */
				/* skip blank line */
				while(*szQuery && (*szQuery == '\r' || *szQuery == '\n')){
					szQuery ++;
					i ++;
				}
				/* disposition expected */
				if(strncasecmp(szQuery, DISPOSITION, lenOfDisposition))
					break;
				szQuery += lenOfDisposition;
				i += lenOfDisposition;
				if(*szQuery != '\"')
					break;
				szQuery ++;
				i ++;
				pHead = szQuery;
				while(*szQuery && *szQuery!='\"' && (*szQuery!='\r' && *szQuery!='\n')){
					szQuery ++;
					i ++;
				}
				if(*szQuery != '\"')
					break;
				*szQuery ++ = 0;
				i ++;
				parser->pParsedQuery[iNumOfQueryItems].szName = pHead;
				
				/*for extra filename */
				if(*szQuery == ';'){
					szQuery ++;
					i ++;
					while(*szQuery && *szQuery == ' '){
						szQuery ++;
						i ++;
					}
					if(strncasecmp(szQuery, "filename=\"", 10) == 0){
						szQuery += 10;
						i += 10;
						pHead = szQuery;
						while(*szQuery && *szQuery!='\"' && (*szQuery!='\r' && *szQuery!='\n')){
							szQuery ++;
							i ++;
						}
						pTail = szQuery;
						if(*szQuery != '\"')
							break;
						*szQuery ++ = 0;
						i ++;
						for(pTail--; pTail >= pHead; pTail--){
							if(*pTail == '\\' || *pTail == '/')break;
						}
						pHead = pTail + 1;
						parser->pParsedQuery[iNumOfQueryItems].filename = pHead;
					}
				}
				
				/* skip to end of line */
				while(*szQuery && *szQuery != '\r' && *szQuery != '\n'){
					szQuery ++;
					i ++;
				}
				if(*szQuery == '\r'){
					szQuery ++;
					i ++;
				}
				if(*szQuery == '\n'){
					szQuery ++;
					i ++;
				}
				/* skip un-blank line */
				while(1){
					if(*szQuery == '\r' || *szQuery == '\n'){
						if(*szQuery == '\r'){
							szQuery ++;
							i ++;
						}
						if(*szQuery == '\n'){
							szQuery ++;
							i ++;
						}
						break;
					}
					if(strncasecmp(szQuery, MULTIPART_TYPE, lenOfType) == 0){
						szQuery += lenOfType;
						i += lenOfType;
						pHead = szQuery;
						while(*szQuery && *szQuery != '\r' && *szQuery != '\n'){
							szQuery ++;
							i ++;
						}
						*szQuery ++ = 0;
						i ++;
						parser->pParsedQuery[iNumOfQueryItems].type = pHead;
						
						if(*szQuery == '\r'){
							szQuery ++;
							i ++;
						}
						if(*szQuery == '\n'){
							szQuery ++;
							i ++;
						}
						continue;
					}
					
					while(*szQuery && *szQuery != '\r' && *szQuery != '\n'){
						szQuery ++;
						i ++;
					}
					if(*szQuery == '\r'){
						szQuery ++;
						i ++;
					}
					if(*szQuery == '\n'){
						szQuery ++;
						i ++;
					}
				}
				
				bName = 0;
				pHead = szQuery;
			}else{
				szQuery ++;
				i ++;
			}
		}else{ /* value */
			if(strncmp(szQuery, boundary, lenOfBoundary) == 0){
				pTail = szQuery - 1;
				if((pTail >= pHead) && *pTail == '\n')*pTail -- = 0;
				if((pTail >= pHead) && *pTail == '\r')*pTail -- = 0;
				pTail ++;
				*pTail = 0;
				parser->pParsedQuery[iNumOfQueryItems].szValue = pHead;
				parser->pParsedQuery[iNumOfQueryItems].len = pTail - pHead;
				iNumOfQueryItems ++;
				/*
				szQuery += (lenOfBoundary + 2);

				if(strncmp(szQuery, "--", 2) == 0){
					break;
				}
				while(*szQuery && *szQuery != '\r' && *szQuery != '\n')szQuery ++;
				if(*szQuery == '\r')szQuery ++;
				if(*szQuery == '\n')szQuery ++;
				*/
				bName = 1;
			}else{
				szQuery ++;
				i ++;
			}
		}
	}
	
	free(boundary);
	return iNumOfQueryItems;
}

/****************************************
 ParseQuery:
  parse the QUERY_STRING of CGI program
 Parameter:
  a pointer to CGIParser
 Return:
  >0: the number of name/value pairs
  0:  no name/pair or any error occurrs
 ****************************************/
#ifndef APACHE_MODULE
int ParseQuery(CGIParser* parser)
#else
int ParseQuery(CGIParser* parser, request_rec* r)
#endif
{
	int num;
	
	if (parser == NULL)
		return 0;
	
#ifndef APACHE_MODULE
	if(parser->pParsedQuery)
		return parser->pairNums;

	if(parser->pQueryString){
		free(parser->pQueryString);
		parser->pQueryString = NULL;
	}
	if(!GetQueryString(parser)){
		return 0;
	}
#else
	if (r == NULL)return 0;

	/* check if the request_rec is parsed */
	if (ap_table_get(r->subprocess_env, CGI_PARSED_FLAG) != NULL)
		return parser->pairNums;
	
	/* set parsing flag */
	ap_table_set(r->subprocess_env, CGI_PARSED_FLAG, "I have been parsed already");
	ReleaseQueryBuffer(parser);
	
	if(!GetQueryString(r, parser)){
		return 0;
	}
#endif

	if(parser->pQueryString == NULL){
		return 0;
	}
	
	num = ParseMultiPart(parser, parser->pQueryString);
	if (num != NOT_MULTIPART) {
		return num;
	}
	num = ParseString(parser, parser->pQueryString);
	if(num <= 0){
		free(parser->pQueryString);
		parser->pQueryString = NULL;
	}

	return num;
}

/***************************************************
 Param:
  get the value of a specified parameter
 Parameter:
  szName: the name of a parameter
 Return:
  the address of the value or NULL
 ***************************************************/
char* Param(const CGIParser* parser, const char *szName){
	int i;

	if(parser == NULL || parser->pairNums <= 0 || szName == NULL || szName[0] == 0)return NULL;

	for(i=0; i<parser->pairNums; i++){
		if(parser->pParsedQuery[i].szName && !strcmp(szName, parser->pParsedQuery[i].szName)){
			return parser->pParsedQuery[i].szValue;
		}
	}
	return NULL;
}

/***************************************************
 Params:
  get all the values of a specified parameter
 Parameters:
  szName: the name of the parameter
  result: an array to store the result
  size:   the size of result
 Return:
  the number of values copied into result
 NOTE: it is the user's responsibility to allocate
  and release the memory of result
 ***************************************************/
int Params(const CGIParser* parser, const char* szName, char* result[], int size){
	int i, count;

	if(parser == NULL || parser->pairNums <= 0 || szName == NULL || szName[0] == 0 || size <= 0)return 0;
	for(i=0, count=0; i<parser->pairNums && count<size; i++){
		/* if(parser->pParsedQuery[i].szName && parser->pParsedQuery[i].szValue && !strcmp(szName, parser->pParsedQuery[i].szName)){ */
		if(parser->pParsedQuery[i].szName && !strcmp(szName, parser->pParsedQuery[i].szName)){
			result[count++] = parser->pParsedQuery[i].szValue;
		}
	}
	return count;
}

/***************************************************
 GetFile:
  get an uploading file
 Parameters:
  szName(IN):    the name of the parameter
  filename(OUT): the name of uploading file, NULL if no file at all.
  result(OUT):   the starting pointer of the uploading file content, NULL if no file at all.
  type(OUT):     the MIME type of the uploading file. NULL if no file at all.
  size(OUT):     the size of the uploading file
 Return:
  if there is uploading file, 1, otherwise 0
 ***************************************************/
int GetFile(const CGIParser* parser, const char* szName, char** filename, char** result, char** type, int* size){
	int i;
	
	*filename = *result = *type = NULL;
	*size = 0;

	if(parser == NULL || parser->pairNums <= 0 || szName == NULL || szName[0] == 0)return 0;
	for(i=0; i<parser->pairNums; i++){
		if(parser->pParsedQuery[i].szName && !strcmp(szName, parser->pParsedQuery[i].szName)){
			*filename = parser->pParsedQuery[i].filename;
			*result   = parser->pParsedQuery[i].szValue;
			*type     = parser->pParsedQuery[i].type;
			*size     = parser->pParsedQuery[i].len;
			
			return 1;
		}
	}
	
	return 0;
}

/***************************************************
 HasParam:
  to test whether a specified parameter is included
  in the QUERY_STRING
 Parameter:
  szName: the name of the parameter to look for
 Return:
  1: the parameter exists
  0: no such parameter
 ***************************************************/
int HasParam(const CGIParser* parser, const char *szName){
	int i;

	if(parser == NULL || parser->pairNums <= 0 || szName == NULL || szName[0] == 0)return 0;

	for(i=0; i<parser->pairNums; i++){
		if(parser->pParsedQuery[i].szName && !strcmp(szName, parser->pParsedQuery[i].szName))
			return 1;
	}

	return 0;
}

/***************************************************
 DumpParams:
  Dump the parameters with the format "name = value"
  in a file stream
 Parameters:
  out: the output stream
  deli: delimeter to split name/value pairs
 Return:
  none
 ***************************************************/
void DumpParams(const CGIParser* parser, FILE* out, const char* deli){
	int i;
	char *name,*value,*filename,*type;
	int len;

	if(parser == NULL || out == NULL)
		return;

	fprintf(out, "CGI VARIABLES DUMP: START%s", (deli?deli:""));
	for(i=0; i<parser->pairNums; i++){
		name = parser->pParsedQuery[i].szName;
		value = parser->pParsedQuery[i].szValue;
		filename = parser->pParsedQuery[i].filename;
		type = parser->pParsedQuery[i].type;
		len = parser->pParsedQuery[i].len;
		if(filename){
			fprintf(out, " %s: %s <= %s, %s, %d%s", (name?name:"[no name]"), (value?value:"[no value]"), filename, (type?type:"[no type]"), len, deli==NULL ? "" : deli);
		}else{
			fprintf(out, " %s: %s%s", (name?name:"[no name]"), (value?value:"[no value]"), deli==NULL ? "" : deli);
		}
	}
	fprintf(out, "CGI VARIABLES DUMP: END%s", deli?deli:"");
}

/***************************************************
 ReleaseQueryBuffer:
  release the buffer to store the name/value pairs
 Parameter & Return
  none
 ***************************************************/
void ReleaseQueryBuffer(CGIParser* parser){
	if (parser != NULL) {
		if(parser->pParsedQuery){
			free(parser->pParsedQuery);
			parser->pParsedQuery = NULL;
		}
		if(parser->pQueryString){
			free(parser->pQueryString);
			parser->pQueryString = NULL;
		}
		parser->pairNums = 0;
	}
}

char* GetMethod(const CGIParser* parser){
	if (parser != NULL) {
		return (char*)(parser->method);
	} else {
		return GET;
	}
}
