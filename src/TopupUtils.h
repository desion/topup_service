/*************************************************************************
	> File Name: TopupUtils.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 02:38:15 PM CST
 ************************************************************************/

#ifndef __TOPUP_UTILS_H
#define __TOPUP_UTILS_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "dllcall.h"
#include "TopupBase.h"
#include "slog.h"
#include "selog.h"
#include <openssl/md5.h>
#include "jsoncpp/json.h"

#define MAX_LOG_LEN  1024

//将日志写入缓冲区
#define TP_WRITE_LOG(info, fmt, ...) { \
	if(info->log_len < MAX_LOG_LEN){\
	    info->log_len += snprintf(info->log + info->log_len, MAX_LOG_LEN - info->log_len, fmt, ##__VA_ARGS__);\
	}\
}

#define TP_WRITE_ERR(info, fmt, ...) { \
	if(info->err_log_len < MAX_LOG_LEN){\
	    info->err_log_len += snprintf(info->err_log + info->err_log_len, MAX_LOG_LEN - info->err_log_len, fmt, ##__VA_ARGS__);\
	}\
}

//调用动态链接库方法
#define DLLCALL(so, call, ret, ...) {\
	    pthread_rwlock_rdlock(&so->lock);\
	    ret = so->call(__VA_ARGS__);\
	    pthread_rwlock_unlock(&so->lock);\
}

#define DLLCALL2(so, call, ...) {\
	    pthread_rwlock_rdlock(&so->lock);\
	    so->call(__VA_ARGS__);\
	    pthread_rwlock_unlock(&so->lock);\
}

#define CHECKNULL(var, fmt, ...) {\
	if(var == NULL){\
		slog_write(LL_FATAL, fmt, ##__VA_ARGS__);\
		return false;\
	}\
}

enum OrderStatus{CREATE = 0, UNDERWAY, SUCCESS, FAILED, CANCELED};

typedef struct ChannelInfo{
	int channelId;			//渠道ID
	string channelName;		//渠道名称
	string sname;			//渠道简称
	float discount;			//折扣
	string interfaceName;	//接口标识
	int priority;			//优先级
	int repeat;				//重试次数
} ChannelInfo;

typedef struct QsInfo{
	string coopId;          //商家编号
    string tbOrderNo;       //淘宝的订单号
	string coopOrderNo;		//系统生成订单号
    string cardId;          //充值卡商品编号
    int cardNum;            //充值卡数量
    string customer;        //手机号码
    double sum;             //本次充值总金额
    string tbOrderSnap;     //商品信息快照
    string notifyUrl;       //异同通知地址
    string sign;            //签名字符串
    string version;         //版本
	string coopOrderStatus;	//notify状态
	double price;			//单价
	int value;				//面值
	int op;					//运营商
	int province;			//省份
}QsInfo;

typedef struct TopupInfo{
	/*
	string coopId;          //商家编号
    string tbOrderNo;       //淘宝的订单号
	uint64_t coopOrderNo;	//系统生成订单号
    string cardId;          //充值卡商品编号
    int cardNum;            //充值卡数量
    string customer;        //手机号码
    double sum;             //本次充值总金额
    string tbOrderSnap;     //商品信息快照
    string notifyUrl;       //异同通知地址
    string sign;            //签名字符串
    string version;         //版本
	double price;			//单价
	int value;				//面值
	int op;					//运营商
	int province;			//省份
	*/
	QsInfo qs_info;						//请求参数相关
	vector<ChannelInfo> channels;		//最优渠道
	Connection *conn;					//数据库连接
	char log[MAX_LOG_LEN];				//业务日志缓冲区
	char err_log[MAX_LOG_LEN];			//错误日志缓冲区
	int log_len;						//缓冲区位置标记
	int err_log_len;					//缓冲区位置标记
	uint32_t seqid;						//请求序列标记
	struct timeval start_time;			//开始处理时间
	OrderStatus status;
	string update_time;
	int notify;
}TopupInfo;


//动态库重加载指令
#define SO_RELOAD_CMD		100
//配置等重加载指令
#define CONF_RELOAD_CMD		101
//服务进入维护状态指令
#define SERVICE_STOP_CMD	102
//服务恢复指令
#define SERVICE_RESUME_CMD	103
//dump缓存数据指令
#define DUMP_CACHE_CMD		104


//XML response状态
#define SSUCCESS		"<coopOrderStatus>SUCCESS</coopOrderStatus>"
#define SUNDERWAY		"<coopOrderStatus>UNDERWAY</coopOrderStatus>"
#define SORDER_FAILED 	"<coopOrderStatus>ORDER_FAILED</coopOrderStatus>"
#define SFAILED			"<coopOrderStatus>FAILED</coopOrderStatus>"
#define SREQUEST_FAILED	"<coopOrderStatus>REQUEST_FAILED</coopOrderStatus>"
#define SCANCEL			"<coopOrderStatus>CANCEL<coopOrderStatus>"


typedef struct err_info{
	int code;
	char* msg;
} err_info;

typedef struct SoBase{
	void *handle;
	pthread_rwlock_t lock;
	char *filename;
	int file_time;
	int (*reload)(struct SoBase *so);
	TopupBase* (*create)();
	void (*destory)(TopupBase* topupBase);
} SoBase;


//初始化动态链接库加载，reload函数为动态链接库加载函数
int so_init(SoBase *so, char *filename, int (reload)(SoBase *so));
int so_change(SoBase *so, char *filename);
int so_check_reload(SoBase *so);
int so_load(SoBase *so);
int so_free(SoBase *so);
//动态链接库加载函数，主要是加载相应的函数
int so_topup_reload(SoBase *h);


inline int split_string (char * s, const char * seperator, std::vector<char *> & field_vec)
{
	field_vec.clear();
	if (s == NULL) return -1;
	if (seperator == NULL) return -1;

	int sep_len = strlen(seperator);
	char * p = s;
	field_vec.push_back(p);
	while ((p = strstr(p, seperator)) != NULL)
	{
		*p = '\0';
		p += sep_len;
		field_vec.push_back(p);
	}
	return field_vec.size();
}

//根据手机号码和时间生成order no
extern uint64_t encode_orderno(string phoneNo);

//解析order no从中得到手机号和时间戳
extern void decode_orderno(const uint64_t orderno, uint64_t *phoneno, uint32_t *ttime);

extern int get_time_now(const char* format, string &time_str);

extern int calc_time_span(struct timeval *start_time);

extern void write_err_msg(TopupInfo *topupInfo, vector<string>& errors);

extern int get_strtime(const uint32_t ts, const char* format, string &time_str);

extern void trans_time(string &from, string &to);

extern void serialize_topupinfo(TopupInfo* topup_info, string &strout);

#endif  //__TOPUP_UTILS_H
