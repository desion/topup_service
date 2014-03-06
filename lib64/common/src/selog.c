/*
 * Copyright (c) 2005, �������з�����
 * All rights reserved.
 *
 * �ļ����ƣ�selog.c
 * ժ    Ҫ��ͳһ��־����
 *         . ��־������/�쳣֮��
 *         . ��־��ʼ��ʱָ��·������־��ʶ
 *         . ��־����ͨ�õ�pid(��ѡ)��ʱ����Ϣ������ΪΪ�Զ�����Ϣ
 *         . ��־����ʱ֧���ļ��ֻ����ֻ���ʽ���԰�ʱ����С���У��ڳ�ʼ��ʱָ��
 *         . ���̰߳�ȫ
 *         . Ϊ����д��־Ӱ�칤���ٶȣ���־���Լӻ��壬���Դ�����־�����塣
 * ��    �ߣ����̱�, 2005.1.11
 * ��    ��: 2005.2.28 ���������־ʱ����ѡ���Ƿ����������ļ������к�
 *           2005.4.1  �����־����ȥ���˱������Զ��建�����ִ�е�����
 *           2005.4.8  ���κ�����¶�ʹ�ô�С�ֻ����ƣ�������ȱʡ�ֻ�
 *           2005.5.10 ������Ƹ�Ϊʹ��glic��setvbuf����ʵ�֣����˴���
 *           2005.5.19 ��������ʱ������־��ʼ���������ԭ�е���־
 *					 2007.4.16 �޸���־�ļ��������������Ϣ��
 * ��    ����1.0
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

#define LOG_MAGIC        0x20050110 /* �����жϾ���Ƿ��ʼ�� */
#define MAX_PATH_LEN           1024
#define MAX_PREFIX_LEN           80
#define MAX_LOG_LEN           10240
#define DEFAULT_TIMEFMT "%Y-%m-%d %H:%M:%S" /* ȱʡʱ���ʽ */
#define LOG_SUFFIX   "log"
#define ERR_SUFFIX   "err"

#define _1K (1024)
#define _1M (_1K*_1K)
#define DEFAULT_SWITCH_SIZE (_1K) /* ȱʡ�ֻ���С1K M == 1G */
#define MAX_SWITCH_SIZE   (2*_1K) /* �����ֻ���С 2G */

typedef struct {
	/* ��־�ļ� */
	FILE* pfLog;
	char* pszSuffix; /* ��׺�� */
	
	/* ʱ����� */
	char szTime[_1K];
	struct tm tmLogTime;
	
	/* �ʹ�С�ֻ���صĲ��� */
	int iCurrNo;          /* ��ǰ��־��� */
	unsigned int iSize;   /* ��ǰ��־��С */
	
	/* ��ʱ���ֻ���� */
	struct tm tmFile;
	
	/* �ͻ������ */
	char* psBuffer;
	/* int iBufLen; */
	
	/* ���߳���� */
	HANDLEMUTEX mutexLog;
} LOG_DATA_T;

/* ��־����ṹ���� */
typedef struct {
	int  iMagic; /* �Ƿ��ʼ����Ϣ */
	
	/* ��ʼ������ */
	char szPath[MAX_PATH_LEN];
	char szPrefix[MAX_PREFIX_LEN];
	int  options;
	char szTimeFmt[MAX_PREFIX_LEN];
	int  iSwitchSize;
	int  iBufferSize;
	
	LOG_DATA_T ldLog;
	LOG_DATA_T ldErrLog;
} SE_LOG_T;

