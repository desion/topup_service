#ifndef URLFORMAT_H
#define URLFORMAT_H

/*
 *判断是否为空格、换行、制表符等空位符
 * 参数：
 * 	ch: 操作字符对象
 * 返回：
 * 	0: 否
 * 	非0: 是
 * */
int is_space(char ch);

/*  	  
 *消除字符串尾部换行符和空白符
 *参数  	  
 * lpStr：源字符串，目的字符串
 *无返回	
 */
void trim_right(char *lpStr);

/*
 *字符串转小写
 * 参数：
 * 	str:	转小写用字符串
 * 返回：
 * 	无
 */
void str2lower(char *str);

/**
 *取得域名
 *参数
 * domain：返回的域名
 * url：源URL
 *返回：
 * 1：参数错 2 非法url
 * 0：成功
 */
int get_full_host_name(const char *url, char *domain);

/* 取得路径
 *参数
 * url：源URL
 * path：返回的路径
 *返回：
 * 1：参数错 2 非法url
 * 0：成功
 */
int get_full_path_name(const char *url, char *path);

// 格式化http协议的url，成功返回0，失败返回1
int format_http_url(const char *rawUrl, char *destUrl);

// 格式化http协议的相对url，成功返回0，失败返回1
// baseUrl应该为已经格式化过的绝对url
int format_http_relative_url(const char *baseUrl, const char *relativeUrl, char *destUrl);

// 索引url归一化
// url必须为格式化过的不带参数的url
// multiLevel表示是否对目录也进行归一化处理
void uniform_index_url(char *url, int multiLevel);

/*
 * 判断url是否是动态url，返回0表示静态，1表示动态
 */
int is_dynamic_url(const char *url);


#endif

