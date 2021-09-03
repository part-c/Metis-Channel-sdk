
// file async_client.h
#pragma once
#include "base_client.h"

using grpc::ClientAsyncResponseReader;
using grpc::ClientAsyncReader;
using grpc::ClientAsyncWriter;
using grpc::ClientAsyncReaderWriter;
using grpc::CompletionQueue;

class AbstractAsyncClientCall
{
public:
	enum CallStatus { PROCESS, FINISH, DESTROY };

	explicit AbstractAsyncClientCall():callStatus(PROCESS){}
	virtual ~AbstractAsyncClientCall(){}
	RetCode reply;
	ClientContext context;
	Status status;
	CallStatus callStatus ;
	SendRequest request_;
	string remote_nodeid_;
	virtual uint32_t Proceed(bool = true) = 0;
};

class AsyncClientCall: public AbstractAsyncClientCall
{
	std::unique_ptr< ClientAsyncResponseReader<RetCode> > responder;
public:
	AsyncClientCall(const string& task_id, const string& remote_nodeid, 
		const SendRequest& request, CompletionQueue& cq_, 
		std::unique_ptr<IoChannel::Stub>& stub_);
	virtual uint32_t Proceed(bool ok = true){};
};

class AsyncClient : public BaseClient
{
public:
    AsyncClient(const ViaInfo& via_info, const string& taskid);

	void SendReqAgain(const AbstractAsyncClientCall* call);
	// Assembles the client's payload and sends it to the server.
	ssize_t send(const string& msg_id, const char* data, const size_t nLen, int64_t timeout = -1L);

	~AsyncClient(){cq_.Shutdown();}
	void AsyncCompleteRpc();
private:
    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq_;
};

