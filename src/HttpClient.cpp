/*************************************************************************
	> File Name: HttpClient.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Wed 22 Jan 2014 02:49:30 PM CST
 ************************************************************************/

#include<iostream>
#include "HttpClient.h"
using namespace std;

#define MAX_QUERYSTRING 2048
#define MAX_KLEN 256
#define MAX_VLEN 512
/***
 * 发送http POST请求
 * 
 * */
bool httpclent_perform(const char *url, const char *params, PARSE_FUNCTION parser){
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) {
		char buf[1024];
		curl_easy_setopt(curl, CURLOPT_URL, url);
	    /* Now specify the POST data */ 
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);

		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parser);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)buf);
	 
	    /* Perform the request, res will get the return code */ 
	    res = curl_easy_perform(curl);
	    /* Check for errors */ 
	    if(res != CURLE_OK)
		      fprintf(stderr, "curl_easy_perform() failed: %s\n",
	          curl_easy_strerror(res));
				 
	    /* always cleanup */ 
	    curl_easy_cleanup(curl);
   }
   return true;
}

/***
 * 解析query string kv对
 * */
bool parse_pair(const char* pair_str, char *key, char *value){
	const char *pos = strchr(pair_str, '=');
	if(pos == NULL)
		return false;
	strncpy(key, pair_str, pos-pair_str);
	key[pos-pair_str] = '\0';
	int len = strlen(pos);
	url_decode(pos + 1, len, value, MAX_VLEN);
	//strncpy(value, pos + 1, len);
	return true;
}

/***
 * 解析query string
 * */
bool parse_params(const char *query_str, map<string,string, cmpKeyAscii>* formData){
	char params[MAX_QUERYSTRING] = {0};
	char *pr = params;
	int len = strlen(query_str);
	if(query_str == NULL){
		return false;
	}
	if(len > MAX_QUERYSTRING){
		return false;
	}
	char *str;
	char key[MAX_KLEN];
	char value[MAX_VLEN];
	memcpy(params, query_str, len);
	while(true){
		str = strchr(pr, '&');
		if(str == NULL){
			if(parse_pair(pr,key, value)){
				formData->insert(std::pair<string, string>(key, value));
				printf("key:%s\tvalue:%s\n", key, value);
			}
			break;
		}
		*str = '\0';
		if(parse_pair(pr, key, value)){
			formData->insert(std::pair<string, string>(key, value));
			printf("key:%s\tvalue:%s\n", key, value);
		}
		int diff = strlen(pr);
		pr += diff + 1;
	}
	return true;
}

bool parse_query(const char *query_str, TopupInfo *topup_info){
	char params[MAX_QUERYSTRING] = {0};
	char *pr = params;
	int len = strlen(query_str);
	if(query_str == NULL){
		return false;
	}
	if(len > MAX_QUERYSTRING){
		return false;
	}
	char *str;
	char key[MAX_KLEN];
	char value[MAX_VLEN];
	memcpy(params, query_str, len);
	while(true){
		str = strchr(pr, '&');
		if(str == NULL){
			if(parse_pair(pr,key, value)){
				if(strcmp(key, "coopId") == 0)          //商家编号
				{
					topup_info->coopId = value;
				}
				else if(strcmp(key, "tbOrderNo") == 0)       //淘宝的订单号
				{
					topup_info->tbOrderNo = value;
				}
				else if(strcmp(key ,"cardId") == 0)          //充值卡商品编号
				{
					topup_info->cardId = value;
				}
				else if(strcmp(key, "cardNum") == 0)            //充值卡数量
				{
					topup_info->cardNum = atoi(value);
				}
				else if(strcmp(key, "customer") == 0)        //手机号码
				{
					topup_info->customer = value;
				}
				else if(strcmp(key , "sum") == 0)             //本次充值总金额
				{
					topup_info->sum = atof(value);
				}
				else if(strcmp(key, "tbOrderSnap") == 0)     //商品信息快照
				{
					topup_info->tbOrderSnap = value;
				}
				else if(strcmp(key, "notifyUrl") == 0)       //异同通知地址
				{
					topup_info->notifyUrl = value;
				}
				else if(strcmp(key, "sign") == 0)            //签名字符串
				{
					topup_info->sign = value;
				}
				else if(strcmp(key , "version") == 0)         //版本
				{
					topup_info->version = value;
				}
				//printf("key:%s\tvalue:%s\n", key, value);
			}
			break;
		}
		*str = '\0';
		if(parse_pair(pr, key, value)){
			if(strcmp(key, "coopId") == 0)          //商家编号
			{
				topup_info->coopId = value;
			}
			else if(strcmp(key, "tbOrderNo") == 0)       //淘宝的订单号
			{
				topup_info->tbOrderNo = value;
			}
			else if(strcmp(key ,"cardId") == 0)          //充值卡商品编号
			{
				topup_info->cardId = value;
			}
			else if(strcmp(key, "cardNum") == 0)            //充值卡数量
			{
				topup_info->cardNum = atoi(value);
			}
			else if(strcmp(key, "customer") == 0)        //手机号码
			{
				topup_info->customer = value;
			}
			else if(strcmp(key , "sum") == 0)             //本次充值总金额
			{
				topup_info->sum = atof(value);
			}
			else if(strcmp(key, "tbOrderSnap") == 0)     //商品信息快照
			{
				topup_info->tbOrderSnap = value;
			}
			else if(strcmp(key, "notifyUrl") == 0)       //异同通知地址
			{
				topup_info->notifyUrl = value;
			}
			else if(strcmp(key, "sign") == 0)            //签名字符串
			{
				topup_info->sign = value;
			}
			else if(strcmp(key , "version") == 0)         //版本
			{
				topup_info->version = value;
			}
			//printf("key:%s\tvalue:%s\n", key, value);
		}
		int diff = strlen(pr);
		pr += diff + 1;
	}
	return true;	
}

