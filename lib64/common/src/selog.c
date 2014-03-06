/*
 * Copyright (c) 2005, 新浪网研发中心
 * All rights reserved.
 *
 * 文件名称：selog.c
 * 摘    要：统一日志处理
 *         . 日志有正常/异常之分
 *         . 日志初始化时指明路径和日志标识
 *         . 日志包括通用的pid(可选)、时间信息，其它为为自定义信息
 *         . 日志过大时支持文件轮换，轮换方式可以按时间或大小进行，在初始化时指明
 *         . 多线程安全
 *         . 为减少写日志影响工作速度，日志可以加缓冲，但对错误日志不缓冲。
 * 作    者：徐绿兵, 2005.1.11
 * 修    改: 2005.2.28 输出错误日志时可以选择是否输出出错的文件名和行号
 *           2005.4.1  清空日志缓冲去掉了必须有自定义缓冲才能执行的限制
 *           2005.4.8  在任何情况下都使用大小轮换机制，增加了缺省轮换
 *           2005.5.10 缓冲机制改为使用glic的setvbuf函数实现，简化了代码
 *           2005.5.19 进程重启时调用日志初始化程序不清除原有的日志
 *					 2007.4.16 修改日志文件命名，添加年信息。
 * 版    本：1.0
 * $Id: selog.c,v 1.9 2005/05/20 01:12:19 lubing Exp $
 */
#include "selog.h"
#include "thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>

#define LOG_MAGIC        0x20050110 /* 用于判断句柄是否初始化 */
#define MAX_PATH_LEN           1024
#define MAX_PREFIX_LEN           80
#define MAX_LOG_LEN           10240
#define DEFAULT_TIMEFMT "%Y-%m-%d %H:%M:%S" /* 缺省时间格式 */
#define LOG_SUFFIX   "log"
#define ERR_SUFFIX   "err"

#define _1K (1024)
#define _1M (_1K*_1K)
#define DEFAULT_SWITCH_SIZE (_1K) /* 缺省轮换大小1K M == 1G */
#define MAX_SWITCH_SIZE   (2*_1K) /* 最大的轮换大小 2G */

typedef struct {
	/* 日志文件 */
	FILE* pfLog;
	char* pszSuffix; /* 后缀名 */
	
	/* 时间相关 */
	char szTime[_1K];
	struct tm tmLogTime;
	
	/* 和大小轮换相关的参数 */
	int iCurrNo;          /* 当前日志序号 */
	unsigned int iSize;   /* 当前日志大小 */
	
	/* 和时间轮换相关 */
	struct tm tmFile;
	
	/* 和缓冲相关 */
	char* psBuffer;
	/* int iBufLen; */
	
	/* 和线程相关 */
	HANDLEMUTEX mutexLog;
} LOG_DATA_T;

/* 日志句柄结构定义 */
typedef struct {
	int  iMagic; /* 是否初始化信息 */
	
	/* 初始化参数 */
	char szPath[MAX_PATH_LEN];
	char szPrefix[MAX_PREFIX_LEN];
	int  options;
	char szTimeFmt[MAX_PREFIX_LEN];
	int  iSwitchSize;
	int  iBufferSize;
	
	LOG_DATA_T ldLog;
	LOG_DATA_T ldErrLog;
} SE_LOG_T;

