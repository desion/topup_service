/*
 * Copyright (c) 2003, 新浪网研发中心
 * All rights reserved.
 *
 * 文件名称：slog.c
 * 摘    要：slog.h的实现
 * 作    者：Bill Duan, jingjing
 * $Id: slog.c,v 1.6 2006/06/22 08:05:24 jingjing Exp $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include "slog.h"

#ifdef DLOG
#include "dlog.h"
#endif

#define MAX_PATH_LEN            256         /* 最大路径长度 */
#define DEF_MAX_LOG_LEN 64000
#define MAX_LOG_MSG_LEN 1024

typedef struct
{
	TLogLevel minLevel;
	int maxLen;
	TLogSpec spec;
	char pathName[MAX_PATH_LEN];
	FILE *fp;
}TProcessLogConf;

static TProcessLogConf logConf;
static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;
static int openFlag = 0;
static pthread_key_t logThrTagKey;
static int keyFlag = 0;

int slog_open(const char *logDir,const char *processName, TLogConf *conf)
{
	time_t tt;
	char tmpBuf[MAX_LINE_LEN];

	if (!logDir || !processName)
		return -1;

    // 设置log
	if (conf)
	{
		logConf.minLevel = conf->minLevel;
		logConf.maxLen = conf->maxLen;
		logConf.spec = conf->spec;

		if (logConf.maxLen <= 0)
			logConf.maxLen = DEF_MAX_LOG_LEN;
	}
	else
	{
		logConf.minLevel = LL_ALL;
		logConf.maxLen = 64000;
		memset(&(logConf.spec), 0, sizeof(TLogSpec));
		logConf.spec.log2TTY = 1;
	}

    // 打开日志文件
	snprintf(logConf.pathName, MAX_PATH_LEN, "%s/%slog", logDir, processName);
	if (!(logConf.fp=fopen(logConf.pathName,"a+")))
		return -1;

	openFlag=1;

    // 生成线程私有数据(日志标志)
	if (pthread_key_create(&logThrTagKey, free) != 0)
	{
		fclose(logConf.fp);
		return -1;
	}
	keyFlag = 1;

    // 日志记录
	time(&tt);
	ctime_r(&tt,tmpBuf);

	fprintf(logConf.fp,"===============================================\n");
	fprintf(logConf.fp,"Open process log at %s",tmpBuf);
	fprintf(logConf.fp,"===============================================\n");
	fflush(logConf.fp);

	fprintf(stderr,"===============================================\n");
	fprintf(stderr,"Open process log at %s",tmpBuf);
	fprintf(stderr,"===============================================\n");
	return 0;
}

// 返回各自线程的日志标志指针
const char **slog_thr_tag(void)
{
	const char **thrTag = pthread_getspecific(logThrTagKey);
	if (thrTag == NULL)
	{
		thrTag = malloc(sizeof(const char **));
		*thrTag = NULL;
		pthread_setspecific(logThrTagKey, thrTag);
	}
	return thrTag;
}

int slog_write(int level, const char *format, ... )
{
	va_list args;
	char msg[MAX_LOG_MSG_LEN];
	char *p;
	time_t tt;
	struct tm ttm;
	pthread_t tid;
	int flen;
	int len;
	const char *thrTag;

	if (level<logConf.minLevel)
		return 0;

	p = msg;

	// log等级
	switch (level)
	{
	case LL_DEBUG:
		strcpy(p, "DEBUG ");
		p += 6;
		break;
	case LL_TRACE:
		strcpy(p, "TRACE ");
		p += 6;
		break;
	case LL_NOTICE:
		strcpy(p, "NOTICE ");
		p += 7;
		break;
	case LL_WARNING:
		strcpy(p, "WARNING ");
		p += 8;
		break;
	case LL_FATAL:
		strcpy(p, "FATAL ");
		p += 6;
		break;
	default:
		fprintf(stderr, "Invalid log level '%d'.\n", level);
		return -1;
	}
	// 时间
	time(&tt);
	localtime_r(&tt, &ttm);
	p += sprintf(p, "%d-%d-%d %02d:%02d:%02d ", ttm.tm_year + 1900, ttm.tm_mon + 1, 
				 ttm.tm_mday, ttm.tm_hour, ttm.tm_min, ttm.tm_sec);
	// 线程
	if (keyFlag && (thrTag = slogThrTag))
	{
		// 使thrTag的长度不超过30个字节
		*(p + 30) = 0;
		strncpy(p, thrTag, 30);
		p += strlen(p);
		strcpy(p, "# ");
		p += 2;
	}
	else
	{
		tid = pthread_self();
		p += sprintf(p, "%lu# ", tid);
	}

	va_start(args,format);
	len=vsnprintf(p,MAX_LOG_MSG_LEN-(p-msg)-1,format,args);
#ifdef DLOG
	if (level == LL_FATAL)
	{
		vdlog("FATAL", format, args);
	}
#endif
	va_end(args);

	if (len>=MAX_LOG_MSG_LEN-(p-msg)-1)
		len=MAX_LOG_MSG_LEN-(p-msg)-2;
	p+=len;

	if (*(p-1)!='\n')
		*p++='\n';
	*p=0;

    // 判断日志长度
	if (openFlag)
	{
		pthread_mutex_lock(&logMutex);

		if (fseek(logConf.fp,0,SEEK_END)==-1
			|| (flen=ftell(logConf.fp))==-1)
		{
			fprintf(stderr,"Fail to get length of file '%s'.\n",logConf.pathName);
			return -1;
		}

		if (flen>=logConf.maxLen)
		{
			fclose(logConf.fp);
			openFlag=0;

			if (truncate(logConf.pathName,0)==-1)
			{
				fprintf(stderr,"Fail to truncate file '%s'.\n",logConf.pathName);
				return -1;
			}
			if (!(logConf.fp=fopen(logConf.pathName,"a+")))
				return -1;
			openFlag=1;
		}

		fprintf(logConf.fp,"%s",msg);
		fflush(logConf.fp);

		pthread_mutex_unlock(&logMutex);

		if (logConf.spec.log2TTY==1)
			fprintf(stderr,"%s",msg);
	}
	else
		fprintf(stderr,"%s",msg);

	return 0;
}

int slog_close(int isErr)
{
	time_t tt;
	char tmpBuf[MAX_LINE_LEN];

	if (isErr)
		snprintf(tmpBuf,MAX_LINE_LEN,"Abnormally close process log at ");
	else
		snprintf(tmpBuf,MAX_LINE_LEN,"Close process log at ");

	time(&tt);
	ctime_r(&tt,tmpBuf+strlen(tmpBuf));

	fprintf(logConf.fp,"===============================================\n");
	fprintf(logConf.fp,"%s",tmpBuf);
	fprintf(logConf.fp,"===============================================\n");
	fflush(logConf.fp);

	fprintf(stderr,"===============================================\n");
	fprintf(stderr,"%s",tmpBuf);
	fprintf(stderr,"===============================================\n");

	openFlag=0;

	fclose(logConf.fp);
	logConf.fp=NULL;
	pthread_key_delete(logThrTagKey);

	return 0;
}
