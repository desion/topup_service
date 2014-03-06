#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include "urlformat.h"

#define MAX_URL_LEN             256         /* ×î´óURL³¤¶È */
#define MAX_DOMAIN_LEN          50          /* ×î´óÓòÃû³¤¶È */

struct Param
{
	char str[MAX_URL_LEN];	//Ò»¸ö²ÎÊı
	char name[MAX_URL_LEN];	//²ÎÊıÀïname
	char value[MAX_URL_LEN];	//²ÎÊıÀïvalue
	int resultType;	//0:strÎªname=valueĞÎÊ½£¬²ğ·ÖµÃnameºÍvalue 1:str²»´æÔÚ»ò¿Õ 2:strÄÚÎŞ'='£¬½öµÃname
	int bOrdered;	//ÊÇ·ñÒÑ¾­ÅÅĞò´¦ÀíµÄ±êÖ¾
}TParam;

typedef struct
{
	char name[10];
	char value;
}TTextCode;


//html×ÖÔª´úÂë¶ÔÓ¦×Ö
TTextCode textCodes[] =
{
	{"nbsp", ' '}, {"gt", '>'}, {"lt", '<'}, {"quot", '"'}, {"#149", 46}, {"#8226", 46}
};

int url_encode(char *strSrc, char *strDest);
int order_params(char *params, char *orderedParams);

/*ÅĞ¶ÏÊÇ·ñÎª¿Õ¸ñ¡¢»»ĞĞ¡¢ÖÆ±í·ûµÈ¿ÕÎ»·û
 * ²ÎÊı£º
 * 	ch: ²Ù×÷×Ö·û¶ÔÏó
 * ·µ»Ø£º
 * 	0: ·ñ
 * 	·Ç0: ÊÇ
 * */
int is_space(char ch)
{
	return ((ch == ' ') || (ch == '\n') || (ch == '\r') || (ch == '\t'));
}


/*  	  
 *Ïû³ı×Ö·û´®Î²²¿»»ĞĞ·ûºÍ¿Õ°×·û
 *²ÎÊı  	  
 * lpStr£ºÔ´×Ö·û´®£¬Ä¿µÄ×Ö·û´®
 *ÎŞ·µ»Ø	
 */
void trim_right(char *lpStr)
{
	char *t;
	if (!lpStr)
	{
		return;
	}
	t = lpStr + strlen(lpStr) - 1;
	while (t >= lpStr && is_space(*t))
	{
		*t = 0;
		t--;
	}
	return;
}

/*×Ö·û´®×ªĞ¡Ğ´
 * ²ÎÊı£º
 * 	str:	×ªĞ¡Ğ´ÓÃ×Ö·û´®
 * ·µ»Ø£º
 * 	ÎŞ
 */
void str2lower(char *str)
{
	int iLen;
	iLen = strlen(str);
	while (iLen > 0)
	{
		*(str + iLen - 1) = tolower(*(str + iLen - 1));
		iLen--;
	}
}


/*½«Ò»¸ö"name=value"µÄ×Ö·û´®²ğ·Ö³önameºÍvalue
 *²ÎÊı£º
 *	str: Ô­´®
 * 	name: ²ğ·Ö³öÀ´µÄname´®
 * 	value: ²ğ·Ö³öÀ´µÄvalue´®
 *·µ»Ø£º
 *	0: strÎªname=valueĞÎÊ½£¬²ğ·ÖµÃnameºÍvalue
 *	1: str²»´æÔÚ»ò¿Õ
 *	2: strÄÚÎŞ'='£¬½öµÃname
 */
int str2pair(char *str, char *name, char *value)
{
	char *t;
	if (!str || !(*str))
	{
		*name = 0;
		*value = 0;
		return 1;
	}
	if ( (t = strchr(str, '=')))
	{
		*t = 0;
		strcpy(name, str);
		strcpy(value, t + 1);
		return 0;
	}
	else
	{
		strcpy(name, str);
		*value = 0;
		return 2;
	}
}


/* È¡µÃÂ·¾¶
 *²ÎÊı
 * url£ºÔ´URL
 * path£º·µ»ØµÄÂ·¾¶
 *·µ»Ø£º
 * 1£º²ÎÊı´í 2 ·Ç·¨url
 * 0£º³É¹¦
 */
int get_full_path_name(const char *url, char *path)
{
	const char *t;

	if (!url || !(*url) || !path)
	{
		return 1;
	}

	if (strncmp(url, "http://", 7) == 0)
	{
		url += 7;
	}

	t = strchr(url, '/');
	if (t)
		t++;
	else
		t = url;
	
	strncpy(path, t, MAX_URL_LEN);
	
	return 0;
}

/* È¡µÃÓòÃû
 *²ÎÊı
 * url£ºÔ´URL
 * domain£º·µ»ØµÄÓòÃû
 *·µ»Ø£º
 * 1£º²ÎÊı´í 2 ·Ç·¨url
 * 0£º³É¹¦
 */
int get_full_host_name(const char *url, char *domain)
{
	const char *t;	//the first '/' after "http://" in url
	int n;

	if (!url || !(*url) || !domain)
	{
		return 2;
	}

	if (strncasecmp(url, "http://", 7) == 0)
	{
		url += 7;
	}

	t = strchr(url, '/');
	n = t ? t - url : strlen(url);
	
	if (n >= MAX_DOMAIN_LEN)
	{
		return 2;
	}

	memcpy(domain, url, n);
	domain[n] = 0;
	return 0;
}


/*Ä¨È¥URL¿ªÍ·ºÍÄ©Î²µÄ¿Õ¸ñ
 * ²ÎÊı£º
 * 	*start:	URLÆğÊ¼Î»ÖÃ
 * 	*end:	URLÄ©Î²Î»ÖÃ
 * ·µ»Ø£º
 * 	ÎŞ
 * */
void url_trim(char **start, char **end)
{
	while (*start <= *end && is_space(**start))
	{
		(*start)++;
	}
	while ((*end >= *start) && (is_space(**end)))
	{
		(*end)--;
	}
	(*end)++;
	**end = 0;
}


/*ÅĞ¶Ï×Ö·û´®ÊÇ·ñÎª¹ú¼Ò´úÂë
 * ×¢£º
 *	¹ú¼Ò´úÂë£º
 *		ac,ad,ae,af,ag,ai,al,am,an,ao,aq,ar,as,at,au,aw,az,
 *		ba,bb,bd,be,bf,bg,bh,bi,bj,bm,bn,bo,br,bs,bt,bv,bw,by,bz,
 *		ca,cc,cd,cf,cg,ch,ci,ck,cl,cm,cn,co,cr,cu,cv,cx,cy,cz,
 *		de,dj,dk,dm,do,dz,
 *		ec,ee,eg,eh,er,es,et,
 *		fi,fj,fk,fm,fo,fr,
 *		ga,gd,ge,gf,gg,gh,gi,gl,gm,gn,gp,gq,gr,gs,gt,gu,gw,gy,
 *		hk,hm,hn,hr,ht,hu,
 *		id,ie,il,im,in,io,iq,ir,is,it,
 *		je,jm,jo,jp,
 *		ke,kg,kh,ki,km,kn,kp,kr,kw,ky,kz,
 *		la,lb,lc,li,lk,lr,ls,lt,lu,lv,ly,
 *		ma,mc,md,mg,mh,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,
 *		na,nc,ne,nf,ng,ni,nl,no,np,nr,nu,nz,
 *		om,
 *		pa,pe,pf,pg,ph,pk,pl,pm,pn,pr,ps,pt,pw,py,
 *		qa,
 *		re,ro,ru,rw,
 *		sa,sb,sc,sd,se,sg,sh,si,sj,sk,sl,sm,sn,so,sr,st,sv,sy,sz,
 *		tc,td,tf,tg,th,tj,tk,tm,tn,to,tp,tr,tt,tv,tw,tz,
 *		ua,ug,uk,um,us,uy,uz,
 *		va,vc,ve,vg,vi,vn,vu,
 *		wf,ws,
 *		ye,yt,yu,
 *		za,zm,zw
 * ²ÎÊı£º
 * 	str:	±»ÅĞ¶ÏµÄ×Ö·û´®
 * ·µ»Ø£º
 * 	0:	²»ÊÇ¹ú¼Ò´úÂë
 * 	1:	ÊÇ
 */
