#ifndef URLFORMAT_H
#define URLFORMAT_H

/*
 *�ж��Ƿ�Ϊ�ո񡢻��С��Ʊ���ȿ�λ��
 * ������
 * 	ch: �����ַ�����
 * ���أ�
 * 	0: ��
 * 	��0: ��
 * */
int is_space(char ch);

/*  	  
 *�����ַ���β�����з��Ϳհ׷�
 *����  	  
 * lpStr��Դ�ַ�����Ŀ���ַ���
 *�޷���	
 */
void trim_right(char *lpStr);

/*
 *�ַ���תСд
 * ������
 * 	str:	תСд���ַ���
 * ���أ�
 * 	��
 */
void str2lower(char *str);

/**
 *ȡ������
 *����
 * domain�����ص�����
 * url��ԴURL
 *���أ�
 * 1�������� 2 �Ƿ�url
 * 0���ɹ�
 */
int get_full_host_name(const char *url, char *domain);

/* ȡ��·��
 *����
 * url��ԴURL
 * path�����ص�·��
 *���أ�
 * 1�������� 2 �Ƿ�url
 * 0���ɹ�
 */
int get_full_path_name(const char *url, char *path);

// ��ʽ��httpЭ���url���ɹ�����0��ʧ�ܷ���1
int format_http_url(const char *rawUrl, char *destUrl);

// ��ʽ��httpЭ������url���ɹ�����0��ʧ�ܷ���1
// baseUrlӦ��Ϊ�Ѿ���ʽ�����ľ���url
int format_http_relative_url(const char *baseUrl, const char *relativeUrl, char *destUrl);

// ����url��һ��
// url����Ϊ��ʽ�����Ĳ���������url
// multiLevel��ʾ�Ƿ��Ŀ¼Ҳ���й�һ������
void uniform_index_url(char *url, int multiLevel);

/*
 * �ж�url�Ƿ��Ƕ�̬url������0��ʾ��̬��1��ʾ��̬
 */
int is_dynamic_url(const char *url);


#endif

