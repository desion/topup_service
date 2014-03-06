/*************************************************************************
	> File Name: TopupUtils.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 03:14:09 PM CST
 ************************************************************************/

#include "TopupUtils.h"
#include <sys/stat.h>

int get_file_time(char *filename){
	struct stat st = {0};
	if(stat(filename, &st)){
		return -1;
	}
	return (int)st.st_mtime;
}

//so初始化
int so_init(SoBase *so, char *filename, int (reload)(SoBase *so)){
    so->filename = filename;
    so->reload = reload;
    pthread_rwlock_init(&so->lock, NULL);
    return so->reload(so);
}

//so文件名改变了
int so_change(SoBase *so, char *filename){
    so->filename = filename;
    so->reload(so);
    return 0;
}
//检查so是否需要重新加载
int so_check_reload(SoBase *so){
    int ret;
    int time = get_file_time(so->filename);
    if(time <= 0 || time == so->file_time){
       return 0;
    }
    ret = so->reload(so);
    return ret;
}
//加载so文件
int so_load(SoBase *so){
    so->handle = LoadLibrary(so->filename);
    if(so->handle == NULL){
       return -1;
    }
    return 0;
}
//卸载so
int so_free(SoBase *so){
    FreeLibrary(so->handle);
    so->handle = NULL;
    return 0;
}

int so_topup_reload(SoBase *so){
    pthread_rwlock_wrlock(&so->lock);
    if(so->handle != NULL){
        so_free((SoBase*)so);
    }
    if(so_load(so)){
        pthread_rwlock_unlock(&so->lock);
        return -1;
    }
    so->create = (create_t)GetProcAddress(so->handle, "m_create");
    so->destory = (destroy_t)GetProcAddress(so->handle, "m_destroy");
    if (so->create == NULL || so->destory == NULL){
        so_free(so);
        pthread_rwlock_unlock(&so->lock);
        return -1;
    }
    so->file_time = get_file_time(so->filename);
    pthread_rwlock_unlock(&so->lock);
    return 0;
}

//根据手机号码和时间生成order no
uint64_t encode_orderno(string phoneNo){
	uint64_t phone_num, orderno;
	sscanf(phoneNo.c_str(), "%lu", &phone_num);
	uint32_t ttime = time(NULL);
	orderno = ttime - 1392906485;
	orderno = orderno << 36;
	orderno += (phone_num - 10000000000);
	return orderno;	
}

//解析order no从中得到手机号和时间戳
void decode_orderno(const uint64_t orderno, uint64_t *phoneno, uint32_t *ttime){
	*phoneno = (orderno & 0xFFFFFFFFF) + 10000000000;
	*ttime = orderno >> 36;
}

int get_time_now(const char* format, string &time_str){
	time_t t = time(NULL);
	char buf[255];
	int ret = strftime(buf, 255, format, localtime(&t));
	time_str = buf;
	return ret;
}