/***
 * 对字符串做MD5加密
 * */
int str2md5(const char* src, int len,char *md5str){
	if(src == NULL)
		return 1;
	if(len <= 0)
		return 2;
	if(md5str == NULL)
		return 3;
	unsigned char md[16];
	unsigned char* tmp = (unsigned char*)malloc(len * sizeof(unsigned char));
	if(tmp == NULL)
		return 4;
	memcpy(tmp, src, len);
	int i;

	MD5(tmp ,len ,md);
    for (i = 0; i < 16; i++){
	    sprintf(md5str + i*2,"%2.2x",md[i]);
    }
	free(tmp);
    return 0;
}

int change_code(const char* from, const char* to, char *inbuf, size_t *insize, char *outbuf, size_t *outsize)
{
	int ret;
	iconv_t convert = iconv_open(to, from);
	if(iconv_t(-1) == convert){
		fprintf(stderr, "iconv_open error!\n");
		return -1;
	}
	ret = iconv(convert, &inbuf, insize, &outbuf, outsize);
	iconv_close(convert);
	return ret;
}
#define NON_NUM '0'

int hex2num(char c)
{
    if (c>='0' && c<='9') return c - '0';
    if (c>='a' && c<='z') return c - 'a' + 10;//这里+10的原因是:比如16进制的a值为10
    if (c>='A' && c<='Z') return c - 'A' + 10;
				    
    printf("unexpected char: %c", c);
    return NON_NUM;
}

int url_encode(const char* str, const int strSize, char* result, const int resultSize)
{
	int i;
	int j = 0;//for result index
	char ch;
	if ((str==NULL) || (result==NULL) || (strSize<=0) || (resultSize<=0)) {
		return 0;
	}
	for ( i=0; (i<strSize)&&(j<resultSize); ++i) {
		ch = str[i];
		if (((ch>='A') && (ch<='Z')) ||
			((ch>='a') && (ch<='z')) ||
			((ch>='0') && (ch<='9'))) {
			result[j++] = ch;
		} else if (ch == ' ') {
			result[j++] = '+';
		} else if (ch == '.' || ch == '-' || ch == '_' || ch == '*') {
			result[j++] = ch;
		} else {
			if (j+3 < resultSize) {
				sprintf(result+j, "%%%02X", (unsigned char)ch);
				j += 3;
			} else {
				return 0;
			}
		}
	}

	result[j] = '\0';
	return j;
}

int url_decode(const char* str, const int strSize, char* result, const int resultSize)
{
	char ch,ch1,ch2;
	int i;
	int j = 0;//record result index
	if ((str==NULL) || (result==NULL) || (strSize<=0) || (resultSize<=0)) {
		return 0;
	}
	for ( i=0; (i<strSize) && (j<resultSize); ++i) {
		ch = str[i];
		switch (ch) {
			case '+':
				result[j++] = ' ';
				break;
			case '%':
				if (i+2<strSize) {
					ch1 = hex2num(str[i+1]);//高4位
					ch2 = hex2num(str[i+2]);//低4位
					if ((ch1!=NON_NUM) && (ch2!=NON_NUM))
						result[j++] = (char)((ch1<<4) | ch2);
						i += 2;
						break;
					} else {
						break;
					}
			default:
				result[j++] = ch;
				break;
		}
	}
    result[j] = 0;
    return j;
}