/* 创建日志文件 */
static int createLogFile(char* szFile, SE_LOG_T* pLog, LOG_DATA_T* pLogData)
{
	char *suffix = pLogData->pszSuffix;
	
	if (pLogData->pfLog == NULL)
	{
		if (pLog->options & SLO_SWITCH_BY_SIZE)
		{
			/* 文件以大小轮换 */
			struct stat sb;
			
			while (1)
			{
				if (pLog->options & SLO_SWITCH_BY_DAY)
				{
					/* 同时有日期轮换 */
					sprintf(szFile, "%s/%s_%04d%02d%02d_%d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, pLogData->iCurrNo, suffix);
				}
				else if (pLog->options & SLO_SWITCH_BY_HOUR)
				{
					/* 同时有时间轮换 */
					sprintf(szFile, "%s/%s_%04d%02d%02d_%02d_%d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, pLogData->tmFile.tm_hour, pLogData->iCurrNo, suffix);
				}
				else
				{
					/* 纯粹的大小轮换 */
					sprintf(szFile, "%s/%s_%d.%s", pLog->szPath, pLog->szPrefix, pLogData->iCurrNo, suffix);
				}
				
				if (stat(szFile, &sb) == 0)
				{
					if (sb.st_size >= pLog->iSwitchSize)
					{
						pLogData->iCurrNo++;
					}
					else
					{
						pLogData->iSize = sb.st_size;
						break;
					}
				}
				else
				{
					break;
				}
			}
		}
		else if (pLog->options & SLO_SWITCH_BY_DAY)
		{
			/* 文件以日轮换 */
			sprintf(szFile, "%s/%s_%04d%02d%02d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, suffix);
		}
		else if (pLog->options & SLO_SWITCH_BY_HOUR)
		{
			/* 文件以小时轮换 */
			sprintf(szFile, "%s/%s_%04d%02d%02d_%02d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, pLogData->tmFile.tm_hour, suffix);
		}
		else
		{
			/* 日志不轮换 */
			sprintf(szFile, "%s/%s.%s", pLog->szPath, pLog->szPrefix, suffix);
		}
		
		pLogData->pfLog = fopen(szFile, "a");
		if (pLogData->pfLog == NULL)
		{
			return 0; /* 创建文件失败 */
		}
		
		if (&(pLog->ldLog) == pLogData)
		{
			if (pLog->iBufferSize > 0)
				setvbuf(pLogData->pfLog, pLogData->psBuffer, _IOFBF, pLog->iBufferSize);
			/* else 使用系统缺省缓存 */
		}
		else
		{
			setvbuf(pLogData->pfLog, NULL, _IONBF, BUFSIZ);
		}
	}
	
	return 1;
}

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
int seLogInit(LOG_HANDLE* phLogHandle, const char *pszPath, const char *pszPrefix, int options, const char* pszTimeFmt, int iSwitchSize, int iBufferSize)
{
	time_t tm;
	SE_LOG_T* pLog;
	char szFile[MAX_PATH_LEN];
	
	if (phLogHandle == NULL || pszPath == NULL || pszPrefix == NULL)
		return -1; /* 参数错误 */
	
	if (options & SLO_SWITCH_BY_SIZE)
	{
		if (iSwitchSize < 10)
			return -2; /* iSwitchSize必须大于10M */
	}
	else
	{
		/* 在未指明大小轮换的时候使用缺省大小轮换 */
		options |= SLO_SWITCH_BY_SIZE;
		iSwitchSize = DEFAULT_SWITCH_SIZE;
	}
	if (iSwitchSize >= MAX_SWITCH_SIZE)
	{
		/* 大小轮换不能超过上限 */
		iSwitchSize = DEFAULT_SWITCH_SIZE;
	}
	
	if (options & SLO_BUFFERED_OUTPUT)
	{
		if (iBufferSize < 10)
			return -3; /* iBufferSize必须大于10K */
	}
	
	pLog = (SE_LOG_T*)calloc(1, sizeof(SE_LOG_T));
	if (pLog == NULL)
		return -4; /* 申请句柄空间失败 */
	
	if (options & SLO_BUFFERED_OUTPUT)
	{
		/* 需要缓冲 */
		pLog->iBufferSize = iBufferSize * _1K;
		pLog->ldLog.psBuffer = (char*)malloc(pLog->iBufferSize);
		if (pLog->ldLog.psBuffer == NULL)
		{
			free(pLog);
			return -5; /* 申请缓冲空间失败 */
		}
	}
	
	/* 复制初始化信息 */
	strncpy(pLog->szPath, pszPath, MAX_PATH_LEN-1);
	strncpy(pLog->szPrefix, pszPrefix, MAX_PREFIX_LEN-1);
	if (pszTimeFmt != NULL)
	{
		strncpy(pLog->szTimeFmt, pszTimeFmt, MAX_PREFIX_LEN-1);
	}
	else
	{
		strcpy(pLog->szTimeFmt, DEFAULT_TIMEFMT);
	}
	pLog->options = options;
	if (options & SLO_SWITCH_BY_SIZE)
	{
		/* 以大小轮换 */
		pLog->iSwitchSize = iSwitchSize * _1M;
	}
	if (options & SLO_SWITCH_BY_DAY)
	{
		/* 以天轮换 */
		pLog->options &= (-1 ^ SLO_SWITCH_BY_HOUR); /* 忽略小时轮换 */
		tm = time(NULL);
		localtime_r(&tm, &(pLog->ldLog.tmFile));
		localtime_r(&tm, &(pLog->ldErrLog.tmFile));
	}
	else if (options & SLO_SWITCH_BY_HOUR)
	{
		/* 以小时轮换 */
		tm = time(NULL);
		localtime_r(&tm, &(pLog->ldLog.tmFile));
		localtime_r(&tm, &(pLog->ldErrLog.tmFile));
	}
	
	pLog->ldLog.pszSuffix = LOG_SUFFIX;
	if (!createLogFile(szFile, pLog, &(pLog->ldLog)))
	{
		if (pLog->ldLog.psBuffer != NULL)
		{
			free(pLog->ldLog.psBuffer);
		}
		free(pLog);
		return -6; /* 创建日志文件失败 */
	}
	pLog->ldErrLog.pszSuffix = ERR_SUFFIX;
	if (!createLogFile(szFile, pLog, &(pLog->ldErrLog)))
	{
		fclose(pLog->ldLog.pfLog);
		if (pLog->ldLog.psBuffer != NULL)
		{
			free(pLog->ldLog.psBuffer);
		}
		free(pLog);
		return -7; /* 创建日志文件失败 */
	}
	
	/* 成功返回 */
	pLog->iMagic = LOG_MAGIC;
	InitMutex(&(pLog->ldLog.mutexLog));
	InitMutex(&(pLog->ldErrLog.mutexLog));
	*phLogHandle = (void*)pLog;
	return 0; 
}

/* 取当前时间并格式化 */
static void getCurrTime(SE_LOG_T* pLog, LOG_DATA_T* pLogData)
{
	time_t tm;
	tm = time(NULL);
	localtime_r(&tm, &(pLogData->tmLogTime));
	strftime(pLogData->szTime, _1K, pLog->szTimeFmt, &(pLogData->tmLogTime));
}

/* 输出正常日志 */
static int seInnerLog(const char* szFileName, int iLine, SE_LOG_T* pLog, LOG_DATA_T* pLogData, const char* pszStr, int iLogLen2)
{
	char szFile[MAX_PATH_LEN];
	char szLog[MAX_LOG_LEN]; /* 保存当前日志 */
	char *pszTmpLog = NULL;  /* 当前日志大于MAX_LOG_LEN时，动态申请空间保存当前日志 */
	int iLogLen = 0;
	
	/* 合成日志 */
	if (pLog->options & SLO_OUTPUT_PID)
	{
		/* 需要输出pid */
		iLogLen = sprintf(szLog, "%u\t", getpid());
	}
	
	/* 输出文件名和行号，用于错误日志 */
	if (szFileName != NULL)
	{
		iLogLen += sprintf(szLog + iLogLen, "[%s:%d]\t", szFileName, iLine);
	}
	
	/* 输出时间 */
	getCurrTime(pLog, pLogData);
	iLogLen += sprintf(szLog + iLogLen, "%s\t", pLogData->szTime);
	
	/* 把日志合成完整的字符串 */
	if (iLogLen + iLogLen2 < MAX_LOG_LEN)
	{
		strcpy(szLog+iLogLen, pszStr);
	}
	else
	{
		/* 当前日志大于MAX_LOG_LEN */
		pszTmpLog = (char*)malloc(iLogLen + iLogLen2+2);
		if (pszTmpLog == NULL)
		{
			return -4; /* 申请内存失败 */
		}
		memcpy(pszTmpLog, szLog, iLogLen);
		strcpy(pszTmpLog+iLogLen, pszStr);
	}
	iLogLen += iLogLen2;
	if(pszTmpLog==NULL)
		szLog[iLogLen++] = '\n';
	else
		pszTmpLog[iLogLen++] = '\n';

	/* 判断是否需要时间轮换 */
	if ( ((pLog->options & SLO_SWITCH_BY_DAY) && (pLogData->tmFile.tm_mday != pLogData->tmLogTime.tm_mday)) ||
		 ((pLog->options & SLO_SWITCH_BY_HOUR) &&
		  (pLogData->tmFile.tm_mday != pLogData->tmLogTime.tm_mday || pLogData->tmFile.tm_hour != pLogData->tmLogTime.tm_hour))
	   )
	{
		memcpy(&(pLogData->tmFile), &(pLogData->tmLogTime), sizeof(struct tm)); /* 记录新的时间 */
		
		fclose(pLogData->pfLog);
		pLogData->pfLog = NULL;
		pLogData->iSize = 0;
		pLogData->iCurrNo = 0; /* 如果同时有大小轮换，编号从0开始 */
		
		/* 生成新的日志文件 */
		if (!createLogFile(szFile, pLog, pLogData))
		{
			if (pszTmpLog != NULL)
				free(pszTmpLog);
			return -5; /* 创建日志文件失败 */
		}
	}
	
	/* 输出到日志文件 */
	fwrite(pszTmpLog?pszTmpLog:szLog, 1, iLogLen, pLogData->pfLog);
	fflush(pLogData->pfLog);
	//fwrite(pszTmpLog?pszTmpLog:szLog, 1, iLogLen, stderr);
	if (pszTmpLog != NULL)
		free(pszTmpLog);
	pLogData->iSize += iLogLen;
	
	/* 是否需要大小轮换 */
	if ((pLog->options & SLO_SWITCH_BY_SIZE) && (pLogData->iSize >= pLog->iSwitchSize))
	{
		fclose(pLogData->pfLog);
		pLogData->pfLog = NULL;
		pLogData->iSize = 0;
		pLogData->iCurrNo++;
		
		if (!createLogFile(szFile, pLog, pLogData))
		{
			return -7; /* 创建文件失败 */
		}
	}
	
	return 0;
}

/**
 * 输出正常工作日志
 * 参数:
 *   hLogHandle: 正常调用seLogInit的日志句柄
 *   pszFormat:  输出日志的格式化串，参考printf
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seLog(LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* 参数错误 */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* 参数错误 */
	
	/* 合成自定义日志 */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* 合成错误 */
	}
	
	ret = seInnerLog(NULL, 0, pLog, &(pLog->ldLog), pszStr, iLogLen2);
	free(pszStr);
	
	return ret;
}

/**
 * 输出正常工作日志，线程安全
 * 参数:
 *   hLogHandle: 正常调用seLogInit的日志句柄
 *   pszFormat:  输出日志的格式化串，参考printf
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seLogEx(LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* 参数错误 */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* 参数错误 */
	
	/* 合成自定义日志 */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* 合成错误 */
	}
	
	LOCK_MUTEX(pLog->ldLog.mutexLog);
	ret = seInnerLog(NULL, 0, pLog, &(pLog->ldLog), pszStr, iLogLen2);
	UNLOCK_MUTEX(pLog->ldLog.mutexLog);
	
	free(pszStr);
	
	return ret;
}

