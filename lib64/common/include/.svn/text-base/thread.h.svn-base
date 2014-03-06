/**
 * 1. 启动线程函数。可以在Unix和Win32平台通用
 * 2. 信号量(mutex)的处理。可以在Unix和Win32平台通用
 *
 * 使用方法：
 *    . 创建线程、等待线程
 *        HANDLETHREAD hThread;           //线程句柄
 *        void thread_routine(void* arg); //执行线程工作的函数，参数可以是自己定义的结构
 *
 *        hThread = StartThread((LPTHREAD_START_ROUTINE)thread_routine, (void*)传递给线程的参数);
 *        if (hThread == 0)
 *        {
 *             //创建线程失败
 *        }
 *
 *        //等待线程结束
 *        WAIT_THREAD(hThread);
 *
 *    . 信号量使用
 *        HANDLEMUTEX mutex;    //信号量句柄
 *
 *        //创建信号量，一般在主线程开始时创建
 *        if (!InitMutex(&mutex))
 *        {
 *            //初始化信号量失败
 *        }
 *        ...
 *        LOCK_MUTEX(mutex);    //线程进入临界资源前给信号量加锁
 *        //执行线程的临界操作
 *        UNLOCK_MUTEX(mutex);  //线程结束临界操作后给信号量解锁
 *        ...
 *        DESTROY_MUTEX(mutex); //由创建信号量的线程关闭信号量
 * Xu Lubing
 * Sept. 25, 2003
 */
#ifndef _THREAD_h
#define _THREAD_h

#ifndef WIN32
#include <pthread.h>
typedef void* (*LPTHREAD_START_ROUTINE)(void*);
#else
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
#define HANDLETHREAD pthread_t
#define WAIT_THREAD(h) pthread_join(h, NULL)
#define STOP_THREAD(h) pthread_cancel(h)
#else
#define HANDLETHREAD HANDLE
#define WAIT_THREAD(h) WaitForSingleObject(h, INFINITE)
#define STOP_THREAD(h) TerminateThread(h, THREAD_TERMINATE)
#endif

/**
 * 启动新线程。
 * 参数：
 *   threadFunc: 线程函数，具有原型void threadFunc(void*)
 *   arg:        传递给线程函数的参数
 * 返回：
 *   0: 创建失败
 *   非0: 线程句柄
 */
HANDLETHREAD StartThread(LPTHREAD_START_ROUTINE threadFunc, void* arg);

/* 有关信号量的处理 */
#ifndef WIN32
#define HANDLEMUTEX pthread_mutex_t                    /* 信号量句柄 */
#define LOCK_MUTEX(h) pthread_mutex_lock(&h)           /* 给信号量加锁 */
#define UNLOCK_MUTEX(h) pthread_mutex_unlock(&h)       /* 给信号量解锁 */
#define DESTROY_MUTEX(h) pthread_mutex_destroy(&h)     /* 关闭信号量 */
#else
#define HANDLEMUTEX HANDLE                             /* 信号量句柄 */
#define LOCK_MUTEX(h) WaitForSingleObject(h, INFINITE) /* 给信号量加锁 */
#define UNLOCK_MUTEX(h) ReleaseMutex(h)                /* 给信号量解锁 */
#define DESTROY_MUTEX(h) CloseHandle(h)                /* 关闭信号量 */
#endif

/**
 * 创建并初始化信号量
 * 参数:
 *   mutex: 要初始化的信号量
 * 返回:
 *   0: 初始化失败
 *   非0: 成功
 */
int InitMutex(HANDLEMUTEX* mutex);

#ifdef __cplusplus
}
#endif

#endif

