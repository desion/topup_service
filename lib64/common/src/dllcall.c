/**
 * DLLʹ�á�������Unix��Win32ƽ̨ͨ��
 *
 * ʹ�÷�����dllcall.h
 *
 * Xu Lubing
 * Dec. 23, 2003
 */
#include "dllcall.h"
#include <stdio.h>
#ifndef WIN32
#include <dlfcn.h>
#endif

#ifndef WIN32
/* Unixƽ̨ */

/**
 * ����DLL
 * ����:
 *   lpLibFileName: DLL������
 * ����:
 *   NULL: ���ظ�����DLLʧ��
 *   ��NULL: ����DLL�ɹ�������ֵ���DLL�ľ��
 */
HINSTANCE LoadLibrary(const char* lpLibFileName)
{
	return dlopen(lpLibFileName, RTLD_LAZY);
}

/**
 * ���DLL��ĳ��������ָ��
 * ����:
 *   hModule: ����LoadLibrary���ص�DLL���
 *   lpProcName: ��Ҫ��ȡ�ĺ���������
 * ����:
 *   NULL: ȡ����ָ��ʧ��
 *   ��NULL: �ɹ�������ֵ�������ĺ���ָ��
 */
void* GetProcAddress(HINSTANCE hModule, const char* lpProcName)
{
	return dlsym(hModule, lpProcName);
}

/**
 * ж��DLL
 * ����:
 *   hModule: ����LoadLibrary���ص�DLL���
 * ����:
 *   0: ʧ��
 *   ��0: ж�سɹ�
 */
int FreeLibrary(HINSTANCE hLibModule)
{
	return !dlclose(hLibModule);
}

#endif

/**
 * ȡ��һ�ε���DLL�����Ĵ�����Ϣ
 * ����:
 *   lpErrMsg: ���ڴ�Ŵ�����Ϣ
 * [ע��] ���øú�����õĴ�����Ϣ������ص���FreeErrMsg�ͷ�
 */
void LastDLLError(char** lpErrMsg)
{
	if (lpErrMsg == NULL)
	{
		return;
	}
#ifndef WIN32
	*lpErrMsg = (char*)dlerror();
#else
	FormatMessage( 
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	    NULL,
	    GetLastError(),
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	    (LPTSTR) &lpErrMsg,
	    0,
	    NULL 
	);
#endif
}

/**
 * �ͷŴ�����Ϣ
 * ������
 *   lpErrMsg: ����LastDLLError�õ��Ĵ�����Ϣ
 */
void FreeErrMsg(char* lpErrMsg)
{
#ifdef WIN32
	LocalFree(lpErrMsg);
#endif
}
