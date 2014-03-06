#ifndef SLOG_H
#define SLOG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_LINE_LEN
#define MAX_LINE_LEN            256         /* 最大行字符串长度 */
#endif

extern const char ** slog_thr_tag(void);

/**
 * 线程标签：slog_open以后才可对其赋值和访问
 * 在每个线程开始处将其赋值，所赋值需要在线程生命周期都能访问，除非在同一线程中另赋新值；
 * 不赋值则采用线程号来标示此线程。
 * 例如slogThrTag = "WORK_THR"，则最后的log格式如下所示：
 * NOTICE 2005-12-30 10:20:30 WORK_THR# 用户自定义消息
 */
#define slogThrTag	(*slog_thr_tag())

/* 日志级别 */
typedef enum
{
	LL_ALL,		/* 所有事件 */
	LL_DEBUG,	/* debug信息 */
	LL_TRACE,	/* debug用事件跟踪 */
	LL_NOTICE,	/* 一般事件 */
	LL_WARNING, /* 警告，可恢复性错误 */
	LL_FATAL,	/* 致命错误 */
	LL_NONE		/* 不记任何日志 */
}TLogLevel;

/* 日志特殊设置 */
typedef struct
{
	unsigned log2TTY : 1;	/* 将日志条目写入文件的同时，是否输出到stderr */
	unsigned other : 31;	/* 保留 */
}TLogSpec;

/* 日志设置 */
typedef struct
{ 
	TLogLevel minLevel;	/* 允许记录的最小日志级别 */
	int maxLen;				/* 最大日志文件长度（超出被自动截断） */
	TLogSpec spec;			/* 特殊设置 */
}TLogConf; 

/*
 * 打开进程日志，slog_open函数的需要配置文件的版本
 * 需要配置文件HOME/conf/common.conf，失败则退出进程
 *
 * 参数：
 *		homeDir - 抓站系统HOME目录的全路径（以/结尾）
 *		procName - 进程名，如ec
 */
extern void open_log(char *homeDir,char *procName);


/*
 * 打开进程日志，应该在进程开始处(main函数开始处)进行一次初始化
 * 返回值：
 *		成功返回0；失败返回-1
 * 参数：
 *		logDir - 日志文件所在的目录
 *		processName - 进程名（用来自动生成日志文件名）
 *		logConf - 日志设置，为NULL则使用缺省设置
 */
extern int slog_open(const char *logDir,const char *processName, TLogConf *logConf);

/*
 * 写日志（适用于进程和线程）
 * 返回值：
 * 		成功返回0；失败返回-1
 * 参数：
 * 		level - 日志级别
 *		format, ... - 类似于printf的变长参数串 
 */
extern int slog_write(int level, const char *format, ... );

/*
 * 关闭进程日志，调用者须保证不使用其他slog函数时才能将其关闭
 * 返回值：
 * 		成功返回0；失败返回-1
 * 参数：
 * 		isErr - 是否非正常关闭 
 */ 
extern int slog_close(int isErr); 

#ifdef __cplusplus
}
#endif

#endif

