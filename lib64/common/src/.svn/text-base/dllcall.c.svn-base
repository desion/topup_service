/**
 * DLL使用。可以在Unix和Win32平台通用
 *
 * 使用方法见dllcall.h
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
/* Unix平台 */

/**
 * 加载DLL
 * 参数:
 *   lpLibFileName: DLL的名称
 * 返回:
 *   NULL: 加载给定的DLL失败
 *   非NULL: 加载DLL成功，返回值存放DLL的句柄
 */
HINSTANCE LoadLibrary(const char* lpLibFileName)
{
	return dlopen(lpLibFileName, RTLD_LAZY);
}

/**
 * 获得DLL中某个函数的指针
 * 参数:
 *   hModule: 调用LoadLibrary返回的DLL句柄
 *   lpProcName: 需要获取的函数的名称
 * 返回:
 *   NULL: 取函数指针失败
 *   非NULL: 成功，返回值存放所需的函数指针
 */
void* GetProcAddress(HINSTANCE hModule, const char* lpProcName)
{
	return dlsym(hModule, lpProcName);
}

/**
 * 卸载DLL
 * 参数:
 *   hModule: 调用LoadLibrary返回的DLL句柄
 * 返回:
 *   0: 失败
 *   非0: 卸载成功
 */
int FreeLibrary(HINSTANCE hLibModule)
{
	return !dlclose(hLibModule);
}

#endif

/**
 * 取上一次调用DLL函数的错误信息
 * 参数:
 *   lpErrMsg: 用于存放错误信息
 * [注意] 调用该函数获得的错误信息，请务必调用FreeErrMsg释放
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
 * 释放错误信息
 * 参数：
 *   lpErrMsg: 调用LastDLLError得到的错误信息
 */
void FreeErrMsg(char* lpErrMsg)
{
#ifdef WIN32
	LocalFree(lpErrMsg);
#endif
}