/* ������־�ļ� */
static int createLogFile(char* szFile, SE_LOG_T* pLog, LOG_DATA_T* pLogData)
{
	char *suffix = pLogData->pszSuffix;
	
	if (pLogData->pfLog == NULL)
	{
		if (pLog->options & SLO_SWITCH_BY_SIZE)
		{
			/* �ļ��Դ�С�ֻ� */
			struct stat sb;
			
			while (1)
			{
				if (pLog->options & SLO_SWITCH_BY_DAY)
				{
					/* ͬʱ�������ֻ� */
					sprintf(szFile, "%s/%s_%04d%02d%02d_%d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, pLogData->iCurrNo, suffix);
				}
				else if (pLog->options & SLO_SWITCH_BY_HOUR)
				{
					/* ͬʱ��ʱ���ֻ� */
					sprintf(szFile, "%s/%s_%04d%02d%02d_%02d_%d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, pLogData->tmFile.tm_hour, pLogData->iCurrNo, suffix);
				}
				else
				{
					/* ����Ĵ�С�ֻ� */
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
			/* �ļ������ֻ� */
			sprintf(szFile, "%s/%s_%04d%02d%02d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, suffix);
		}
		else if (pLog->options & SLO_SWITCH_BY_HOUR)
		{
			/* �ļ���Сʱ�ֻ� */
			sprintf(szFile, "%s/%s_%04d%02d%02d_%02d.%s", pLog->szPath, pLog->szPrefix, pLogData->tmFile.tm_year+1900, pLogData->tmFile.tm_mon+1, pLogData->tmFile.tm_mday, pLogData->tmFile.tm_hour, suffix);
		}
		else
		{
			/* ��־���ֻ� */
			sprintf(szFile, "%s/%s.%s", pLog->szPath, pLog->szPrefix, suffix);
		}
		
		pLogData->pfLog = fopen(szFile, "a");
		if (pLogData->pfLog == NULL)
		{
			return 0; /* �����ļ�ʧ�� */
		}
		
		if (&(pLog->ldLog) == pLogData)
		{
			if (pLog->iBufferSize > 0)
				setvbuf(pLogData->pfLog, pLogData->psBuffer, _IOFBF, pLog->iBufferSize);
			/* else ʹ��ϵͳȱʡ���� */
		}
		else
		{
			setvbuf(pLogData->pfLog, NULL, _IONBF, BUFSIZ);
		}
	}
	
	return 1;
}

/**
 * ��־��ʼ��
 * ����:
 *   phLogHandle: ���ڱ����ʼ�������־����������ڵ��ú�������ʱʹ��
 *   pszPath:     ��־���·��
 *   pszPrefix:   ��־�ļ���ǰ׺
 *   options:     ��־ѡ������壬��ѡ�����"��"��
 *                ���ѡ��SLO_SWITCH_BY_DAY���֣�SLO_SWITCH_BY_HOUR��������
 *   pszTimeFmt   ����ʱ��������ʽ��Ϊ����strftime��format�ַ�����ΪNULLʱʹ��"%Y-%m-%d %H:%M:%S"
 *   iSwitchSize: ���ֻ�����Ĵ�С(��λM)����ֵ��ѡ��SLO_SWITCH_BY_SIZEָ��ʱ�����ṩ�ұ������10M���޴�ѡ��ʱ������ʹ��ȱʡ�ֻ�ֵ
 *   iBufferSize: ������־�����С(��λK)����ֵ��ѡ��SLO_BUFFERED_OUTPUTָ��ʱ�����ṩ�ұ������10K���޴�ѡ��ʱ����
 * ����:
 *   0: ��ʼ���ɹ���phLogHandle������־���
 *   <0:��ʼ��ʧ�ܣ����ִ�����
 * [˵��] �ڲ�ָ����С�ֻ�������£�ȱʡ��СΪ1G���ֻ���С���>=2048M(2G)��Ҳ��ʹ��ȱʡ�ֻ���С
 */
int seLogInit(LOG_HANDLE* phLogHandle, const char *pszPath, const char *pszPrefix, int options, const char* pszTimeFmt, int iSwitchSize, int iBufferSize)
{
	time_t tm;
	SE_LOG_T* pLog;
	char szFile[MAX_PATH_LEN];
	
	if (phLogHandle == NULL || pszPath == NULL || pszPrefix == NULL)
		return -1; /* �������� */
	
	if (options & SLO_SWITCH_BY_SIZE)
	{
		if (iSwitchSize < 10)
			return -2; /* iSwitchSize�������10M */
	}
	else
	{
		/* ��δָ����С�ֻ���ʱ��ʹ��ȱʡ��С�ֻ� */
		options |= SLO_SWITCH_BY_SIZE;
		iSwitchSize = DEFAULT_SWITCH_SIZE;
	}
	if (iSwitchSize >= MAX_SWITCH_SIZE)
	{
		/* ��С�ֻ����ܳ������� */
		iSwitchSize = DEFAULT_SWITCH_SIZE;
	}
	
	if (options & SLO_BUFFERED_OUTPUT)
	{
		if (iBufferSize < 10)
			return -3; /* iBufferSize�������10K */
	}
	
	pLog = (SE_LOG_T*)calloc(1, sizeof(SE_LOG_T));
	if (pLog == NULL)
		return -4; /* �������ռ�ʧ�� */
	
	if (options & SLO_BUFFERED_OUTPUT)
	{
		/* ��Ҫ���� */
		pLog->iBufferSize = iBufferSize * _1K;
		pLog->ldLog.psBuffer = (char*)malloc(pLog->iBufferSize);
		if (pLog->ldLog.psBuffer == NULL)
		{
			free(pLog);
			return -5; /* ���뻺��ռ�ʧ�� */
		}
	}
	
	/* ���Ƴ�ʼ����Ϣ */
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
		/* �Դ�С�ֻ� */
		pLog->iSwitchSize = iSwitchSize * _1M;
	}
	if (options & SLO_SWITCH_BY_DAY)
	{
		/* �����ֻ� */
		pLog->options &= (-1 ^ SLO_SWITCH_BY_HOUR); /* ����Сʱ�ֻ� */
		tm = time(NULL);
		localtime_r(&tm, &(pLog->ldLog.tmFile));
		localtime_r(&tm, &(pLog->ldErrLog.tmFile));
	}
	else if (options & SLO_SWITCH_BY_HOUR)
	{
		/* ��Сʱ�ֻ� */
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
		return -6; /* ������־�ļ�ʧ�� */
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
		return -7; /* ������־�ļ�ʧ�� */
	}
	
	/* �ɹ����� */
	pLog->iMagic = LOG_MAGIC;
	InitMutex(&(pLog->ldLog.mutexLog));
	InitMutex(&(pLog->ldErrLog.mutexLog));
	*phLogHandle = (void*)pLog;
	return 0; 
}

/* ȡ��ǰʱ�䲢��ʽ�� */
static void getCurrTime(SE_LOG_T* pLog, LOG_DATA_T* pLogData)
{
	time_t tm;
	tm = time(NULL);
	localtime_r(&tm, &(pLogData->tmLogTime));
	strftime(pLogData->szTime, _1K, pLog->szTimeFmt, &(pLogData->tmLogTime));
}

/* ���������־ */
static int seInnerLog(const char* szFileName, int iLine, SE_LOG_T* pLog, LOG_DATA_T* pLogData, const char* pszStr, int iLogLen2)
{
	char szFile[MAX_PATH_LEN];
	char szLog[MAX_LOG_LEN]; /* ���浱ǰ��־ */
	char *pszTmpLog = NULL;  /* ��ǰ��־����MAX_LOG_LENʱ����̬����ռ䱣�浱ǰ��־ */
	int iLogLen = 0;
	
	/* �ϳ���־ */
	if (pLog->options & SLO_OUTPUT_PID)
	{
		/* ��Ҫ���pid */
		iLogLen = sprintf(szLog, "%u\t", getpid());
	}
	
	/* ����ļ������кţ����ڴ�����־ */
	if (szFileName != NULL)
	{
		iLogLen += sprintf(szLog + iLogLen, "[%s:%d]\t", szFileName, iLine);
	}
	
	/* ���ʱ�� */
	getCurrTime(pLog, pLogData);
	iLogLen += sprintf(szLog + iLogLen, "%s\t", pLogData->szTime);
	
	/* ����־�ϳ��������ַ��� */
	if (iLogLen + iLogLen2 < MAX_LOG_LEN)
	{
		strcpy(szLog+iLogLen, pszStr);
	}
	else
	{
		/* ��ǰ��־����MAX_LOG_LEN */
		pszTmpLog = (char*)malloc(iLogLen + iLogLen2+2);
		if (pszTmpLog == NULL)
		{
			return -4; /* �����ڴ�ʧ�� */
		}
		memcpy(pszTmpLog, szLog, iLogLen);
		strcpy(pszTmpLog+iLogLen, pszStr);
	}
	iLogLen += iLogLen2;
	if(pszTmpLog==NULL)
		szLog[iLogLen++] = '\n';
	else
		pszTmpLog[iLogLen++] = '\n';

	/* �ж��Ƿ���Ҫʱ���ֻ� */
	if ( ((pLog->options & SLO_SWITCH_BY_DAY) && (pLogData->tmFile.tm_mday != pLogData->tmLogTime.tm_mday)) ||
		 ((pLog->options & SLO_SWITCH_BY_HOUR) &&
		  (pLogData->tmFile.tm_mday != pLogData->tmLogTime.tm_mday || pLogData->tmFile.tm_hour != pLogData->tmLogTime.tm_hour))
	   )
	{
		memcpy(&(pLogData->tmFile), &(pLogData->tmLogTime), sizeof(struct tm)); /* ��¼�µ�ʱ�� */
		
		fclose(pLogData->pfLog);
		pLogData->pfLog = NULL;
		pLogData->iSize = 0;
		pLogData->iCurrNo = 0; /* ���ͬʱ�д�С�ֻ�����Ŵ�0��ʼ */
		
		/* �����µ���־�ļ� */
		if (!createLogFile(szFile, pLog, pLogData))
		{
			if (pszTmpLog != NULL)
				free(pszTmpLog);
			return -5; /* ������־�ļ�ʧ�� */
		}
	}
	
	/* �������־�ļ� */
	fwrite(pszTmpLog?pszTmpLog:szLog, 1, iLogLen, pLogData->pfLog);
	fflush(pLogData->pfLog);
	//fwrite(pszTmpLog?pszTmpLog:szLog, 1, iLogLen, stderr);
	if (pszTmpLog != NULL)
		free(pszTmpLog);
	pLogData->iSize += iLogLen;
	
	/* �Ƿ���Ҫ��С�ֻ� */
	if ((pLog->options & SLO_SWITCH_BY_SIZE) && (pLogData->iSize >= pLog->iSwitchSize))
	{
		fclose(pLogData->pfLog);
		pLogData->pfLog = NULL;
		pLogData->iSize = 0;
		pLogData->iCurrNo++;
		
		if (!createLogFile(szFile, pLog, pLogData))
		{
			return -7; /* �����ļ�ʧ�� */
		}
	}
	
	return 0;
}

/**
 * �������������־
 * ����:
 *   hLogHandle: ��������seLogInit����־���
 *   pszFormat:  �����־�ĸ�ʽ�������ο�printf
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seLog(LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* �������� */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* �������� */
	
	/* �ϳ��Զ�����־ */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* �ϳɴ��� */
	}
	
	ret = seInnerLog(NULL, 0, pLog, &(pLog->ldLog), pszStr, iLogLen2);
	free(pszStr);
	
	return ret;
}

/**
 * �������������־���̰߳�ȫ
 * ����:
 *   hLogHandle: ��������seLogInit����־���
 *   pszFormat:  �����־�ĸ�ʽ�������ο�printf
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seLogEx(LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* �������� */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* �������� */
	
	/* �ϳ��Զ�����־ */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* �ϳɴ��� */
	}
	
	LOCK_MUTEX(pLog->ldLog.mutexLog);
	ret = seInnerLog(NULL, 0, pLog, &(pLog->ldLog), pszStr, iLogLen2);
	UNLOCK_MUTEX(pLog->ldLog.mutexLog);
	
	free(pszStr);
	
	return ret;
}

