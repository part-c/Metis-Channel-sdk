// file base_server.cc
#include "base_server.h"
#include <grpc++/security/credentials.h>
#include <thread>
#include <chrono>   
using namespace chrono;

BaseServer::BaseServer(const NodeInfo& server_info, 
	map<string, shared_ptr<ClientConnection>>* ptr_client_conn_map)
{
	ptr_client_conn_map_ = ptr_client_conn_map;

	// ServerBuilder builder;
	builder_ = make_shared<ServerBuilder>();
	builder_->SetMaxReceiveMessageSize(INT_MAX);
	// Listen on the given address without any authentication mechanism.
	std::shared_ptr<grpc::ServerCredentials> creds;
	
	#if(1 == SSL_TYPE)  
	{   // openssl
		if(server_info.ca_cert_path_.empty() || server_info.server_key_path_.empty() || server_info.server_cert_path_.empty())
		{
			const char* strErrMsg = "Invalid server openssl certificate, please check!";
			gpr_log(GPR_ERROR, "%s", strErrMsg);
			throw (strErrMsg);
		}
		auto str_root_crt = get_file_contents(server_info.ca_cert_path_.c_str()); // for verifying clients
    	auto str_server_key = get_file_contents(server_info.server_key_path_.c_str());
    	auto str_server_cert = get_file_contents(server_info.server_cert_path_.c_str());
		grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = {str_server_key.c_str(), str_server_cert.c_str()};
		grpc::SslServerCredentialsOptions ssl_opts(GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY);
		ssl_opts.pem_root_certs = str_root_crt.c_str();
		ssl_opts.pem_key_cert_pairs.push_back(pkcp);
		creds = grpc::SslServerCredentials(ssl_opts);
	}
	#elif(2 == SSL_TYPE)  
    {	// gmssl
        if(server_info.ca_cert_path_.empty() || server_info.server_sign_key_path_.empty() || 
		   server_info.server_sign_cert_path_.empty() || server_info.server_enc_key_path_.empty() ||
		   server_info.server_enc_cert_path_.empty() )
        { 
			const char* strErrMsg = "Invalid server gmssl certificate, please check!";
			gpr_log(GPR_ERROR, "%s", strErrMsg);
			throw (strErrMsg);
        }

        grpc::SslServerCredentialsOptions::PemKeyCertPair sig_pkcp = {
			server_info.server_sign_key_path_.c_str(), server_info.server_sign_cert_path_.c_str()};

        grpc::SslServerCredentialsOptions::PemKeyCertPair enc_pkcp = {
			server_info.server_enc_key_path_.c_str(), server_info.server_enc_cert_path_.c_str()};
        grpc::SslServerCredentialsOptions ssl_opts(GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY);
        ssl_opts.pem_root_certs = server_info.ca_cert_path_.c_str();
        ssl_opts.pem_key_cert_pairs.push_back(sig_pkcp);
        ssl_opts.pem_key_cert_pairs.push_back(enc_pkcp);
        creds = grpc::SslServerCredentials(ssl_opts);
    }
	#else
	{
		creds = grpc::InsecureServerCredentials();
	}
	#endif
	builder_->AddListeningPort(server_info.address, creds);
}

