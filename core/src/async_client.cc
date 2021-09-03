// file async_client.cc
#include "async_client.h"
#if USE_BUFFER
#include "simple_buffer.h"
#endif
#include <thread>
#include <chrono>   
using namespace chrono;
extern string g_self_nodeid;
AsyncClientCall::AsyncClientCall(const string& task_id, const string& remote_nodeid,
		const SendRequest& request, CompletionQueue& cq_, 
		std::unique_ptr<IoChannel::Stub>& stub_):AbstractAsyncClientCall()
{
	request_ = request;
	remote_nodeid_ = remote_nodeid;
	// std::cout << "[Proceed11]: new client, name:" << request.name() << std::endl;
	context.AddMetadata("task_id", task_id);
    context.AddMetadata("party_id", remote_nodeid);
	
	responder = stub_->PrepareAsyncSend(&context, request, &cq_);
	responder->StartCall();
	responder->Finish(&reply, &status, (void*)this);
	callStatus = PROCESS ;
}

AsyncClient::AsyncClient(const ViaInfo& via_info, const string& taskid):
	BaseClient(via_info, taskid){}

void AsyncClient::SendReqAgain(const AbstractAsyncClientCall* call)
{
	new AsyncClientCall(task_id_, call->remote_nodeid_, call->request_, cq_, stub_);
}

// Assembles the client's payload and sends it to the server.
ssize_t AsyncClient::send(const string& msg_id, const char* data, const size_t nLen, int64_t timeout)
{
	SendRequest req_info;
	// 发送客户端的nodeid到服务器
	req_info.set_nodeid(g_self_nodeid);

#if USE_BUFFER
  	simple_buffer buffer(msg_id, data, nLen);
	req_info.set_data((const char*)buffer.data(), buffer.len());
#else
  	req_info.set_id(msg_id);
 	req_info.set_data(data, nLen);
#endif

	AsyncClientCall* call = new AsyncClientCall(task_id_, server_nid_, req_info, cq_, stub_);
	
	return nLen;
}

void AsyncClient::AsyncCompleteRpc()
{
	void* got_tag;
	bool ok = false;
	while(cq_.Next(&got_tag, &ok))
	{
		AbstractAsyncClientCall* call = static_cast<AbstractAsyncClientCall*>(got_tag);
		if(call->callStatus == call->PROCESS)
		{
			GPR_ASSERT(ok);
			
			if (!call->status.ok())
			{
				// 重发
				SendReqAgain(call);
			}

			// 完成后，释放call对象
			delete call;
		}
	}
}
