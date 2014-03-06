#ifndef TERM_ID
#define TERM_ID 1

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long TERMID_T; 

/*
*	下面的各函数都生成8字节的TERMID

termID的生成规则:
	一共8个字节.由2字节的类别信息和6字节的词序号组成.
	
	类别信息:
		是否字典中的词.			
		是否是BIGRAM.
		词的域信息.
	词序号:
		如果是字典中的词,采用字典中的序号. 类别信息：0x00
		如果不是字典中的词,由Hash函数生成. 类别信息：0x01	
		所有的bigram词,都由hash函数生成.   类别信息：0x02
			
	field对类别信息的影响：
		0xab：	0x00ab	
		
		
生成termID的词汇共有两类词,包括普通词和BIGRAM,
普通词:	
	包括三个类别:	
	1 字典中已有的词,存在id,并且不是禁用词.
		调用GetTermIDWithField,传入词ID和以及词Field信息, 从而获的该词的termID.
	
	2 禁用词,没必要生成termID.
	
	3 字典中中没有,不存在id.
		调用函数GenTermID或GenTermID_U,传入词汇位置和长度以及field信息, 由系统生成一个termID.
	

BIGRAM词汇:
	接受标准:
		如果两个有ID词直接相连，做bi-gram；
                如果两个有ID词之间是禁用词，并且只有一个,也做bi-gram；
	方法:
		调用函数GenTermIDEx或GenTermIDEx_U,传入两个字符串的信息,以及Field信息,由系统生成一个termID.

*	
*/

#define SPECIALPREFIX 0x100000000000000LL
#define BIGRAMPREFIX 0x200000000000000LL

/*
	功能:	获取域的影响值。
	参数:	fieldID:域信息。	
*/

#define GetFieldPower(field) (((TERMID_T)field)<<48)

/*
	功能:	获取具有域信息的termID值。
	参数:	wordID:字典中已有的wordID。
		fieldID:与信息.
*/	

#define GetTermIDWithField(wordID,field) ((((TERMID_T)field)<<48)|wordID)


/*
*s是字符串,len是长度,fiedID是区域标志.
*/
TERMID_T  GenTermID(const char* s,int len,unsigned char fieldID);

/*
*针对bigram词汇
*s是字符串,fiedID是区域标志.
*len1是第一个字符串长度
*pos2是第二个字符串位置
*len2是第二个字符串长度
*/
TERMID_T  GenTermIDEx(const char* s,int len1,int pos2, int len2,unsigned char fieldID);

/*
*针对UNICODE码的函数.
*s是UNICODE串,len是unicode的长度,fiedID是区域标志.
*/
TERMID_T  GenTermID_U(const unsigned short* s,int len,unsigned char fieldID);

/*
*针对UNICODE码的函数.
*s是UNICODE串,fiedID是区域标志.
*len1是第一个字符串长度
*pos2是第二个字符串位置
*len2是第二个字符串长度
*/
TERMID_T  GenTermIDEx_U(const unsigned short* s,int len1,int pos2, int len2,unsigned char fieldID);

#ifdef __cplusplus
	}
#endif

#endif