/**
 * ��������еĹ�����־[�̰߳�ȫ�ĺ���]
 * ����:
 *   hLogHandle: ��������seLogInit����־���
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seFlushLog(LOG_HANDLE hLogHandle)
{
	SE_LOG_T* pLog;
	LOG_DATA_T* pLogData;
	char szFile[MAX_PATH_LEN];

	if (hLogHandle == NULL)
		return -1; /* �������� */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* �������� */
	
	pLogData = &(pLog->ldLog);
	/* ��ղ���ϵͳ���ļ����� */
	fflush(pLogData->pfLog);
	return 0;
}

int seFlushErrLog(LOG_HANDLE hLogHandle)
{
	return 0;
}

/**
 * ���������־ʵ��
 * ����:
 *   szFileName: �����Դ�����ļ���
 *   iLine:      ����Ĵ�����
 *   hLogHandle: ��������seLogInit����־���
 *   pszFormat:  �����־�ĸ�ʽ�������ο�printf
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seErrLogImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* �������� */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* �������� */
	
	/* �ϳ��Զ�����־ */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* ������� */
	}
	ret = seInnerLog(szFileName, iLine, pLog, &(pLog->ldErrLog), pszStr, iLogLen2);
	free(pszStr);
	
	return ret;
}

/**
 * ���������־ʵ�֣����̰߳�ȫ
 * ����:
 *   szFileName: �����Դ�����ļ���
 *   iLine:      ����Ĵ�����
 *   hLogHandle: ��������seLogInit����־���
 *   pszFormat:  �����־�ĸ�ʽ�������ο�printf
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
 */