int is_country_code(char *str)
{
	int iLen;	//str³¤¶È
	char l;	//strµÄ×ó×Ö·û
	char r;	//strµÄÓÒ×Ö·û
	iLen = strlen(str);
	//½öÏŞ2¸ö×Ö·ûµÄ×Ö·û´®
	if (iLen != 2)
	{
		return 0;
	}
	l = *str;	//Ê××Ö·û
	r = *(str + 1);	//ÁíÒ»×Ö·û
	if (('a' == l && strchr("cdefgilmnoqrstuwz", r)) 
		|| ('b' == l && strchr("abdefghijmnorstvwyz", r)) 
		|| ('c' == l && strchr("acdfghiklmnoruvxyz", r)) 
		|| ('d' == l && strchr("ejkmoz", r)) 
		|| ('e' == l && strchr("ceghrst", r)) 
		|| ('f' == l && strchr("jkmor", r)) 
		|| ('g' == l && strchr("adefghilmnpqrstuwy", r)) 
		|| ('h' == l && strchr("kmnrtu", r)) 
		|| ('i' == l && strchr("delmnoqrst", r)) 
		|| ('j' == l && strchr("emop", r)) 
		|| ('k' == l && strchr("eghimnprwyz", r)) 
		|| ('l' == l && strchr("abcikrstuvy", r)) 
		|| ('m' == l && strchr("acdghklmnopqrstuvwxyz", r)) 
		|| ('n' == l && strchr("acefgilopruz", r)) 
		|| ('o' == l && 'm' == r) 
		|| ('p' == l && strchr("aefghklmnrstwy", r)) 
		|| ('q' == l && 'a' == r) 
		|| ('r' == l && strchr("eouw", r)) 
		|| ('s' == l  && strchr("abcdeghijklmnortvyz", r)) 
		|| ('t' == l && strchr("cdfghjkmnoprtvwz", r)) 
		|| ('u' == l && strchr("agkmsyz", r)) 
		|| ('v' == l && strchr("aceginu", r)) 
		|| ('w' == l && strchr("fs", r)) 
		|| ('y' == l && strchr("etu", r)) 
		|| ('z' == l && strchr("amw", r)))
	{
		return 1;
	}
	return 0;
}


/*ÅĞ¶Ï×Ö·û´®ÊÇ·ñÎª¶¥¼¶ÓòÃû
 * ×¢£º
 * 	¶¥¼¶ÓòÃû£ºcom,edu,gov,cc,int,mil,net,org,biz,info,tv,pro,name,museum,coop,aero
 * ²ÎÊı£º
 * 	str:	±»ÅĞ¶ÏµÄ×Ö·û´®
 * ·µ»Ø£º
 * 	0:	²»ÊÇ¶¥¼¶ÓòÃû»ò¹ú¼Ò´úÂë
 * 	1:	ÊÇ
 */
int is_top_domain(char *str)
{
	int iLen;	//str³¤¶È
	int i;	//Ñ­»·±äÁ¿
	char *topDomain[] =
	{
		"com", "net", "org", "edu", "gov", "cc", "tv", "int", "mil", "biz",
		"info", "pro", "name", "museum", "coop", "aero"
	};
	int sNum;	//topDomainÔªËØ¸öÊı
	//iLen: str³¤¶È
	iLen = strlen(str);
	//½öÏŞ3-6¸ö×Ö·ûµÄ×Ö·û´®
	if (iLen<2 || iLen>6)
	{
		return 0;
	}
	sNum = sizeof(topDomain) / sizeof(char *);
	for (i = 0; i < sNum; i++)
	{
		if (0 == strcmp(str, topDomain[i]))
		{
			return 1;
		}
	}
	return 0;
}


/*ÅĞ¶ÏÊÇ·ñÊÇEmail
 * ²ÎÊı£º
 *	href:	hrefÖµ
 *	iLen:	hrefÖµ×Ö·û´®³¤¶È
 * ·µ»Ø£º
 *	0:	·ÇEmail
 *	1:	ÊÇEmail
 * */
int is_email(char *href, int iLen)
{
	char *t1, *t2;	// Ö¸Õë£¬Ö¸Ïò×îºóÒ»¸ö'.'£¬ÒÔ¼°Ö®Ç°Ò»¸ö'.'µÄÎ»ÖÃ
	if (iLen <= 0)
	{
		return 0;
	}
	//²»´ø'@'µÄ²»ÊÇEmail
	if (!strchr(href, '@'))
	{
		return 0;
	}
	//ÕÒ×îºóÒ»¸ö'.'µÄÎ»ÖÃ
	if (!(t1 = strrchr(href, '.')))
	{
		return 0;
	}
	//ÅĞ¶Ï'.'ºó×Ö·û´®ÊÇ·ñÎª¶¥¼¶ÓòÃû
	if (is_top_domain(t1 + 1))
	{
		return 1;
	}
	//ÅĞ¶Ï'.'ºó×Ö·û´®ÊÇ·ñÎª¹ú¼Ò´úÂë
	if (is_country_code(t1 + 1))
	{
		return 1;
	}
	*t1 = 0;
	//ÔÙÇ°ÕÒÒ»¸ö'.'µÄÎ»ÖÃ
	if (!(t2 = strrchr(href, '.')))
	{
		*t1 = '.';
		return 0;
	}
	//ÅĞ¶Ïµ¹ÊıµÚ¶ş¶Î×Ö·û´®ÊÇ·ñÎª¶¥¼¶ÓòÃû
	if (is_top_domain(t2 + 1))
	{
		*t1 = '.';
		return 1;
	}
	return 0;
}

/*ÅĞ¶ÏÊÇ·ñÒÔjavascript:, mailto:, about:,#, »òÆäËüĞ­Òé(Èç¹ûÓĞ)¿ªÍ·, »òÎÄ¼şÃûº¬@×Ö·û£¬²»ÊÇÔòºÏ·¨
 * ²ÎÊı£º
 * 	href:	hrefÖµ
 * ·µ»Ø£º
 *	0:	·Ç·¨
 *	1:	ºÏ·¨
 * */
int is_valid_link(char *href)
{
	char tmp[MAX_URL_LEN];	//backup of href
	int iLen;	//length of href
	char *t;	//for seeking in tmp
	if (!href || !(*href))
	{
		return 0;
	}
	iLen = strlen(href);
	if (iLen > MAX_URL_LEN - 1 || iLen <= 0)
	{
		return 0;
	}
	strcpy(tmp, href);
	str2lower(tmp);
	if (*tmp == '#')	// ÒÔ '#' ¿ª
	{
		return 0;
	}
	if (iLen >= 11 && *tmp == 'j' && *(tmp + 10) == ':' 
	    && 0 == strncmp(tmp + 1, "avascript", 9))	// begins with "javascript:"
	{
		return 0;
	}
	if (iLen >= 7 && *tmp == 'm' && *(tmp + 6) == ':' 
	    && 0 == strncmp(tmp + 1, "ailto", 5))	//begins with "mailto:"
	{
		return 0;
	}
	if (iLen >= 11 && *tmp == 'a' && *(tmp + 5) == ':'
	    && 0 == strncmp(tmp + 1, "bout", 4))	//begins with "about:"
	{
		return 0;
	}
	// º¬ "://", µ«²» "http:"¿ªÍ·, ¼´ÆäËüĞ­ÒéÍ·
	if ( (t = strchr(tmp, ':')))
	{
		if ((*(t + 1) == '\\' || *(t + 1) == '/')
		 && (*(t + 2) == '\\' || *(t + 2) == '/')
		 && 0 != strncmp(tmp, "http:", 5))
		{
			return 0;
		}
	}
	if (is_email(tmp, iLen))
	{
		return 0;
	}
	return 1;
}

/*½«×Ö·û´®ÖĞ'\'×ª'/', È¥"./", È¥Ä©Î²¿ÕÎ»·û, ½öÓòÃûµÄ¼Ó'/'
 * ²ÎÊı£º
 * 	str:		Ô­×Ö·û´®
 *	formated:	×ª»»ºó×Ö·û´®
 *	maxLen:		formated±»ÔÊĞí×î´ó³¤¶È
 * ·µ»Ø£º
 * 	0:	Ê§°Ü
 * 	1:	³É¹¦
 * */
