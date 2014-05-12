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

using namespace oracle::occi;
using namespace std;

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

#define CHARGEQUEUE "underway"
#define QUERYQUEUE "query"
#define NOTIFYQUEUE "notify"


//通知状态定义
#define NOTIFY_UNDONE 0			//未通知
#define NOTIFY_SUCCESS 1		//已经成功通知
#define NOTIFY_FAIL 2			//通知失败

enum RequestType{
    CHARGE = 0,
    QUERY,
    BALANCE,
    CANCEL,
    NOTIFY
};

enum OrderStatus{CREATE = 0, UNDERWAY, SUCCESS, FAILED, CANCELED};

typedef struct ChannelInfo{
	int channelId;					//渠道ID
	std::string channelName;		//渠道名称
	std::string sname;				//渠道简称
	float discount;					//折扣
	std::string interfaceName;		//接口标识
	int priority;					//优先级
	int repeat;						//重试次数
	std::string pid;				//代理商用产品id
} ChannelInfo;

typedef struct QsInfo{
	std::string coopId;				//商家编号
    std::string tbOrderNo;			//淘宝的订单号
	std::string coopOrderNo;		//系统生成订单号
    std::string cardId;				//充值卡商品编号
    int cardNum;					//充值卡数量
    std::string customer;			//手机号码
    double sum;						//本次充值总金额
    std::string tbOrderSnap;		//商品信息快照
    std::string notifyUrl;			//异同通知地址
    std::string sign;				//签名字符串
    std::string version;			//版本
	std::string coopOrderStatus;	//notify状态
	double price;					//总价，不含打折信息
	int value;						//面值
	int op;							//运营商
	int province;					//省份
}QsInfo;

typedef struct TopupInfo{
	QsInfo qs_info;							//请求参数相关
	std::vector<ChannelInfo> channels;		//最优渠道
	Connection *conn;						//数据库连接
	char log[MAX_LOG_LEN];					//业务日志缓冲区
	char err_log[MAX_LOG_LEN];				//错误日志缓冲区
	int log_len;							//缓冲区位置标记
	int err_log_len;						//缓冲区位置标记
	uint32_t seqid;							//请求序列标记
	struct timeval start_time;				//开始处理时间
	OrderStatus status;
	std::string update_time;
	int notify;								//是否通知
	int channelId;							//使用的渠道id
	double channel_discount;				//使用渠道折扣
	std::string interfaceName;				//当前使用接口标识
	int repeat;
	std::string channelSname;				//当前使用代理商简称
	uint32_t create_time;					//订单创建时间
	std::string pid;						//当前使用代理商产品id
	uint32_t last_op_time;					//记录最后一次操作改记录时间
	std::string userid;						//标记请求用户的身份信息，天猫用tmall
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
int so_customer_reload(SoBase *h);


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

/**最优渠道选择函数**/
inline bool ChannelRank(ChannelInfo channelA, ChannelInfo channelB){                                                                                                                                           
	        return channelA.priority > channelB.priority;
}



//根据手机号码和时间生成order no
extern uint64_t encode_orderno(std::string phoneNo);

//解析order no从中得到手机号和时间戳
extern void decode_orderno(const uint64_t orderno, uint64_t *phoneno, uint32_t *ttime);

extern int get_time_now(const char* format, std::string &time_str);

extern int calc_time_span(struct timeval *start_time);

extern void write_err_msg(TopupInfo *topupInfo, std::vector<std::string>& errors);

extern int get_strtime(const uint32_t ts, const char* format, std::string &time_str);

extern void trans_time(std::string &from, std::string &to);

extern void serialize_topupinfo(TopupInfo* topup_info, std::string &strout);

extern void deserialize_topupinfo(const std::string& json, TopupInfo* topup_info);

#endif  //__TOPUP_UTILS_H
