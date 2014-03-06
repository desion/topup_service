#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include "common.h"

//检查包头，返回包的总长度(兼容UDP和TCP协议,调用函数前需要保证该socket可读)
//只是检查包头，并不真正读取数据，假如检查失败，需要关闭socket连接
int common_head_check(int sock){
	CommonHead head;
	while(recv(sock, &head, sizeof(CommonHead), MSG_PEEK) != sizeof(CommonHead)){
		if(errno != EINTR){
			return -1;	//读取失败，客户端关闭连接了
		}
	}
	//验证包头格式是否正确
	if(head.magic != HEAD_MAGIC){
		return -2;	//不是新架构的包头
	}
	if(head.bodylen >= HEAD_MAX_BODY_LEN){
		return -3;	//bodylen长度不合法
	}
	if(head.version != PROTOCOL_VERSION){
		return -4;	//包头版本不正确
	}
	return head.bodylen + sizeof(CommonHead);
}
