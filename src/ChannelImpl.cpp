/*************************************************************************
	> File Name: ChannelImpl.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Thu 20 Mar 2014 06:25:33 PM CST
	> 分类处理路由
 ************************************************************************/

#include "ChannelImpl.h"
#include "GlobalConfig.h"
using namespace std;

ChannelImpl::ChannelImpl(){

}

ChannelImpl::~ChannelImpl(){

}

int ChannelImpl::ChargeRequest(TopupInfo *m_topup_info){
	int ret = 0;
	//foreach the channels
	vector<ChannelInfo>::iterator channels = m_topup_info->channels.begin();
	for(; channels != m_topup_info->channels.end(); ++channels){
		if(m_topup_info->interfaceName == ""){
	
		}else if(m_topup_info->interfaceName == ""){
	
		}else{
		
		}
	}
	return ret;
}

//ret = 0 normal
int ChannelImpl::QueryRequest(TopupInfo *m_topup_info){
	int ret = 0;
	string result;
	if(strcmp(m_topup_info->interfaceName.c_str(), GlobalConfig::Instance()->p_sls_interface) == 0){
		Channel *channel = new ChannelSLS();
		int ret = channel->Query(m_topup_info, result);
		///parse result
	}else if(strcmp(m_topup_info->interfaceName.c_str(),GlobalConfig::Instance()->p_llww_interface)){
	
	}else if(strcmp(m_topup_info->interfaceName.c_str(),GlobalConfig::Instance()->p_yeepay_interface)){

	}else{
		ret = 1;
	}
	return ret;
}

// ret = 0 normal
int ChannelImpl::BalanceRequest(TopupInfo *m_topup_info, double &balance){
	int ret = 0;
	string result;
	if(m_topup_info->interfaceName == ""){
	
	}else if(m_topup_info->interfaceName == ""){
	
	}else{
		ret = 1;
	}
	return ret;
}

// ret = 0 normal
int ChannelImpl::AcceptNotify(TopupInfo *m_topup_info){
	int ret = 0;
	string result;
	if(m_topup_info->interfaceName == ""){
	
	}else if(m_topup_info->interfaceName == ""){
	
	}else{
	
	}
	return ret;
}

//动态链接库调用接口，用于创建相应实例
extern "C" ChannelBase* channel_create() {
     return new ChannelImpl;
}
//动态链接库调用接口，用于销毁相应的实例,可不可以通过得到的指针直接销毁
extern "C" void channel_destroy(ChannelBase* p) {
    delete p;
}