int pre_format(char *str, char *formated, int maxLen)
{
	char *pf;	// Ö¸Õë£¬Ä¿±ê×Ö·û´®formatedµÄ±¸·İ
	int iLen;	// Ä¿±ê×Ö·û´®strµÄ³¤¶È
	int bAbsolute;	// ÒÔ"http://"¿ªÍ·µÄ¾ø¶ÔµØÖ·±êÖ¾
	int bInDomain;	// ÓòÃûÄÚ±êÖ¾
	char *end;	// Ö¸Õë£¬Ö¸ÏòstrÎ²
	char *begin;	// Ö¸Ïòstr¿ªÍ·
	if (!str || !(*str) || !formated || !maxLen)
	{
		return 0;
	}
	//´ø'*'µÄURLÓĞ´í
	if (strchr(str, '*'))
	{
		return 0;
	}
	//³õÊ¼»¯
	memset(formated, 0, MAX_URL_LEN);
	pf = formated;
	bAbsolute = 0;
	bInDomain = 0;
	end = str + strlen(str) - 1;
	//Ä¨È¥URL¿ªÍ·ºÍÄ©Î²µÄ¿Õ¸ñ
	url_trim(&str, &end);
	begin = str;
	//strÒÔ"http:"¿ªÍ·£¨²»·Ö´óĞ¡Ğ´£©
	if (0 == strncasecmp(str, "http:", 5))
	{
		bInDomain = 1;	//ÖÃÓòÃûÄÚ±êÖ¾
		strcpy(pf, "http:");	//ÁîÄ¿±ê´®ÒÔ"http:"¿ªÍ·
		pf += 5;
		str += 5;
		if ((*str == '/' || *str == '\\')
		 && (*(str + 1) == '/' || *(str + 1) == '\\'))
		{
			//×ª»»Ô­´®"http:"ºóµÄ'/'»ò'\'Í³Ò»Îª'/'
			*pf = '/'; pf++;
			*pf = '/'; pf++;
			str += 2;
			bAbsolute = 1;	//ÖÃ¾ø¶ÔURL±êÖ¾
		}
		else	//"http:"ºó²»ÊÇĞ±¸Ü£¬´í
		{
			return 0;
		}
	}
	else
	{
		bInDomain = 0;
	}
	//iLen: Ê£ÓàÔ­×Ö·û´®³¤
	iLen = strlen(str);
	while (iLen > 0)
	{
		//'\'×ª»»Îª'/'
		if (*str == '\\' || *str == '/')
		{
			bInDomain = 0;
			/* 2006-10-30È¡Ïû´Ë×ª»»£¬ÒòÎªformat_url_levelÖĞÓĞ¸üÍêÉÆµÄ´¦Àí
			//¶à¸ö'/'»»ÎªÒ»¸ö'/'
			if (pf == formated || (pf > formated && *(pf - 1) != '/'))
			{
			*/
				*pf = '/';
				pf++;
				//³¬³¤ÅĞ¶Ï
				if (pf - formated >= maxLen)
				{
					return 0;
				}
//			}
			//ÏÂÒ»×Ö·û
			str++; iLen--;
		}
		else if (*str == '.'
			  && (str > begin && *(str - 1) == '/')
			  && *(str + 1) == '.'
			  && *(str + 2) == 0)	// ÈôÓöÔ­´®µÄ"/.."½áÎ
		{
			//³¬³¤ÅĞ¶Ï
			if (pf + 4 - formated >= maxLen)
			{
				return 0;
			}
			//¼Ó'/'£¬¼´£º"/.." ±ä "/../"
			*pf = '.';
			*(pf + 1) = '.';
			*(pf + 2) = '/';
			pf += 3;
			break;
		}
		else if (*str == '.' && (str > begin && *(str - 1) == '/') && *(str + 1) == 0)	// ÈôÓöÔ­´®µÄ"/."½áÎ
		{
			break;
		}	//ÂÔ¹ı¡£×Ö·û´®´¦Àí½áÊø£¬ÍË³öÑ­»·
		else if (*str == '.' && *(str + 1) == '/' && (str > begin && *(str - 1) == '/') )	// /./
		{
			//ÔÚÔ­´®ÖĞÖ±½ÓÂÔ¹ı
			str += 2; iLen -= 2;
		}
		else if ('\t' == *str || '\n' == *str || '\r' == *str)	//TAB·û¡¢»»ĞĞ¡¢»Ø³µ·û
		{
			 	//ÔÚÔ­´®ÖĞÖ±½ÓÂÔ¹ı
			 	str++; iLen--;
		}
		else if (' ' == *str && bAbsolute && bInDomain)	//¾ø¶ÔµØÖ·ÓòÃûÄÚ³öÏÖ¿Õ¸ñ£¬´í
		{
			 	return 0;
		}
		else if ('%' == *str && bAbsolute && bInDomain)	//¾ø¶ÔµØÖ·ÓòÃûÄÚ³öÏÖ'%'£¬´í
		{
			 	return 0;
		}
		else	//ÆäËü×Ö·û
		{
			//Ä¿±ê×Ö·û´®´ÓÔ­×Ö·û´®ÕÕ³­´Ë×Ö·û
			*pf = *str;
			pf++;
			//³¬³¤ÅĞ¶Ï
			if (pf - formated >= maxLen)
			{
				return 0;
			}
			str++; iLen--;
		}
	}
	*pf = 0;
	//Ä¿±ê×Ö·û´®ÊÇ¾ø¶ÔURL£¬ÇÒ"http://"ºó²»ÊÇ'/'(±ÜÃâ"http:///"Çé¿ö)£¬ÇÒÃ»ÓĞÆäËü'/'£¬ËµÃ÷ÊÇ½öÓòÃû
	if ((bAbsolute)
	 && (*(formated + 7) != '/')
	 && (!(strchr(formated + 8, '/'))))
	{
		//Ä¿±ê×Ö·û´®Ä©Î²¼Ó'/'
		*pf = '/';
		pf++;
		*pf = 0;
	}
	//Ä¿±ê×Ö·û´®ÊÇ½öÎª"."£¬ËµÃ÷Á´Ïòµ±Ç°ÍøÒ³
	if ('.' == *formated && 0 == *(formated + 1))
	{
		*formated = 0;
	}	//ºöÂÔ´ËURL
	return 1;
}


/*¶à'/'×ªÒ»¸ö'/'£¬"/../"×ªÉÏÉıÒ»¼¶£¬´¦Àí'#'
 * ²ÎÊı£º
 *	URL:	Ô­µÈ¸ñÊ½»¯¾ø¶ÔµØÖ·URL×Ö·û´®
 *	fURL:	¸ñÊ½»¯ºó×Ö·û´®
 *	maxLen:	×î´ó³¤¶È
 * ·µ»Ø£º
 * 	0:	Ê§°Ü
 * 	1:	³É¹¦
 * */
