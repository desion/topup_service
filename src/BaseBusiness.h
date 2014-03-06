#include <iostream>
#include <occi.h>
#include <vector>
#include "glog/logging.h"
#include <exception>

#ifndef __BASEBUSINESS_H_
#define __BASEBUSINESS_H_

using namespace std;
using namespace oracle::occi;

class BaseBusiness{
	protected:
		vector<string> errors;
		Connection *conn;
	public:
		BaseBusiness(){
			//dcm = ConnectionManager::Instance();
		}
		
		void HandleException(std::exception &e);
		
		void Finish();

		void Init(Connection *m_conn);

		bool HasError();

		vector<string> & GetErrors();
};

#endif //__BASEBUSINESS_H_

