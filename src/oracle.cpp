// OracleTest.cpp : 定义控制台应用程序的入口点。
//
 
#include "iostream"
#include "occi.h"
#include "glog/logging.h"
#include <stdio.h>
#include <string>
 
using namespace oracle::occi;
using namespace std;

inline int split_string (char * s, const char * seperator, std::vector<char *> & field_vec)
{
   field_vec.clear();
   if (s == NULL) return -1;
   if (seperator == NULL) return -1;
   int sep_len = strlen(seperator);
   char * p = s;
   field_vec.push_back(p);
   while ((p = strstr(p, seperator)) != NULL)
   {
        *p = '\0';
        p += sep_len;
        field_vec.push_back(p);
    }
    return field_vec.size();
}
 
int main(int argc, char* argv[])
{
     
  try
    {
	FILE *op = fopen(argv[1], "r");
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "hello glog";
        const string userName = "zkcl";
        const string password = "zkcl";
        const string connectString= "//117.121.26.19:1521/zkcl";
 
        unsigned int maxConn=10;
        unsigned int minConn=1;
        unsigned int incrConn=2;
 
        oracle::occi::Environment *env = oracle::occi::Environment::createEnvironment("ZHS16GBK","UTF8");
         
        //建立连接池
        ConnectionPool *connPool=env->createConnectionPool(
            userName,
            password,
            connectString,
            minConn,
            maxConn,
            incrConn);
 
        //从连接池获取连接
		char buf[2048];
		vector<char *> field_vec;
		Statement *stmt;
		Connection *conn=connPool->createConnection(userName,password);
		while(fgets(buf, 2047,op) != NULL){
			printf(buf);
			buf[strlen(buf) -1] = '\0';
			try{
				stmt = conn->createStatement(buf);
				stmt->executeUpdate();
				conn->commit();
			}catch (SQLException &Excp){
				string strinfo=Excp.getMessage();
				cout<<strinfo;
				conn->terminateStatement(stmt);
				continue;	
			}
			conn->terminateStatement(stmt);
		}
		connPool->terminateConnection(conn);
        env->terminateConnectionPool(connPool);
        //env->terminateConnection(conn);
        oracle::occi::Environment::terminateEnvironment(env);
    }
    catch (SQLException &sqlExcp)
    {
        int i=sqlExcp.getErrorCode();
        string strinfo=sqlExcp.getMessage();
        cout<<strinfo;
    }
     
  int i;
  return 0;
}