int format_url_level(char *URL, char *fURL, int maxLen)
{
	char *astart;	// Ö¸Õë£¬Ö¸ÏòÄ¿±ê×Ö·û´®fURLÀï"http://"ºóÃæÒ»¸öÎ»ÖÃ
	char *aend;	// Ö¸Õë£¬Ö¸ÏòÄ¿±ê×Ö·û´®fURLÀï×îºóÒ»¸ö×Ö·ûºóÃæµÄÒ»¸öÎ»ÖÃ
	char *t;	// Ö¸Õë£¬ÔÚfURLÀï×Ö·û´®²éÕÒÓÃ
	int iLen;	// URLÀï"http://"ºóÃæ×Ö·û´®µÄ³¤¶È
	int bQues;	// ²ÎÊı'?'´æÔÚµÄ±êÖ¾Î»¡£0:²»´æÔÚ, 1:´æÔÚ
	if (!URL || !(*URL))
	{
		return 0;
	}
	if (strncasecmp(URL, "http://", 7))	// ¹ıÂË²»ÊÇÒÔ"http://"´òÍ·µÄURL
	{
		return 0;
	}
	strcpy(fURL, "http://");	// Ä¿±ê×Ö·û´®ÒÔ"http://"¿ªÍ·
	astart = aend = fURL + 7;	// Ä¿±êURL×Ö·û´®ÖĞÒÆÖÁ×îºó£¨"http://"ºó£©
	URL += 7;	// Ô­×Ö·û´®ÒÆÖÁ"http://"ºó
	iLen = strlen(URL);	// URLÀï"http://"ºóÃæ×Ö·û´®µÄ³¤¶È
	bQues = 0;
	while (iLen > 0 && aend - fURL < maxLen - 1)	//Ô­´®»¹ÓĞÓàÏÂ×Ö·û£¬²¢ÇÒÄ¿±ê´®»¹ÓĞ¿ÕÓàÎ»Ö
	{
		if ((iLen >= 3
		  && *URL == '.'
		  && *(URL + 1) == '.'
		  && *(URL + 2) == '/')	// "../"
		 || (iLen >= 3 && *URL == '.' && *(URL + 1) == '.' && *(URL + 2) == 0))	// »ò×Ö·û´®Óöµ½ÒÔ".."½áÎ
		{
			//Ä¿±ê×Ö·û´®Î²·Ç'/'£º".."×÷ÎªÄ¿Â¼ÃûµÄÒ»²¿·ÖĞø½Óµ½Ä¿±ê×Ö·û´®Ä©Î²
			if (*(aend - 1) != '/')	//ÈôÄ¿±ê×Ö·û´®×îºóÒ»¸ö×Ö·û²»ÊÇ'/'
			{
				//Ä¿±ê×Ö·û´®ºóĞøÉÏ"../"
				*aend = '.'; aend++;
				*aend = '.'; aend++;
				*aend = '/'; aend++;
				//Ô­×Ö·û´®ºóÒÆ3Î»£»Ô­´®Ê£Óà³¤¶ÈÉÙ3
				URL += 3; iLen -= 3; 
				continue;	//¼ÌĞøÔ­´®ÀïÏÂÒ»×Ö·û
			}
			//µ±Ä¿±ê×Ö·û´®ÒÔ'/'½áÎ²£¬ÔòÁî¸ÃÎ»Îª½áÊøÎ»
			aend--;
			*aend = 0;
			if (!(t = strrchr(astart, '/')))	//ÈôÄ¿±ê×Ö·û´®"http://"ºóÃ»ÓĞ'/'
			{
				aend = astart;
			}	//Ä©Î»Ö¸ÕëÖ¸Ïò"http://"ºó
			else	//·ñÔò
			{
				//Ä¿±ê´®È¥µô×îºóÒ»¼¶
				aend = t + 1;	//Ä©Î»Ö¸ÕëÖ¸Ïò×îÄ©Ò»¸ö'/'ºó
				//Ô­×Ö·û´®ºóÒÆ3Î»£»Ô­´®Ê£Óà³¤¶ÈÉÙ3
				URL += 3; iLen -= 3;
			}
		}
		else if (iLen >= 2 && *URL == '.' && *(URL + 1) == '/')	// Ô­´®Àïµ±Ç°Îª"./"
		{
			if (*(aend - 1) != '/')	//ÈôÄ¿±ê×Ö·û´®×îºóÒ»¸ö×Ö·û²»ÊÇ'/'
			{
				//Ä¿±ê×Ö·û´®ºóĞøÉÏ"./"
				*aend = '.'; aend++;
				*aend = '/'; aend++;
				//Ô­×Ö·û´®ºóÒÆ2Î»£»Ô­´®Ê£Óà³¤¶ÈÉÙ2
				URL += 2; iLen -= 2;
				continue;	//¼ÌĞøÔ­´®ÀïÏÂÒ»×Ö·û
			}
			// ÈôÄ¿±ê×Ö·û´®×îºóÒ»¸ö×Ö·ûÊÇ'/'
			//Ô­×Ö·û´®ºóÒÆ2Î»£»Ô­´®Ê£Óà³¤¶ÈÉÙ2
			URL += 2; iLen -= 2;
		}
		else if (iLen >= 1 && *URL == '/')	// Ô­´®Àïµ±Ç°Îª'/'
		{
			if (bQues == 0 && *(aend - 1) == '/')	//ÉĞÎ´·¢ÏÖURL²ÎÊı²¿·Ö£¬²¢ÇÒÄ¿±ê´®ÒÔ'/'½áÎ
			{
				//Ô­´®ºóÒÆÒ»Î»
				URL++; iLen--;
			}
			else
			{
				//Ä¿±ê´®ĞøÒÔ'/'
				*aend = '/';
				aend++;
				//Ô­´®ºóÒÆÒ»Î»
				URL++;	iLen--;
			}
		}
		else	// ÆäËü×Ö·û
		{
			// ÈôÎª'?'£¬bQues±êÖ¾Î»ÖÃ1
			if (*URL == '?')
			{
				bQues = 1;
			}
			// ÈôÓöÃª×Ö·û£¬ÇÒ²»ÔÚ²ÎÊı²¿·Ö
			// Ãª×Ö·û²»¹ÜÊÇ·ñÔÚ²ÎÊı²¿·Ö¶¼¹ıÂËµô
			else if (*URL == '#' /* && bQues == 0 */)
			{
				// ÔİÊ±ÈÓµô#ÃªURL
				return 0;
				break;
			}	//Ä¿±ê´®µ½´ËÎªÖ¹
			*aend = *URL;	//Ô­´®µ±Ç°×Ö·ûĞø¼Óµ½Ä¿±ê´®ºó
			//Ä¿±ê´®Ö¸ÕëºóÒÆÒ»Î»
			aend++;
			//Ô­´®ºóÒÆÒ»Î»
			URL++; iLen--;
		}
	}
	*aend = 0;
	return 1;
}

/*´¦ÀíÓòÃûºóµÄ'.'£»ÒÔ¼°¼ì²éÓòÃûÊÇ·ñÓÉÓ¢ÎÄ×ÖÄ¸¡¢Êı×Ö¡¢'-'¡¢'.'¡¢':'×é³É£¬²»ÊÇÔò·µ»ØÊ§°Ü
 * ×¢£ºÓòÃû´¦Àí¾ÙÀı£º	½«"http://www.a.com./" => "http://www.a.com/", ²¢·µ»Ø1
 * 			½«"http://www.a.com../"¶ªµô(·µ»Ø0)
 * 			½«"http://www.a...com/"¶ªµô(·µ»Ø0)
 * ´¦ÀíÓòÃûÖĞµÄ¶Ë¿Ú£¬ÅĞ¶ÏÊÇ·ñÎŞĞ§¶Ë¿Ú²¢ÇÒ¹éÒ»»¯80¶Ë¿Ú
 * ²ÎÊı£º
 * 	URL:	´ı´¦ÀíµÄURL¡£ ·µ»Ø1Ê±Îª´¦ÀíºóµÄURL
 * ·µ»Ø£º
 *	0:	Ê§°Ü
 *	1:	³É¹¦
 */
int deal_domain(char *URL)
{
	char *p;	// Ö¸Õë£¬Ö¸ÏòURLÀï"http://"ºóµÚÒ»¸ö'/'´¦
	int preSpecial;
	long portNum;
	char *portEnd;
	
	if (!URL || !*URL)
	{
		return 0;
	}
	preSpecial = 0;
	URL += 7;	//Ö¸ÕëÒÆÖÁ"http://"ºó
	while (*URL && '/' != *URL)	//URL½áÊøÇ°£¬»ò'/'Ç°
	{
		if (*URL == '.' || *URL == ':')
		{
			// ²»ÄÜÁ¬ĞøÁ½¸öÌØÊâ×Ö·û
			if (preSpecial)
				return 0;
			preSpecial = 1;
		}
		else if ((*URL >= 'A' && *URL <= 'Z') || (*URL >= 'a' && *URL <= 'z') || (*URL >= '0' && *URL <= '9') 
				 || *URL == '_' || *URL == '-')
		{
			preSpecial = 0;
		}
		else
		{
			// ·Ç·¨×Ö·û
			//ÈôÓöÓ¢ÎÄ×ÖÄ¸¡¢Êı×Ö¡¢'_'¡¢'-'¡¢'.'¡¢':'ÒÔÍâÆäËü×Ö·û
			return 0;
		}
		if (*URL == ':')
		{
			// ÅĞ¶Ï¶Ë¿ÚºÅµÄºÏ·¨ĞÔ
			// ´Ë´¦µÄÅĞ¶ÏÒÑ¾­È·±£ÁË£¬portÖ»³öÏÖÔÚdomainµÄ×îºó²¿·Ö
			portNum = strtol(URL + 1, &portEnd, 10);
			if (portNum < 80 || portNum > 65535 || (*portEnd != '/' && *portEnd != 0))
			{
				return 0;
			}
			if (portNum == 80)
			{
				p = portEnd;
				while (*p)
				{
					*URL = *p;
					URL++;
					p++;
				}
				*URL = 0;
				break;
			}
		}
		if ('.' == *URL)
		{
/*
 * Ç°ÃæÅĞ¶Ï¹ıÁË£¬´Ë´¦Ã»±ØÒª
 			//ÈôÁ¬ĞøÁ½¸ö'.'£¨¼´".."£©
			if ('.' == *(URL + 1))
			{
				return 0;
			}
*/			//Èô"./"
			if ('/' == *(URL + 1))
			{
				p = URL + 1;	//pÖ¸ÏòURLÀï"http://"ºóµÚÒ»¸ö'/'´¦
				//½«Ô­´®"http://"ºóµÄÄÚÈİĞø½Óµ½Ä¿±ê×Ö·û´®
				while (*p)
				{
					*URL = *p;
					URL++; p++;
				}
				*URL = 0;
				break;
			}
		}
		URL++;	//ÓòÃûÀïÏÂÒ»¸ö×Ö·û
	}
	return 1;
}

