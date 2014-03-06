#ifndef TERM_ID
#define TERM_ID 1

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long TERMID_T; 

/*
*	����ĸ�����������8�ֽڵ�TERMID

termID�����ɹ���:
	һ��8���ֽ�.��2�ֽڵ������Ϣ��6�ֽڵĴ�������.
	
	�����Ϣ:
		�Ƿ��ֵ��еĴ�.			
		�Ƿ���BIGRAM.
		�ʵ�����Ϣ.
	�����:
		������ֵ��еĴ�,�����ֵ��е����. �����Ϣ��0x00
		��������ֵ��еĴ�,��Hash��������. �����Ϣ��0x01	
		���е�bigram��,����hash��������.   �����Ϣ��0x02
			
	field�������Ϣ��Ӱ�죺
		0xab��	0x00ab	
		
		
����termID�Ĵʻ㹲�������,������ͨ�ʺ�BIGRAM,
��ͨ��:	
	�����������:	
	1 �ֵ������еĴ�,����id,���Ҳ��ǽ��ô�.
		����GetTermIDWithField,�����ID���Լ���Field��Ϣ, �Ӷ���ĸôʵ�termID.
	
	2 ���ô�,û��Ҫ����termID.
	
	3 �ֵ�����û��,������id.
		���ú���GenTermID��GenTermID_U,����ʻ�λ�úͳ����Լ�field��Ϣ, ��ϵͳ����һ��termID.
	

BIGRAM�ʻ�:
	���ܱ�׼:
		���������ID��ֱ����������bi-gram��
                ���������ID��֮���ǽ��ôʣ�����ֻ��һ��,Ҳ��bi-gram��
	����:
		���ú���GenTermIDEx��GenTermIDEx_U,���������ַ�������Ϣ,�Լ�Field��Ϣ,��ϵͳ����һ��termID.

*	
*/

#define SPECIALPREFIX 0x100000000000000LL
#define BIGRAMPREFIX 0x200000000000000LL

/*
	����:	��ȡ���Ӱ��ֵ��
	����:	fieldID:����Ϣ��	
*/

#define GetFieldPower(field) (((TERMID_T)field)<<48)

/*
	����:	��ȡ��������Ϣ��termIDֵ��
	����:	wordID:�ֵ������е�wordID��
		fieldID:����Ϣ.
*/	

#define GetTermIDWithField(wordID,field) ((((TERMID_T)field)<<48)|wordID)


/*
*s���ַ���,len�ǳ���,fiedID�������־.
*/
TERMID_T  GenTermID(const char* s,int len,unsigned char fieldID);

/*
*���bigram�ʻ�
*s���ַ���,fiedID�������־.
*len1�ǵ�һ���ַ�������
*pos2�ǵڶ����ַ���λ��
*len2�ǵڶ����ַ�������
*/
TERMID_T  GenTermIDEx(const char* s,int len1,int pos2, int len2,unsigned char fieldID);

/*
*���UNICODE��ĺ���.
*s��UNICODE��,len��unicode�ĳ���,fiedID�������־.
*/
TERMID_T  GenTermID_U(const unsigned short* s,int len,unsigned char fieldID);

/*
*���UNICODE��ĺ���.
*s��UNICODE��,fiedID�������־.
*len1�ǵ�һ���ַ�������
*pos2�ǵڶ����ַ���λ��
*len2�ǵڶ����ַ�������
*/
TERMID_T  GenTermIDEx_U(const unsigned short* s,int len1,int pos2, int len2,unsigned char fieldID);

#ifdef __cplusplus
	}
#endif

#endif
