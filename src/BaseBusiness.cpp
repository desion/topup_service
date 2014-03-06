/*************************************************************************
	> File Name: BaseBusiness.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Wed 15 Jan 2014 07:38:57 PM CST
 ************************************************************************/

#include<iostream>
#include<exception>
#include "BaseBusiness.h"
#include "TopupServer.h"
using namespace std;

void BaseBusiness::HandleException(std::exception &e){
	errors.push_back("Exception");
	LOG(ERROR) << e.what();
}

void BaseBusiness::Finish(){
    try{
	    if(conn != NULL){
			if(errors.empty()){
				conn->commit();	
			}else{
				conn->rollback();
			}
		}
	}catch(SQLException &sqlExcp){
		TP_WRITE_LOG(P_TPServer, "\t{%s}", sqlExcp.getMessage().c_str());
	    //LOG(ERROR) << sqlExcp.getMessage();
	}
	TP_WRITE_LOG(P_TPServer, "\t{FINISH}");
	/*
	if(conn != NULL){
		P_TPServer->conn_manager->Recover(conn);
	}
	*/
}

void BaseBusiness::Init(Connection *m_conn){
	conn = m_conn;
}
