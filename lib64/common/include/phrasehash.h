#ifndef PHRASEHASH_H
#define PHRASEHASH_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned int siteHash(unsigned char * key,int length);

unsigned long long textHash(unsigned char * key,int length);

#ifdef __cplusplus
}
#endif

#endif

