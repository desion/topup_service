/*
* Copyright (c) 2003, 新浪网研发中心
* All rights reserved.
* 
* 文件名称：searchfile.c
* 摘    要：用于搜索项目的文件操作函数库.
* 作    者：彭中华 2003/9/24
* 版    本：1000
* $Id: searchfile.c,v 1.1 2005/01/24 01:34:11 zhonghua Exp $
*/

#include "searchfile.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>


S_FILE *s_fopen(const char *filename, const char *mode)
{
	return fopen(filename, mode);
}

int s_fclose (S_FILE *fp)
{
	return fclose(fp);
}

int s_fseek(S_FILE *fp, long int offset, int whence)
{
	return fseek(fp, offset, whence);
}

size_t s_fwrite(const void *buf, size_t elsize, size_t nelem, S_FILE *fp)
{
	return fwrite(buf, elsize, nelem, fp);
}

size_t s_fread(void *buf, size_t elsize, size_t nelem, S_FILE *fp)
{
	return fread(buf, elsize, nelem, fp);
}

int s_rename(const char *oldname, const char *newname)
{
	if(remove(newname) == -1)
	{
		;
	}
	return rename(oldname, newname);
}
