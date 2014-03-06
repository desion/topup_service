/*
 * Copyright (c) 2005, 新浪网研发中心
 * All rights reserved.
 *
 * 文件名称：selog.h
 * 摘    要：统一日志处理
 *         . 日志有正常/异常之分
 *         . 日志初始化时指明路径和日志标识
 *         . 日志包括通用的pid(可选)、时间信息，其它为为自定义信息
 *         . 日志过大时支持文件轮换，轮换方式可以按时间或大小进行，在初始化时指明
 *         . 多线程安全
 *         . 为减少写日志影响工作速度，日志可以加缓冲，但对错误日志不缓冲。
 * 作    者：徐绿兵, 2005.1.11
 * 版    本：1.0
 * $Id: selog.h,v 1.6 2005/04/08 02:38:26 lubing Exp $
 */
#ifndef _SE_LOG_H
#define _SE_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/* 日志句柄数据类型 */
typedef void* LOG_HANDLE;

/* 日志输出选项定义，在调用seLogInit时传递参数，多个值可以通过"或"一起传递 */
enum
{
	SLO_OUTPUT_PID      = 0x00000001,  /* 输出pid标志 */
	SLO_SWITCH_BY_HOUR  = 0x00000002,  /* 日志以小时进行轮换输出 */
	SLO_SWITCH_BY_DAY   = 0x00000004,  /* 日志以日进行轮换输出 */
	SLO_SWITCH_BY_SIZE  = 0x00000008,  /* 日志以大小进行轮换输出，同时需指明轮换大小 */
	SLO_BUFFERED_OUTPUT = 0x00000010   /* 正常日志进行缓存后输出，同时需指明缓存大小 */
};

/**
 * 日志初始化
 * 参数:
 *   phLogHandle: 用于保存初始化后的日志句柄，可以在调用后续函数时使用
 *   pszPath:     日志存放路径
 *   pszPrefix:   日志文件名前缀
 *   options:     日志选项，见定义，各选项可以"或"。
 *                如果选项SLO_SWITCH_BY_DAY出现，SLO_SWITCH_BY_HOUR将被忽略
 *   pszTimeFmt   日期时间的输出格式，为函数strftime的format字符串，为NULL时使用"%Y-%m-%d %H:%M:%S"
 *   iSwitchSize: 以轮换输出的大小(单位M)，该值在选项SLO_SWITCH_BY_SIZE指明时必须提供且必须大于10M；无此选项时忽略且使用缺省轮换值
 *   iBufferSize: 正常日志缓存大小(单位K)，该值在选项SLO_BUFFERED_OUTPUT指明时必须提供且必须大于10K；无此选项时忽略
 * 返回:
 *   0: 初始化成功，phLogHandle保存日志句柄
 *   <0:初始化失败，各种错误码
 * [说明] 在不指明大小轮换的情况下，缺省大小为1G；轮换大小如果>=2048M(2G)，也会使用缺省轮换大小
 */
int seLogInit(LOG_HANDLE* phLogHandle, const char *pszPath, const char *pszPrefix, int options, const char* pszTimeFmt, int iSwitchSize, int iBufferSize);

/**
 * 输出正常工作日志，带"Ex"为线程安全的函数
 * 参数:
 *   hLogHandle: 正常调用seLogInit的日志句柄
 *   pszFormat:  输出日志的格式化串，参考printf
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seLog(LOG_HANDLE hLogHandle, const char *pszFormat, ...);
int seLogEx(LOG_HANDLE hLogHandle, const char *pszFormat, ...);

/**
 * 输出缓存中的工作日志[线程安全的函数]
 * 参数:
 *   hLogHandle: 正常调用seLogInit的日志句柄
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seFlushLog(LOG_HANDLE hLogHandle);
int seFlushErrLog(LOG_HANDLE hLogHandle); /* 调试的时候用，否则会降低输出速度 */

/**
 * 输出错误日志，带"Ex"为线程安全的函数
 * seErrLog/seErrLogEx不输出文件名和行号
 * seErrLog2/seErrLog2Ex输出文件名和行号
 * 参数:
 *   hLogHandle: 正常调用seLogInit的日志句柄
 *   pszFormat:  输出日志的格式化串，参考printf
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
#define seErrLog(   hLogHandle, pszFormat, ...)  seErrLogImpl  (NULL, 0, hLogHandle, pszFormat, ##__VA_ARGS__)
#define seErrLogEx( hLogHandle, pszFormat, ...)  seErrLogExImpl(NULL, 0, hLogHandle, pszFormat, ##__VA_ARGS__)
#define seErrLog2(  hLogHandle, pszFormat, ...)  seErrLogImpl  (__FILE__, __LINE__, hLogHandle, pszFormat, ##__VA_ARGS__)
#define seErrLog2Ex(hLogHandle, pszFormat, ...)  seErrLogExImpl(__FILE__, __LINE__, hLogHandle, pszFormat, ##__VA_ARGS__)

/**
 * 输出错误日志函数实现。请使用上面定义的宏进行调用
 */
int seErrLogImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...);
int seErrLogExImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...);

/**
 * 关闭日志，释放各种资源
 * 参数:
 *   phLogHandle: 正常调用seLogInit的日志句柄
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seCloseLog(LOG_HANDLE* phLogHandle);

#ifdef __cplusplus
}
#endif

#endif
