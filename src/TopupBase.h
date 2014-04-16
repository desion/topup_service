/*************************************************************************
	> File Name: TopupBase.h
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Sat 08 Feb 2014 03:21:15 PM CST
 ************************************************************************/
#ifndef __TOPUP_BASE_H
#define __TOPUP_BASE_H

#include <iostream>
#include "TopupInterface_types.h"
#include "occi.h"
#include "TopupUtils.h"

using namespace std;
using namespace  ::topupinterface;  
using namespace oracle::occi;

//class TopupServer;
struct TopupInfo;

class TopupBase{
	public:
		virtual ~TopupBase(){};
		virtual int HandleRequest(const TopupRequest& request, string& result) = 0;
		virtual int Notify() = 0;
		virtual int Init(TopupInfo* m_topup_info) = 0;
};

typedef TopupBase* (*create_t)();
typedef void (*destroy_t)(TopupBase*);



#endif //__TOPUP_BASE_H
