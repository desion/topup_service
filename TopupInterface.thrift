namespace cpp topupinterface

struct TopupRequest{
	1: string query
	2: string checksum
	3: string ip
	4: string version
	5: string uri
	6: i32 itimestamp
	7: map<string, string> exparams
}

struct ManageRequest{
	1: i32 cmd
	2: string key
	3: string value
}

service Topup{
	string SendRequest(1:TopupRequest request),
	i32 Admin(1:ManageRequest request)
}