/*¾ø¶ÔµØÖ·URL¹æ·¶»¯
 *²ÎÊı£º
 *	URL: Ô­URL
 *	fURL: Ä¿±êURL£¬¹æ·¶»¯ºóµÄURL
 *	maxLen: ¹æ·¶»¯URL×î´ó³¤¶È
 *·µ»Ø£º
 *	0: Ê§°Ü
 * 	1: ³É¹¦
 */
int format_absolute_url(char *URL, char *fURL, int maxLen)
{
	int iLen;	//formatUrlLevelºóURL³¤¶È
	int ifLen;	//¹æ·¶URL³¤¶È
	char *t;	//ÁÙÊ±±äÁ¿, ×Ö·û²éÕÒÓÃ
	char *params;	//¹æ·¶ÓÃ²ÎÊı´®
	char temp[MAX_URL_LEN];	//temp URL string
	char *tmpURL;	//backup pointer of temp URL
	if (!URL || !(*URL))
	{
		return 0;
	}
	*fURL = 0; ifLen = 0;
	if (!format_url_level(URL, temp, MAX_URL_LEN))
	{
		return 0;
	}
	tmpURL = temp;
	iLen = strlen(tmpURL);
	if (iLen < 10 
		|| ((*tmpURL == 'h' || *tmpURL == 'H') && strncasecmp(tmpURL + 1, "ttp://", 6)))
	{
		// ³¤¶È²»¹» »ò ²»ÒÔhttp://¿ªÍ·
		return 0;
	}
	//ÁîÄ¿±ê×Ö·û´®ÒÔ"http://"¿ªÍ·
	strcpy(fURL, "http://"); ifLen += 7;
	//iLen: Ô­´®Ê£Óà×Ö·ûÊı
	tmpURL += 7; iLen -= 7;
	if ( (t = strchr(tmpURL, '?')))	// Ô­´®º¬²ÎÊı±êÖ¾·û'?'
	{
		*t = 0;	// '?' -> 0x0
		//ÔÚ'?'Ç°²éÕÒ'/'
		if (strchr(tmpURL, '/'))	//ÔÚ'?'Ç°´æÔÚ'/'
		{
			strcat(fURL, tmpURL);	//½«²ÎÊıÇ°URL²¿·ÖĞø½Óµ½Ä¿±ê×Ö·û´®
			strcat(fURL, "?");	//½«'?'Ğø½Óµ½Ä¿±ê×Ö·û´®
			ifLen += strlen(tmpURL) + 1;	//¸üĞÂÄ¿±ê×Ö·û´®µÄ×Ö·û¸öÊı
		}
		else
		{
			//²ÎÊıÇ°²»º¬'/'£¬ËµÃ÷½öÎªÓòÃû¼´½Ó×Å'?'
			//ÔÚÓòÃûºÍ'?'¼ä²¹¼Ó'/'Ğø¼Óµ½Ä¿±ê×Ö·û´®
			strcat(fURL, tmpURL);	
			strcat(fURL, "/");
			ifLen += strlen(tmpURL) + 1;
			//¹ıÂË²ÎÊıÇ°³¬³¤µÄURL
			if (ifLen + 1 >= maxLen)
			{
				return 0;
			}
			strcat(fURL, "?");
		}
		//Ô­´®Ö¸ÕëÒÆµ½'?'ºóµÄ²ÎÊı¿ªÊ¼´¦
		tmpURL = t + 1;
		///printf("tmpURL:%s\n", tmpURL);
		params = (char *) malloc(maxLen);
		*params = 0;
		//paramsÎª½«²ÎÊıÅÅĞòºóµÄ×Ö·û´®
		if (order_params(tmpURL, params) != 0)
		{
			free(params);
			return 0;
		}
		//¹ıÂËURL¼°²ÎÊı³¬³¤µÄURL
		if (ifLen + strlen(params) >= maxLen)
		{
			free(params);
			return 0;
		}
		//½«²ÎÊıÆ´½Óµ½Ä¿±ê×Ö·û´®
		strcat(fURL, params);
		free(params);
		goto end;
	}
	else	//Ô­URLÀïÃ»ÓĞ²ÎÊıµÄ±êÖ¾'?'
	{
		if ( (t = strrchr(tmpURL, '/')))	//²éÕÒ×îÄ©Ò»¸ö'/'
		{
//			if (t == tmpURL + iLen - 1)	//Ä©Î²×Ö·ûÊÇ'/'
			{
				strcat(fURL, tmpURL);	//½«Ô­´®"http://"Ğø½Óµ½Ô­×Ö·û´®"http://"ºó
				goto end;
			}
/*			else	// tÖ¸Ïò×îºóÒ»¸ö'/'£¬µ«²»ÊÇ×îºóÒ»¸ö×Ö·û
			{
				if (strchr(t, '.'))	// ×îºóÒ»¼¶('/'ºó)ÓĞ'.'(ÎÄ¼şÀ©Õ¹Ãû), ËµÃ÷Ô­URL×Ö·û´®ÊÇ¾²Ì¬ÎÄ¼şURL
				{
					strcat(fURL, tmpURL);	//Ö±½Ó½«Ô­´®"http://"Ğø½Óµ½Ô­×Ö·û´®"http://"ºó
					goto end;
				}
				else	// ²»´æÔÚÀ©Õ¹Ãû£¬¿ÉÄÜÊÇ¸öÄ¿Â¼£¬ÔÚ×îÄ©Î²¼Ó'/'
				{
					strcat(fURL, tmpURL);	//½«Ô­´®"http://"Ğø½Óµ½Ô­×Ö·û´®"http://"ºó
					ifLen += iLen;
					//¹ıÂË²ÎÊıÇ°³¬³¤µÄURL
					if (ifLen + 1 >= maxLen)
					{
						return 0;
					}
					strcat(fURL, "/");	//ÔÚÔ­Ä¿±ê×Ö·û´®Î²¼Ó'/'
					goto end;
				}
			}
*/		}
		else	//Ã»ÓĞÕÒµ½'/' (Ã»ÓĞ'/'µÄÓòÃû)
		{
			//Ô­×Ö·û´®ºó²¹'/'
			strcat(fURL, tmpURL);
			ifLen += iLen;
			if (ifLen + 1 >= maxLen)
			{
				return 0;
			}
			strcat(fURL, "/");
			goto end;
		}
	}
	end:
	//´¦Àí³ı×ÖÄ¸¡¢Êı×Ö¡¢'.', '-'ÒÔÍâ¶àÓàµÄ×Ö·û£¬»òÖØ¸´º¬ÓĞ'.'µÄÓòÃû¡£
	if (0 == deal_domain(fURL))
	{
		return 0;
	}
	//ÓòÃû±äĞ¡Ğ´
	fURL += 7;
	while (*fURL && *fURL != '/')
	{
		*fURL = tolower(*fURL);
		fURL++;
	}
	return 1;
}


/*È¡µÃ»ùµØÖ·
 * ²ÎÊı£º
 * 	href:	base±êÇ©ÄÚÈ¡µÃµÄhrefÖµ
 *	base:	·µ»Ø1Ê±ÊÇ¸ù¾İhrefÈ¡µÃµÄ»ùµØÖ·
 * ·µ»Ø£º
 * 	0:	È¡µÃ»ùµØÖ·Ê§°Ü
 *	1:	³É¹¦È¡µÃ»ùµØÖ·
 * */
int make_base(char *href, char *base)
{
	char *temp;	// ³É¹¦È¡µÃ»ùµØÖ·Ç°µÄÁÙÊ±»ùµØÖ·
	int iTLen;	// ÁÙÊ±»ùµØÖ·³¤
	int iHLen;	// »ùÁ´½ÓÊôĞÔÖµ³¤
	char *t;	// Ö¸Õë£¬Ö¸Ïò»ùÁ´½ÓÊôĞÔÖµÀï×îºóÒ»¸ö'/'
	if (!href || !(*href) || !base)
	{
		return 0;
	}
	if (strlen(href) < 3)
	{
		return 0;
	}
	temp = (char *) malloc(MAX_URL_LEN);
	iTLen = 0;
	if (!('h' == *href && 0 == strncmp(href + 1, "ttp://", 6)))	// ²»ÒÔ"http://"¿ª
	{
		strcpy(temp, "http://");	// ÖÃtempÄÚÈİÎª"http://"
		iTLen = 7;
	}
	iHLen = strlen(href);	// »ùÁ´½ÓÊôĞÔÖµ³¤
	if ('/' == *(href + iHLen - 1))	//Èô»ùÁ´½ÓÊôĞÔÖµÊÇÒÔ'/'½áÎ
	{
		goto unite;
	}
	if (!(t = strrchr(href, '/')))	//½öÓòÃû£¬ÇÒÄ©Î²ÎŞ'/'
	{
		//¹ıÂË³¬³¤»ùµØÖ·
		if (iHLen + iTLen + 1 >= MAX_URL_LEN)
		{
			free(temp);
			return 0;
		}
		//ÔÚÄ©Î²²¹¼Ó'/'
		*(href + iHLen) = '/';
		iHLen++;
		*(href + iHLen) = 0;
	}
	else	//´æÔÚ'/'
	{
		*(t + 1) = 0;	//×îÄ©Ò»¸ö'/'ºó×Ö·û´®½ØÖ¹£¬¼´´ø'/'µÄÓòÃû
		iHLen = strlen(href);
	}
	unite:
	//¹ıÂË³¬³¤»ùµØÖ·
	if (iTLen + iHLen >= MAX_URL_LEN)
	{
		free(temp);
		return 0;
	}
	//½«hrefÖµºÏ³ÉÖÁÁÙÊ±»ùµØÖ·×Ö·û´®±äÁ¿
	strncpy(temp + iTLen, href, iHLen);
	*(temp + iTLen + iHLen) = 0;
	//ÁÙÊ±»ùµØÖ·×Ö·û´®=>»ùµØÖ·×Ö·û´®£¬·µ»Ø
	strcpy(base, temp);
	free(temp);
	return 1;
}

