/**
 * 1. �����̺߳�����������Unix��Win32ƽ̨ͨ��
 * 2. �ź���(mutex)�Ĵ���������Unix��Win32ƽ̨ͨ��
 *
 * ʹ�÷�����
 *    . �����̡߳��ȴ��߳�
 *        HANDLETHREAD hThread;           //�߳̾��
 *        void thread_routine(void* arg); //ִ���̹߳����ĺ����������������Լ�����Ľṹ
 *
 *        hThread = StartThread((LPTHREAD_START_ROUTINE)thread_routine, (void*)���ݸ��̵߳Ĳ���);
 *        if (hThread == 0)
 *        {
 *             //�����߳�ʧ��
 *        }
 *
 *        //�ȴ��߳̽���
 *        WAIT_THREAD(hThread);
 *
 *    . �ź���ʹ��
 *        HANDLEMUTEX mutex;    //�ź������
 *
 *        //�����ź�����һ�������߳̿�ʼʱ����
 *        if (!InitMutex(&mutex))
 *        {
 *            //��ʼ���ź���ʧ��
 *        }
 *        ...
 *        LOCK_MUTEX(mutex);    //�߳̽����ٽ���Դǰ���ź�������
 *        //ִ���̵߳��ٽ����
 *        UNLOCK_MUTEX(mutex);  //�߳̽����ٽ��������ź�������
 *        ...
 *        DESTROY_MUTEX(mutex); //�ɴ����ź������̹߳ر��ź���
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
 * �������̡߳�
 * ������
 *   threadFunc: �̺߳���������ԭ��void threadFunc(void*)
 *   arg:        ���ݸ��̺߳����Ĳ���
 * ���أ�
 *   0: ����ʧ��
 *   ��0: �߳̾��
 */
HANDLETHREAD StartThread(LPTHREAD_START_ROUTINE threadFunc, void* arg);

/* �й��ź����Ĵ��� */
#ifndef WIN32
#define HANDLEMUTEX pthread_mutex_t                    /* �ź������ */
#define LOCK_MUTEX(h) pthread_mutex_lock(&h)           /* ���ź������� */
#define UNLOCK_MUTEX(h) pthread_mutex_unlock(&h)       /* ���ź������� */
#define DESTROY_MUTEX(h) pthread_mutex_destroy(&h)     /* �ر��ź��� */
#else
#define HANDLEMUTEX HANDLE                             /* �ź������ */
#define LOCK_MUTEX(h) WaitForSingleObject(h, INFINITE) /* ���ź������� */
#define UNLOCK_MUTEX(h) ReleaseMutex(h)                /* ���ź������� */
#define DESTROY_MUTEX(h) CloseHandle(h)                /* �ر��ź��� */
#endif

/**
 * ��������ʼ���ź���
 * ����:
 *   mutex: Ҫ��ʼ�����ź���
 * ����:
 *   0: ��ʼ��ʧ��
 *   ��0: �ɹ�
 */
int InitMutex(HANDLEMUTEX* mutex);

#ifdef __cplusplus
}
#endif

#endif