int seErrLogExImpl(const char* szFileName, int iLine, LOG_HANDLE hLogHandle, const char *pszFormat, ...)
{
	SE_LOG_T* pLog;
	va_list ap;
	char *pszStr = NULL;
	int iLogLen2;
	int ret;
	
	if (hLogHandle == NULL || pszFormat == NULL)
		return -1; /* �������� */
	
	pLog = (SE_LOG_T*)hLogHandle;
	if (pLog->iMagic != LOG_MAGIC)
		return -2; /* �������� */
	
	/* �ϳ��Զ�����־ */
	va_start(ap, pszFormat);
	iLogLen2 = vasprintf(&pszStr, pszFormat, ap);
	va_end(ap);
	if (iLogLen2 == -1)
	{
		return -3; /* ������� */
	}
	
	LOCK_MUTEX(pLog->ldErrLog.mutexLog);
	ret = seInnerLog(szFileName, iLine, pLog, &(pLog->ldErrLog), pszStr, iLogLen2);
	UNLOCK_MUTEX(pLog->ldErrLog.mutexLog);
	
	free(pszStr);
	
	return ret;
}

/**
 * �ر���־���ͷŸ�����Դ
 * ����:
 *   phLogHandle: ��������seLogInit����־���
 * ����:
 *   0: �ɹ�
 *   <0: ���ִ�����
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
		return -2; /* �������� */
	
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
	
	/* �ͷ���Դ */
	if (pLog->ldLog.psBuffer != NULL)
	{
		free(pLog->ldLog.psBuffer);
	}
	free(pLog);
	*phLogHandle = NULL;
	
	return 0;
}