/*ÓÉÏà¶ÔµØÖ·ºÍ¾ø¶Ô»ùµØÖ·Éú³É¾ø¶ÔURL
 * ²ÎÊı£º
 *	href:		Ô­×Ö·û´®£¬preFormatºÍºÏ·¨¼ìÑéºóµÄhrefÖµ,Ïà¶ÔµØÖ·
 *	base:		¾ø¶Ô»ùµØÖ·
 *	absoluteURL:Ä¿±ê×Ö·û´®£¬·µ»Ø1Ê±ÊÇ¾ø¶ÔURL
 * ·µ»Ø£º
 * 	0: Ê§°Ü
 * 	1: ³É¹¦
 * */
int make_absolute_url(char *href, char *base, char *absoluteURL)
{
	char *temp;	//Ö¸Õë£¬Ô­×Ö·û´®hrefµÄµØÖ·±¸·İ
	char *astart;	//Ö¸Õë£¬Ö¸ÏòÄ¿±ê×Ö·û´®µÄ²Ù×÷ÆğÊ¼Î»ÖÃ
	char *aend;	//Ö¸Õë£¬Ö¸ÏòÄ¿±ê×Ö·û´®µÄ²Ù×÷ÖÕÖ¹Î»ÖÃ
	int iLen;	//Ô­×Ö·û´®hrefµÄ³¤¶È
	char *t;	//Ä¿±ê×Ö·û´®absoluteURLÖĞÑ°ÕÒ'/'ÓÃ
	int bQues;	//²ÎÊı'?'´æÔÚµÄ±êÖ¾Î»¡£0:²»´æÔÚ, 1:´æÔÚ
	if (!href || !*href)
	{
		return 0;
	}
	//Ô­×Ö·û´®Á´½ÓURLÊ××Ö·ûÊÇ'#'£¬ËµÃ÷ÊÇµ±Ç°Ò³µÄÃª
	if (*href == '#')
	{
		return 0;
	}
	temp = href;
	//½«Ô­×Ö·û´®ÀïµÄ'\'Í³Ò»ÎªÄ¿Â¼·Ö¼¶×Ö·û'/'
	while (*temp)
	{
		if (*temp == '\\')
		{
			*temp = '/';
		}
		temp++;
	}
	//ÔÚ»ùµØÖ·»ù´¡ÉÏÓëÁ´½ÓºÏ³É(ÈôÊÇÏà¶ÔÁ´½Ó)
	strcpy(absoluteURL, base);
	//²Ù×÷Æğ¡¢Ö¹Ö¸Õë¶¨Î»
	//²Ù×÷ÆğÊ¼Ö¸ÕëastartÖ¸ÏòÄ¿±ê×Ö·û´®absoluteURLÀï´Ó"http://.../"(´ø'/'ÓòÃû)ºó±ßÒ»Î»
	//²Ù×÷ÖÕÖ¹Ö¸ÕëaendÖ¸ÏòÄ¿±ê×Ö·û´®absoluteURLÀï×îºóÒ»Î»
	aend = absoluteURL + strlen(absoluteURL);
	if (!(astart = strchr(absoluteURL + 7, '/')))
	{
		return 0;
	}
	astart++;
	//Ô­×Ö·û´®Á´½ÓURLÊ××Ö·ûÊÇ'/'£¬ËµÃ÷ÊÇÓÉ¸ùÄ¿Â¼¿ªÊ¼
	if (*href == '/')
	{
		aend = astart;
		href++;
	}
	//ÒÔÁ¬Ğø2¸ö»ò¶à¸ö'/'¿ªÍ·µÄÔ­Á´½Ó×Ö·û´®£¬ÂÔ¹ı¶àÓà'/'
	while (*href && *href == '/')
	{
		*href++;
	}
	//iLen: Ô­×Ö·û´®hrefµÄÊ£Óà£¨´ı´¦Àí£©³¤¶È
	iLen = strlen(href);
	bQues = 0;
	//Ô­×Ö·û´®ÔÚÏŞ¶¨×î´ó³¤¶ÈÄÚÉĞÓĞÊ£Óà×Ö·û£¬ÇÒÎ´Óöµ½Ãª×Ö·û»ò²ÎÊı
	while ((iLen > 0)
		&& (*href)
//		&& (!(*href == '#' && bQues == 0))
		&& (aend - absoluteURL < MAX_URL_LEN - 1))
	{
		if (iLen >= 3
		 && *href == '.'
		 && *(href + 1) == '.'
		 && *(href + 2) == '/')	// ../
		{
			//ÒÑ¾­ÔÚ¸ùÄ¿Â¼£ºÂÔ¹ı"../"
			if (aend == astart)
			{
				href += 3; iLen -= 3;
				continue;
			}
			//Ä¿±ê×Ö·û´®Î²·Ç'/'£º".."×÷ÎªÄ¿Â¼ÃûµÄÒ»²¿·ÖĞø½Óµ½Ä¿±ê×Ö·û´®Ä©Î²
			if (*(aend - 1) != '/')
			{
				*aend = '.'; aend++;
				*aend = '.'; aend++;
				*aend = '/'; aend++;
				href += 3; iLen -= 3;
				continue;
			}
			//µ±Ä¿±ê×Ö·û´®ÒÔ'/'½áÎ²£¬ÔòÁî¸ÃÎ»Îª½áÊøÎ»
			aend--;
			*aend = 0;
			if (!(t = strrchr(astart, '/')))	//ÈôÄ¿±ê×Ö·û´®"http://"ºóÃ»ÓĞ'/'
			{
				aend = astart;
			}	//Ä©Î»Ö¸ÕëÖ¸Ïò"http://"ºó
			else	//·ñÔò
			{
				//Ä¿±ê´®È¥µô×îºóÒ»¼¶
				aend = t + 1;	//Ä©Î»Ö¸ÕëÖ¸Ïò×îÄ©Ò»¸ö'/'ºó
				//Ô­×Ö·û´®ºóÒÆ3Î»£»Ô­´®Ê£Óà³¤¶ÈÉÙ3
				href += 3; iLen -= 3;
			}
		}
		else if (iLen >= 2 && *href == '.' && *(href + 1) == '/')	// ./
		{
			if (*(aend - 1) != '/')	//ÈôÄ¿±ê×Ö·û´®×îºóÒ»¸ö×Ö·û²»ÊÇ'/'
			{
				//Ä¿±ê×Ö·û´®ºóĞøÉÏ"./"
				*aend = '.'; aend++;
				*aend = '/'; aend++;
				//Ô­×Ö·û´®ºóÒÆ2Î»£»Ô­´®Ê£Óà³¤¶ÈÉÙ2
				href += 2; iLen -= 2;
				continue;	//¼ÌĞøÔ­´®ÀïÏÂÒ»×Ö·û
			}
			// ÈôÄ¿±ê×Ö·û´®×îºóÒ»¸ö×Ö·ûÊÇ'/'
			//Ô­×Ö·û´®ºóÒÆ2Î»£»Ô­´®Ê£Óà³¤¶ÈÉÙ2
			href += 2; iLen -= 2;
		}
		else if (iLen >= 1 && *href == '/')	// '/'
		{
			if (bQues == 0 && *(aend - 1) == '/')	//ÉĞÎ´·¢ÏÖURL²ÎÊı²¿·Ö£¬²¢ÇÒÄ¿±ê´®ÒÔ'/'½áÎ
			{
				//Ô­´®ºóÒÆÒ»Î»
				href++; iLen--;
			}
			else
			{
				//Ä¿±ê´®ĞøÒÔ'/'
				*aend = '/';
				aend++;
				//Ô­´®ºóÒÆÒ»Î»
				href++;	iLen--;
			}
		}
		else	// ÆäËü×Ö·û
		{
			// ÈôÎª'?'£¬bQues±êÖ¾Î»ÖÃ1
			if (*href == '?')
			{
				bQues = 1;
			}
			*aend = *href;	//Ô­´®µ±Ç°×Ö·ûĞø¼Óµ½Ä¿±ê´®ºó
			//Ä¿±ê´®Ö¸ÕëºóÒÆÒ»Î»
			aend++;
			//Ô­´®ºóÒÆÒ»Î»
			href++; iLen--;
		}
	}
	*aend = 0;
	if (strlen(absoluteURL) >= MAX_URL_LEN - 1)
	{
		return 0;
	}
	return 1;
}

