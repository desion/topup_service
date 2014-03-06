#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include "common.h"

//����ͷ�����ذ����ܳ���(����UDP��TCPЭ��,���ú���ǰ��Ҫ��֤��socket�ɶ�)
//ֻ�Ǽ���ͷ������������ȡ���ݣ�������ʧ�ܣ���Ҫ�ر�socket����
int common_head_check(int sock){
	CommonHead head;
	while(recv(sock, &head, sizeof(CommonHead), MSG_PEEK) != sizeof(CommonHead)){
		if(errno != EINTR){
			return -1;	//��ȡʧ�ܣ��ͻ��˹ر�������
		}
	}
	//��֤��ͷ��ʽ�Ƿ���ȷ
	if(head.magic != HEAD_MAGIC){
		return -2;	//�����¼ܹ��İ�ͷ
	}
	if(head.bodylen >= HEAD_MAX_BODY_LEN){
		return -3;	//bodylen���Ȳ��Ϸ�
	}
	if(head.version != PROTOCOL_VERSION){
		return -4;	//��ͷ�汾����ȷ
	}
	return head.bodylen + sizeof(CommonHead);
}
