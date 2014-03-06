/**
 * 1. 启动线程函数。可以在Unix和Win32平台通用
 * 2. 信号量(mutex)的处理。可以在Unix和Win32平台通用
 * 使用方法见thread.h
 *
 * Xu Lubing
 * Sept. 25, 2003
 */
#include "thread.h"

/**
 * 启动新线程。
 * 参数：
 *   threadFunc: 线程函数，具有原型void threadFunc(void*)
 *   arg:        传递给线程函数的参数
 * 返回：
 *   0: 创建失败
 *   非0: 线程句柄
 */
HANDLETHREAD StartThread(LPTHREAD_START_ROUTINE threadFunc, void* arg)
{
	HANDLETHREAD hThread;
	
#ifndef WIN32
	if (pthread_create(&hThread, 0, threadFunc, arg) != 0)
	{
		return 0;
	}
#else
	DWORD hThreadID;
	hThread = CreateThread(NULL, 0, threadFunc, arg, 0, &hThreadID);
#endif
	
	return hThread;
}

/**
 * 创建并初始化信号量
 * 参数:
 *   mutex: 要初始化的信号量
 * 返回:
 *   0: 初始化失败
 *   非0: 成功
 */
int InitMutex(HANDLEMUTEX* mutex)
{
#ifndef WIN32
	if (pthread_mutex_init(mutex, NULL) != 0)
	{
		return 0;
	}
#else
	*mutex = CreateMutex(NULL, FALSE, NULL);
	if (*mutex == NULL)
	{
		return 0;
	}
#endif

	return 1;
}