/*ÉáÈ¥¾ø¶Ô»ùµØÖ·µÄÎÄ¼şÃû£¨ÈôÓĞ£©£¬¼Ó'/'(ÈôÓĞ±ØÒª)
 * ²ÎÊı£º
 * 	absoluteBase:	¾ø¶Ô»ùµØÖ·
 * 	maxLen:			±»ÏŞ×î´ó³¤¶È
 * ·µ»Ø£º
 * 	0:	Ê§°Ü
 * 	1:	³É¹¦
 * */
int format_base(char *absoluteBase, int maxLen)
{
	int iaLen;	// Ô­¾ø¶Ô»ùµØÖ·absoluteBaseµÄ³¤¶È
	char *t;	// ÔÚabsoluteBaseÖĞ²éÕÒÓÃ
	
	t = strchr(absoluteBase, '?');
	if (t)
		*t = 0;
	iaLen = strlen(absoluteBase);
	if (*(absoluteBase + iaLen - 1) != '/')
	{
		if ( (t = strrchr(absoluteBase + 7, '/')))	//²»ÊÇ²»´ø'/'µÄÓòÃû
		{
			if (strchr(t + 1, '.'))	//ÓòÃûºóÓĞÎÄ¼şÃûºó×º
			{
				// turn the one after the last '/' to '/', and end the absoluteBase string
				*(t + 1) = '/';
				*(t + 2) = 0;
			}
			else	//ÎŞºó×º
			{
				if (iaLen + 1 > maxLen)
				{
					return 0;
				}
				//add '/' to the end
				*(absoluteBase + iaLen) = '/';
				*(absoluteBase + iaLen + 1) = 0;
			}
		}
		else	//only domain
		{
			if (iaLen + 1 > maxLen)
			{
				return 0;
			}
			//add '/' to the end
			*(absoluteBase + iaLen) = '/';
			*(absoluteBase + iaLen + 1) = 0;
		}
	}
	return 1;
}

// ±È½ÏÁ½¸ö²ÎÊıµÄ´óĞ¡£¬·µ»ØÖµÀàËÆstrcmpº¯Êı
inline int param_compare(struct Param *p1, struct Param *p2)
{
	int value;

	value = strcmp(p1->name, p2->name);
	if (value == 0)
	{
		value = strcmp(p1->value, p2->value);
	}

	return value;
}

/*½«name1=value1&name2=valueĞÎÊ½µÄ²ÎÊı´®£¬°´nameÅÅĞòĞÎ³ÉĞÂ×Ö·û´®
 * ²ÎÊı£º
 * 	params:Ô­²ÎÊı´®
 * 	orderedParams:ÅÅĞòºóµÄ²ÎÊı×Ö·û´®
 * ·µ»Ø£º0³É¹¦,-1Ê§°Ü
 * 
 * */
int order_params(char *params, char *orderedParams)
{
	char *paramStart, *paramEnd;	// the start and the end position of each param
	int paramLen;					// ²ÎÊıµÄ³¤¶ÈparamEnd - paramStart
	int iLen;	//length of params string
	int ioLen;	//length of orded params string
	int i, count;	//count of param pairs
	int min;	//±È½ÏÓÃ×îĞ¡name´®µÄĞòºÅ
	int cmpValue;

	struct Param tPa[200];	//a URL can have 200 params at max

	if (!params || !(*params))
	{
		return -1;
	}
	iLen = strlen(params);
	paramStart = params;
	count = 0;
	while (iLen > 0)
	{
		// ÕÒ²ÎÊı½áÊøÎ»ÖÃ
		paramEnd = strchr(params, '&');
		if (paramEnd == NULL)
		{
			paramEnd = params + iLen;
		}
		paramLen = paramEnd - paramStart;
		// ÒÆ¶¯Ö¸ÕëÎ»ÖÃ
		params = paramEnd + 1;
		iLen -= params - paramStart;
		if (paramLen == 0)
		{
			// ¿Õ²ÎÊı
			paramStart = params;
			continue;
		}
		//½«²ÎÊı¶Ô±£´æÖÁ½á¹¹»¯Êı×éÀï
		strncpy(tPa[count].str, paramStart, paramLen);
		*(tPa[count].str + paramLen) = 0;
		paramStart = params;
		// È¥³ıÃ»ÓĞÖµµÄ·Ç²¼¶û²ÎÊı
		if (*(paramEnd - 1) == '=')
			continue;
		tPa[count].bOrdered = 0;
		count++;
	}
	
	for (i = 0; i < count; i++)
	{
		tPa[i].resultType = str2pair(tPa[i].str, tPa[i].name, tPa[i].value);
	}

	// ÅÅĞò²ÎÊı
	ioLen = 0;
	while (1)
	{
		//seek for an unordered one
		for (min = count - 1; min >= 0 && tPa[min].bOrdered == 1; min--)
			;
		if (min < 0)	//all ordered
		{
			break;
		}
		//seek for the minimal name
		for (i = 0; i < count; i++)
		{
			if (i != min && tPa[i].bOrdered == 0)
			{
				cmpValue = param_compare(&(tPa[i]), &(tPa[min]));
				/*
				if (cmpValue != 0 && strcmp(tPa[i].name, tPa[min].name) == 0)
				{
					// ÏàÍ¬²ÎÊıÃû£¬²»Í¬²ÎÊıÖµ£¬·µ»Ø´íÎó
					return -1;
				}
				*/
				if (cmpValue < 0)
				{
					//current is smaller then min one
					min = i;
				}
				else if (cmpValue == 0)
				{
					// ÏàÍ¬²ÎÊıÃû£¬ÏàÍ¬²ÎÊıÖµ£¬Ìø¹ı
					tPa[i].bOrdered = 1;
					continue;
				}
			}
		}
		tPa[min].bOrdered = 1;
		if (tPa[min].resultType == 1)
		{
			continue;
		}
		if (ioLen != 0)
		{
			*(orderedParams + ioLen) = '&';
			ioLen++;
			*(orderedParams + ioLen) = 0;
		}
		strcpy(orderedParams + ioLen, tPa[min].name);
		ioLen += strlen(tPa[min].name);
		if (tPa[min].resultType == 2)
		{
			continue;
		}
		*(orderedParams + ioLen) = '=';
		ioLen++;
		strcpy(orderedParams + ioLen, tPa[min].value);
		ioLen += strlen(tPa[min].value);
	}
	//printf("orderedParams:%s\n", orderedParams);
	return 0;
}

// Ë÷Òıurl¹éÒ»»¯
// url±ØĞëÎª¸ñÊ½»¯¹ıµÄ²»´ø²ÎÊıµÄurl
// multiLevel±íÊ¾ÊÇ·ñ¶ÔÄ¿Â¼Ò²½øĞĞ¹éÒ»»¯´¦Àí
void uniform_index_url(char *url, int multiLevel)
{
/*	char newUrl[MAX_URL_LEN];
	char *t;
	char *fileName;

	strcpy(newUrl, url);
	str2lower(newUrl);

	// tÖ¸Ïòhttp://Ö®ºó
	t = newUrl + 7;
	t = strchr(t, '/');
	if (t == NULL)
		return;

	// fileNameÖ¸ÏòurlÖĞµÄÎÄ¼şÃû
	fileName = strrchr(t, '/');
	if (fileName < t || (multiLevel == FALSE && fileName > t))
		return;

	// Ìø¹ı¶¯Ì¬ÍøÒ³
	if (strchr(fileName, '?'))
		return;

	fileName++;
	// ½ö´¦Àíindex.*ºÍdefault.*
	if (strncmp(fileName, "index.", 6) == 0 || strncmp(fileName, "default.", 8) == 0)
	{
		t = url + (fileName - newUrl);
		*t = '\0';
	}
*/
}

