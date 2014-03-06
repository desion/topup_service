#include <Topup.h>
#include "TopupInterface_types.h"
#include "TopupUtils.h"
#include "slog.h"

using namespace  ::topupinterface;
class TopupService:virtual public TopupIf{
	public:
	TopupService(){
	}

	// 发送充值，查询订单，查询余额，回调，取消订单等请求
	void SendRequest(std::string& _return, const TopupRequest& request);
	// 发送服务管理等请求
    int32_t Admin(const ManageRequest& request);
};
