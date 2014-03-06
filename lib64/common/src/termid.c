#include <stdio.h>
#include <ctype.h>
#include "termid.h"


#define IDMASK 0xffffffffffffLL


TERMID_T  GenTermID(const char* s,int len,unsigned char fieldID)
{
	unsigned char  ch;
	int i;		
	TERMID_T hash = 0;
				
	for(i=0;i<len;i++)
	{
		ch=*(s+i);
		if((ch&0x80)==0)
		{
			ch=tolower(ch);
			hash = hash*129 + ch+ 987654321L;
			
		}	
		else							
		{
			hash = hash*129 + ch+ 987654321L;							
			if(++i<len)
			{
				ch=*(s+i);
				hash = hash*129 + ch+ 987654321L;																	
			}
		}					
	}
			
	return (((TERMID_T)fieldID)<<48)|SPECIALPREFIX|(hash&IDMASK);
}

TERMID_T  GenTermIDEx(const char* s,int len1,int pos2, int len2,unsigned char fieldID)
{		
	if(len1==pos2)
		return (GenTermID(s,len1+len2,fieldID)&(~SPECIALPREFIX))|BIGRAMPREFIX;
	else
	{		
		unsigned char  ch;
		int i,len=pos2+len2;	
		
		TERMID_T hash = 0;
		
		for(i=0;i<len1;i++)
		{
			ch=*(s+i);
			if((ch&0x80)==0)
			{
				ch=tolower(ch);
				hash = hash*129 + ch+ 987654321L;				
			}	
			else							
			{
				hash = hash*129 + ch+ 987654321L;							
				if(++i<len1)
				{
					ch=*(s+i);
					hash = hash*129 + ch+ 987654321L;																	
				}
			}					
		}			
		
		for(i=pos2;i<len;i++)
		{
			ch=*(s+i);
			if((ch&0x80)==0)
			{
				ch=tolower(ch);
				hash = hash*129 + ch+ 987654321L;				
			}	
			else							
			{
				hash = hash*129 + ch+ 987654321L;							
				if(++i<len)
				{
					ch=*(s+i);
					hash = hash*129 + ch+ 987654321L;																	
				}
			}					
		}	
				
		return (((TERMID_T)fieldID)<<48)|BIGRAMPREFIX|(hash&IDMASK);						
	}			
}

TERMID_T  GenTermID_U(const unsigned short* s,int len,unsigned char fieldID)
{
        unsigned short ch;
        int i;	
		TERMID_T hash = 0;                		
                
        for(i=0;i<len;i++)
        {
                ch = s[i];
                ch = tolower(ch);
                hash = (hash*43729 + ch+ 987654321L);                       
        }
        return (((TERMID_T)fieldID)<<48)|SPECIALPREFIX|(hash&IDMASK);      
}

TERMID_T  GenTermIDEx_U(const unsigned short* s,int len1,int pos2, int len2,unsigned char fieldID)
{
	if(len1==pos2)
		return (GenTermID_U(s,len1+len2,fieldID)&(~SPECIALPREFIX))|BIGRAMPREFIX;
	else
	{		
		unsigned short ch;
		int i,len=pos2+len2;	

		TERMID_T hash = 0;   				
		for(i=0;i<len1;i++)
        {
                ch = s[i];
                ch = tolower(ch);
                hash = hash*43729 + ch+ 987654321L;                        
        }		
                	
		for(i=pos2;i<len;i++)
        {
                ch = s[i];
                ch = tolower(ch);
                hash = hash*43729 + ch+ 987654321L;                        
        }
				
		return (((TERMID_T)fieldID)<<48)|BIGRAMPREFIX|(hash&IDMASK);						
	}	
}

