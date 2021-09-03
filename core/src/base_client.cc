// file base_client.cc
#include "base_client.h"

bool BaseClient::MakeCredentials(const ViaInfo& via_info)
{
	#if(1 == SSL_TYPE)
	{
		if(via_info.server_cert_path_.empty() || via_info.client_key_path_.empty() || 
		   via_info.client_cert_path_.empty())
		{
			gpr_log(GPR_ERROR, "Invalid client openssl certificate, please check!");
			return false;
		}
		auto str_root_crt = get_file_contents(via_info.server_cert_path_); // for verifying clients
		auto str_client_key = get_file_contents(via_info.client_key_path_);
		auto str_client_cert = get_file_contents(via_info.client_cert_path_);

		grpc::SslCredentialsOptions ssl_opts;
		ssl_opts.pem_root_certs  = str_root_crt.c_str();
		ssl_opts.pem_private_key = str_client_key.c_str();
		ssl_opts.pem_cert_chain  = str_client_cert.c_str();
		creds_ = grpc::SslCredentials(ssl_opts);
	}
	#elif(2 == SSL_TYPE) 
	{
		if(via_info.server_cert_path_.empty() || via_info.client_sign_key_path_.empty() || 
		via_info.client_sign_cert_path_.empty() || via_info.client_enc_key_path_.empty() ||
		via_info.client_enc_cert_path_.empty())
		{
			gpr_log(GPR_ERROR, "Invalid client gmssl certificate, please check!");
			return false;
		}

		grpc::SslCredentialsOptions ssl_opts;
		ssl_opts.pem_root_certs  = via_info.server_cert_path_.c_str();
		ssl_opts.pem_private_key = via_info.client_sign_key_path_.c_str();
		ssl_opts.pem_cert_chain  = via_info.client_sign_cert_path_.c_str();
		ssl_opts.pem_enc_private_key =  via_info.client_enc_key_path_.c_str();
		ssl_opts.pem_enc_cert_chain = via_info.client_enc_cert_path_.c_str();

		creds_ = grpc::SslCredentials(ssl_opts);
	}	
	#else
	{
		creds_ = grpc::InsecureChannelCredentials();
	}
	#endif

	return true;
}

BaseClient::BaseClient(const ViaInfo& via_info, const string& taskid)
{
	task_id_ = taskid;
	server_nid_ = via_info.id;
	if(!MakeCredentials(via_info)){return;}
	auto channel = grpc::CreateChannel(via_info.address, creds_);
	stub_ = IoChannel::NewStub(channel);
}

BaseClient::BaseClient(const NodeInfo& node_info, const string& taskid)
{
	task_id_ = taskid;
	server_nid_ = node_info.id;
	ViaInfo via_info;
	via_info.address = node_info.via_address;
	
	#if(1 == SSL_TYPE)
		via_info.server_cert_path_ = node_info.ca_cert_path_;
		via_info.client_key_path_ = node_info.client_key_path_;
		via_info.client_cert_path_ = node_info.client_cert_path_;
	#elif(2 == SSL_TYPE) 
		via_info.server_cert_path_ = node_info.ca_cert_path_;
		via_info.client_sign_key_path_ = node_info.client_sign_key_path_;
		via_info.client_sign_cert_path_ = node_info.client_sign_cert_path_;
		via_info.client_enc_key_path_ = node_info.client_enc_key_path_;
		via_info.client_enc_cert_path_ = node_info.client_enc_cert_path_;
	#endif

	if(!MakeCredentials(via_info)){return;}
	auto channel = grpc::CreateChannel(via_info.address, creds_);
	via_stub_ = VIAService::NewStub(channel);
}

bool BaseClient::SignUpToVia(const NodeInfo& server_info)
{
	if(nullptr == via_stub_)
	{
		gpr_log(GPR_ERROR, "uninitialized stub to connect to the VIA service, please initialize!");
		return false;
	}

	grpc::ClientContext context;
	context.AddMetadata("task_id", task_id_);
	context.AddMetadata("party_id", server_info.id);

	SignupReq reg_req;
	Boolean ret_code;
	reg_req.set_taskid(task_id_);
	reg_req.set_partyid(server_info.id);
	reg_req.set_servicetype("");
	reg_req.set_address(server_info.address);

	via_stub_->Signup(&context, reg_req, &ret_code);
	if (false == ret_code.result()) 
	{
		gpr_log(GPR_ERROR, "Signup via server failed, via server address:%s!", server_info.via_address.c_str());
		return false;
	}    
	return true;
}
