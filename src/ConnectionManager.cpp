/*************************************************************************
	> File Name: ConnectionManager.cpp
	> Author: desionwang
	> Mail: wdxin1322@qq.com 
	> Created Time: Wed 15 Jan 2014 01:53:41 PM CST
 ************************************************************************/

#include<iostream>
#include "ConnectionManager.h"
using namespace std;

ConnectionManager ConnectionManager::m_connention_manager;

bool ConnectionManager::Init(string userName, string passWord,
		const string connectString,uint32_t maxConnetion,
		uint32_t minConnection,uint32_t incrConnection){
    try
    {
        m_userName = userName;
        m_passWord = passWord;
        oracle::occi::Environment *env = oracle::occi::Environment::createEnvironment("ZHS16GBK","UTF8");

        m_connection_pool = env->createConnectionPool(
            m_userName,
            m_passWord,
            connectString,
            minConnection,
            maxConnetion,
            incrConnection);
    }
    catch (SQLException &sqlExcp)
    {
        int i = sqlExcp.getErrorCode();
        string strinfo=sqlExcp.getMessage();
        LOG(ERROR) << strinfo;
		return false;
    }
    return true;
	
}
//创建连接
Connection * ConnectionManager::CreateConnection(){
	//cout << "CreateConnection:" << m_userName << "\t" << m_passWord << endl;
	Connection* conn = NULL;
    try
    {
		conn =  m_connection_pool->createConnection(m_userName,m_passWord);
	}
	catch (SQLException &sqlExcp)
    {
        int i = sqlExcp.getErrorCode();
        string strinfo=sqlExcp.getMessage();
        LOG(ERROR) << strinfo;
    }
	return conn;
}

void ConnectionManager::Recover(Connection *conn){
	m_connection_pool->terminateConnection(conn);
}