// ¸ñÊ½»¯httpĞ­ÒéµÄurl£¬³É¹¦·µ»Ø0£¬Ê§°Ü·µ»Ø1
int format_http_url(const char *rawUrl, char *destUrl)
{
	char *p;
	char tmpUrl[MAX_URL_LEN];

	if (!rawUrl || !(*rawUrl) || !destUrl)
	{
		return 1;
	}

	if (strlen(rawUrl) > MAX_URL_LEN - 1)
	{
		return 1;
	}
	
	p = (char *)rawUrl;
	if (strstr(rawUrl, "://"))
	{
		if (strncasecmp(rawUrl, "http://", 7))
		{
			return 1;
		}
		p += 7;
	}
	strncpy(tmpUrl, "http://", MAX_URL_LEN);
	strncat(tmpUrl, p, MAX_URL_LEN - 7);
	tmpUrl[MAX_URL_LEN - 1] = 0;

	if (pre_format(tmpUrl, destUrl, MAX_URL_LEN) == 0)
	{
		return 1;
	}

	// ´Ë´¦µÄtmpUrlºÍdestUrlºÍ×ÖÃæÒâË¼Ïà·´£¬ºÇºÇ
	if (format_absolute_url(destUrl, tmpUrl, MAX_URL_LEN) == 0)
	{
		return 1;
	}
	
	if (url_encode(tmpUrl, destUrl))
	{
		return 1;
	}

	return 0;
}

// ¸ñÊ½»¯httpĞ­ÒéµÄÏà¶Ôurl£¬³É¹¦·µ»Ø0£¬Ê§°Ü·µ»Ø1
// baseUrlÓ¦¸ÃÎªÒÑ¾­¸ñÊ½»¯¹ıµÄ¾ø¶Ôurl
int format_http_relative_url(const char *baseUrl, const char *relativeUrl, char *destUrl)
{
	int relative;
	char absoluteBase[MAX_URL_LEN];
	char *t;
	int tlen;
	char tmpUrl[MAX_URL_LEN];

	if (!baseUrl || !(*relativeUrl) || !destUrl)
	{
		return 1;
	}

	if (strlen(baseUrl) > MAX_URL_LEN - 1 || strlen(relativeUrl) > MAX_URL_LEN - 1)
	{
		return 1;
	}
	
	// Éú³É»ùµØÖ·
	strcpy(absoluteBase, baseUrl);
	t = strchr(absoluteBase, '?');
	if (t)
		*t = 0;
	
	// Çø·ÖrelativeUrlÊÇ·ñ½öº¬²ÎÊı£¬½öÊÇ²ÎÊıµÄÊ±ºòabsoluteBase²»ÄÜ½Ø¶Ïµ½Ä¿Â¼
	if (*relativeUrl != '?')
	{
		if (strncmp(absoluteBase, "http://", 7) == 0)
			t = strrchr(absoluteBase + 7, '/');
		else
			t = strrchr(absoluteBase, '/');
		if (t)
		{
			*(t + 1) = 0;
		}
		else
		{
			t = absoluteBase;
			tlen = strlen(t);
			*(t + tlen) = '/';
			*(t + tlen + 1) = 0;
		}
	}
	
	relative = 1;
	if (strstr(relativeUrl, "://"))
	{
		if (strncasecmp(relativeUrl, "http://", 7))
		{
			return 1;
		}
		relative = 0;
	}
	
	// Á´½ÓÊôĞÔÖµÔ¤±ê×¼»¯
	if (pre_format((char *)relativeUrl, destUrl, MAX_URL_LEN) == 0)
	{
		return 1;
	}
	
	// ÓĞĞ§ĞÔ¼ìÑé
	if (is_valid_link(destUrl) == 0)
	{
		return 1;
	}
	if (relative)
	{
		// Éú³É¾ø¶ÔURL£¬´Ë´¦µÄtmpUrlºÍdestUrlºÍ×ÖÃæÒâË¼Ïà·´£¬ºÇºÇ
		if (make_absolute_url(destUrl, absoluteBase, tmpUrl) == 0)
		{
			return 1;
		}
		strncpy(destUrl, tmpUrl, MAX_URL_LEN);
	}

	// ´Ë´¦µÄtmpUrlºÍdestUrlºÍ×ÖÃæÒâË¼Ïà·´£¬ºÇºÇ
	if (format_absolute_url(destUrl, tmpUrl, MAX_URL_LEN) == 0)
	{
		return 1;
	}
	
	if (url_encode(tmpUrl, destUrl))
	{
		return 1;
	}

	return 0;
}

/*
 *¶ÔÁ´½Ó×Ö·û´®ÀïµÄÌÓÒİ×Ö·û½øĞĞ±àÂë
 *²ÎÊı
 * strSrc£ºÔ´×Ö·û´®£¬Á´½Ó×Ö·û´®¡£
 * strDest£º·µ»Ø0Ê±ÊÇ±àÂëºóµÄ×Ö·û´®¡£
 *·µ»Ø£º
 * 0£º³É¹¦
 * 1£º²ÎÊı´í£¬»ò×ªºó×Ö·û´®³¬³¤¡£
 *
 *¸½URL×Ö·û¼¯ºÏ¹ØÏµ£º
 *	uric		= reserved | unreserved | escaped
 *	reserved	= ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
 *	unreserved  = alphanum | mark
 *	mark		= "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
 *	alphanum = alpha | digit
 *	digit    = "0" -"9"
 *	alpha    = lowalpha | upalpha
 *	lowalpha = "a" - "z"
 *	upalpha  = "A" - "Z"
 */
int url_encode(char *strSrc, char *strDest)
{
	int counter = 0;
	char temp[3];
	unsigned char ch;
	int i, j, len;
	char reserved[] =
	{
		';', '/', '?', ':', '@', '&', '=', '+', '$', ','
	};
	char mark[] =
	{
		'-', '_', '.', '!', '~', '*', '\'', '(', ')'
	};
	int isEscaped;
	int bQues;
	if (!strSrc || !strDest)
	{
		return 1;
	}
	len = strlen(strSrc);
	bQues = 0;
	for (i = 0; i < len; i++)
	{
		ch = (unsigned char) (strSrc[i]);
		if ((ch > 0) && (ch < 127))
		{
			isEscaped = 1;
			if (ch == '%')
			{
				isEscaped = 0;
			}
			if (isEscaped)
			{
				for (j = 0; j < sizeof(reserved) / sizeof(char); j++)
				{
					if (ch == reserved[j])
					{
						isEscaped = 0;
						break;
					}
				}
			}
			if (isEscaped)
			{
				for (j = 0; j < sizeof(mark) / sizeof(char); j++)
				{
					if (ch == mark[j])
					{
						isEscaped = 0;
						break;
					}
				}
			}
			if ((isEscaped)
			 && ((ch == '%')
			  || ((ch >= '0') && (ch <= '9'))
			  || ((ch >= 'A') && (ch <= 'Z'))
			  || ((ch >= 'a') && (ch <= 'z'))))
			{
				isEscaped = 0;
			}
			if ((isEscaped) && (ch == '\\'))	//½«ÒÔ '\' ×÷Ä¿Â¼·Ö¸ô·ûµÄ£¬×ª»»Îª '/' ×Ö·û
			{
				ch = '/';
				isEscaped = 0;
			}
			if (isEscaped == 0)
			{
				if (counter + 1 < MAX_URL_LEN)
				{
					*(strDest++) = ch;
					counter++;
				}
				else
				{
					return 1;
				}
			}
			else
			{
				// º¬ÓĞ¿Õ°×µÄurl²»Òª£¬¶àÊıÎŞÓÃ
				if (is_space(ch))
					return 1;
				if (counter + 2 < MAX_URL_LEN - 1)
				{
					sprintf(temp, "%x", ch);
					*strDest ++ = '%';
					*strDest ++ = temp[0];
					*strDest ++ = temp[1];
					counter += 3;
				}
				else
				{
					return 1;
				}
			}
		}
		else
		{
			if (counter + 3 < MAX_URL_LEN - 1)
			{
				sprintf(temp, "%x", ch);
				*strDest ++ = '%';
				*strDest ++ = temp[0];
				*strDest ++ = temp[1];
				counter += 3;
			}
			else
			{
				return 1;
			}
		}
	}
	*strDest = '\0';
	return 0;
}

int is_dynamic_url(const char *url)
{
	while (*url != 0)
	{
		if (*url == '?')
			return 1;
		url++;
	}
	return 0;
}


