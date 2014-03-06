
 
/**
 * @file ul_file.h
 * @date 2007/12/06 22:34:07
 * @version $Revision: 1.2 $ 
 * @brief 文件操作库
 *  
 **/

#ifndef __UL_FILE_H
#define __UL_FILE_H

#include <stdlib.h>
#include <stdio.h>
//#include "dep/dep_file.h"


/**
 * @brief 将path 和 fname 组合成一个完整的路径名，处理出现多个'/'的情况
 *
 * @param [in] path   : 路径
 * @param [in] fname   : 文件名
 * @param [out] fullpath   : 完整的文件名
 * @param [in] size   : fullpath的最大长度,包括'\0'
 * @return  0成功 -1转化失败 
**/
int ul_fullpath(const char *path, const char *fname, char *fullpath, size_t size);

/**
 * @brief 判断文件是否存在
 *
 * @param [in] path : 文件所在目录
 * @param [in] fname : 文件名
 * @return 1存在，0不存在
 */
int ul_fexist(const char *path,const char *fname);

/**
 * @brief 获得文件的长度
 *
 * @param [in] path : 文件所在目录
 * @param [in] fname : 文件名
 * @return -1文件不存在，否则为文件长度
 */
off_t ul_fsize(const char *path, const char *fname);

/**
 * @brief 获得文件的一些内容统计信息,包括文本文件的行数,单词数和字符数
 *
 * @param [in] path : 文件所在目录
 * @param [in] fname : 文件名
 * @param [out] nline : 文件行数
 * @param [out] nword : 单词数
 * @param [out] nchar : 字符数
 * @return 1成功，-1文件不存在
 */
int ul_finfo(const char *path, const char *fname,int *nline,int *nword,int *nchar);

/**
 * @brief 生成一个空文件，如果存在则截断到0，如果不存在则创建
 *
 * @param [in] path : 文件所在目录
 * @param [in] fname : 文件名
 * @return 1成功，-1失败
 */
int ul_nullfile(const char *path, const char *fname);

/**
 * @brief 在同一个目录下拷贝文件
 *
 * @param [in] path : 文件所在目录
 * @param [in] fsrc : 源文件名
 * @param [in] fdest :  目标文件名
 * @return 1成功，-1失败
 */
int ul_cpfile(const char *path, const char *fsrc, const char *fdest);

/**
 * @brief 在同一个目录下将源文件内容添加到目标文件的末尾
 *
 * @param [in] path : 文件所在目录
 * @param [in] fsrc : 源文件名
 * @param [in] fdest : 目标文件名
 * @return 1成功，-1失败
 */
int ul_appfile(const char *path, const char *fsrc, const char *fdest);

/**
 * @brief 在同一个目录下为文件更名
 *
 * @param [in] path :  文件所在目录
 * @param [in] fsrc :  旧文件名
 * @param [in] fdest :  新文件名
 * @return 0成功，-1失败
 */
int ul_mvfile(const char *path, const char *fsrc, const char *fdest);

/**
 * @brief 将一个文件内容全部读入内存中
 *
 * @param [in] path :  文件所在目录
 * @param [in] fname :  文件名
 * @param [in] buf :  读入缓冲区
 * @param [in] size :  缓冲区大小
 * @return -1失败, 否则为读到字节数
 */
ssize_t ul_readfile(const char *path, const char *fname, void *buf, int size);

/**
 * @brief 将内存中的数据写入文件
 *
 * @param [in] path :  文件所在目录
 * @param [in] fname :  文件名
 * @param [in] buf :  读入缓冲区
 * @param [in] size :  缓冲区大小
 * @return -1失败，否则为写入字节数
 */
ssize_t ul_writefile(const char *path, const char *fname, void *buf, int size);
 
/**
 * @brief 交换文件
 *
 * @param [in] path : 文件所在目录
 * @param [in] sname : 文件名s
 * @param [in] dname : 文件名d
 * @return 1成功，-1失败
 */
int ul_exchangedir(const char *path, const char *sname, const char *dname);

int ul_readnum_oneuint(const char *work_path, const char *fname, char *vname, u_int *pvalue);

int ul_readnum_oneint64(const char *work_path, const char *fname, char *vname, long long *pvalue);

int ul_readnum_oneint(const char *work_path, const char *fname, char *vname, int *pvalue);

int ul_readnum_oneuint64(const char *work_path, const char *fname, char *vname, unsigned long long *pvalue);

int ul_writenum_init(const char *work_path, const char *fname);

int ul_writenum_oneint(const char *work_path, const char *fname, const char *vname, int value);

int ul_writenum_oneuint(const char *work_path, const char *fname, const char *vname, u_int value);

int ul_writenum_oneint64(const char *work_path, const char *fname, const char *vname, long long value);

int ul_writenum_oneuint64(const char *work_path, const char *fname, const char *vname, unsigned long long value);

#endif // __UL_FILE_H_