/**
 * 输出缓存中的工作日志[线程安全的函数]
 * 参数:
 *   hLogHandle: 正常调用seLogInit的日志句柄
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seFlushLog(LOG_HANDLE hLogHandle)
{
	SE_LOG_T* pLog;
	LOG_DATA_T* pLogData;
	char szFile[MAX_PATH_LEN];

	if (hLogHandle == NULL)
		return -1; /* 参数错误 */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* 参数错误 */
	
	pLogData = &(pLog->ldLog);
	/* 清空操作系统的文件缓冲 */
	fflush(pLogData->pfLog);
	return 0;
}

int seFlushErrLog(LOG_HANDLE hLogHandle)
{
	return 0;
}

/**
 * 输出错误日志实现
 * 参数:
 *   szFileName: 出错的源程序文件名
 *   iLine:      出错的代码行
 *   hLogHandle: 正常调用seLogInit的日志句柄
 *   pszFormat:  输出日志的格式化串，参考printf
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seErrLogImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* 参数错误 */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* 参数错误 */
	
	/* 合成自定义日志 */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* 输出错误 */
	}
	ret = seInnerLog(szFileName, iLine, pLog, &(pLog->ldErrLog), pszStr, iLogLen2);
	free(pszStr);
	
	return ret;
}

/**
 * 输出错误日志实现，多线程安全
 * 参数:
 *   szFileName: 出错的源程序文件名
 *   iLine:      出错的代码行
 *   hLogHandle: 正常调用seLogInit的日志句柄
 *   pszFormat:  输出日志的格式化串，参考printf
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seErrLogExImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* 参数错误 */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* 参数错误 */
	
	/* 合成自定义日志 */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* 输出错误 */
	}
	
	LOCK_MUTEX(pLog->ldErrLog.mutexLog);
	ret = seInnerLog(szFileName, iLine, pLog, &(pLog->ldErrLog), pszStr, iLogLen2);
	UNLOCK_MUTEX(pLog->ldErrLog.mutexLog);
	
	free(pszStr);
	
	return ret;
}

/**
 * 关闭日志，释放各种资源
 * 参数:
 *   phLogHandle: 正常调用seLogInit的日志句柄
 * 返回:
 *   0: 成功
 *   <0: 各种错误码
 */
int seCloseLog(LOG_HANDLE* phLogHandle)
{
	SE_LOG_T* pLog;

	if (phLogHandle == NULL)
	{
		return 0;
	}
	
	pLog = (SE_LOG_T*)*phLogHandle;
	if (pLog == NULL)
	{
		return 0;
	}
	
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* 参数错误 */
	
	DESTROY_MUTEX(pLog->ldLog.mutexLog);
	DESTROY_MUTEX(pLog->ldErrLog.mutexLog);

	if (pLog->ldLog.pfLog != NULL)
	{
		fclose(pLog->ldLog.pfLog);
	}
	if (pLog->ldErrLog.pfLog)
	{
		fclose(pLog->ldErrLog.pfLog);
	}
	
	/* 释放资源 */
	if (pLog->ldLog.psBuffer != NULL)
	{
		free(pLog->ldLog.psBuffer);
	}
	free(pLog);
	*phLogHandle = NULL;
	
	return 0;
}
