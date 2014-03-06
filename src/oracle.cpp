// OracleTest.cpp : �������̨Ӧ�ó������ڵ㡣
//
 
#include "iostream"
#include "occi.h"
#include "glog/logging.h"
 
 
using namespace oracle::occi;
using namespace std;
 
int main(int argc, char* argv[])
{
     
  try
    {
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "hello glog";
        const string userName = "zkcl";
        const string password = "zkcl";
        const string connectString= "//117.121.26.10:1521/zkcl";
 
        unsigned int maxConn=5;
        unsigned int minConn=1;
        unsigned int incrConn=2;
 
        oracle::occi::Environment *env = oracle::occi::Environment::createEnvironment("ZHS16GBK","UTF8");
         
        //�������ӳ�
        ConnectionPool *connPool=env->createConnectionPool(
            userName,
            password,
            connectString,
            minConn,
            maxConn,
            incrConn);
 
        //�����ӳػ�ȡ����
        Connection *conn=connPool->createConnection(userName,password);
        Statement *stmt = conn->createStatement("select 1 from dual");
        ResultSet *rs = stmt->executeQuery();
        while (rs->next())
        {
 
            int l1=rs->getInt(1);
            cout <<"�û�ID\t"<<l1 <<endl;
        }
        stmt->closeResultSet(rs);
        conn->terminateStatement(stmt);
        connPool->terminateConnection(conn);
        //�ͷ�����
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

