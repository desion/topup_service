/*
* Copyright (c) 2003, 新浪网研发中心
* All rights reserved.
* 
* 文件名称：searchfile.h
* 摘    要：用于搜索项目的文件操作函数库.
* 作    者：彭中华 2003/9/24
* 版    本：1000
* $Id: searchfile.h,v 1.1 2005/01/24 01:34:11 zhonghua Exp $
*/


#ifndef SEARCH_FILE_H
#define SEARCH_FILE_H



#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef S_FILE
#define S_FILE FILE
#endif

S_FILE *s_fopen(const char *filename, const char *mode);
int s_fclose (S_FILE *fp);

int s_fseek(S_FILE *fp, long int offset, int whence);

size_t s_fwrite(const void *buf, size_t elsize, size_t nelem, S_FILE *fp);
size_t s_fread(void *buf, size_t elsize, size_t nelem, S_FILE *fp);

int s_rename(const char *oldname, const char *newname);

#ifdef __cplusplus
}
#endif

#endif
