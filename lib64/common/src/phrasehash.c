#include "phrasehash.h"
#include <ctype.h>

unsigned int siteHash(unsigned char * key,int length)
{
	int i;
	unsigned int  hash = 0;	
		
	for(i=0; i<length; i++)
		hash = hash*129 + (unsigned char)tolower(key[i]) + 987654321L;
		
	return hash;		
}

unsigned long long textHash(unsigned char * key,int length)
{
	unsigned long long hash = 0;	
	char *startPtr = (char *)key;
	char *endPtr =(char *)( key+length-1);
	while(startPtr<=endPtr)
	{
		if(isspace(startPtr[0]))
		{
			startPtr++;			
		}
		else 
			break;
	}
	while(startPtr<=endPtr)
	{
		if(isspace(endPtr[0]))
		{
			endPtr--;			
		}
		else
			break;
	}
	
	while(startPtr<=endPtr)
	{
		hash = hash*129 + (unsigned char)tolower(startPtr[0]) + 987654321L;
		startPtr++;
	}
	return hash;		
}

