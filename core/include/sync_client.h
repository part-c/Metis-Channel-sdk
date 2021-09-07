// file sync_client.h
#pragma once
#include "base_client.h"
#include "cycle_buffer.h"
#include "simple_buffer.h"
#include <iostream>
#include <mutex>
using grpc::CompletionQueue;
using grpc::ClientWriter;
using namespace std;

/*
	同步客户端
*/
class SyncClient : public BaseClient
{
public:
	SyncClient(const ViaInfo& via_info, const string& taskid);
	// Create a connection between the node where the server is located and VIA to register the interface.
	SyncClient(const NodeInfo& node_info, const string& taskid);
	~SyncClient(){flush_send_buffer();}

	void loop_send();
	ssize_t put_into_send_buffer(const string& msg_id, const char* data, size_t len);
	void flush_send_buffer();
	ssize_t send(const SendRequest& req, int64_t timeout = 100 * 100000);
	ssize_t send(const string& msg_id, const char* data, uint64_t length, int64_t timeout = -1L);
  	// ssize_t recv(const string& id, string& data, int64_t timeout = -1L);

private:
	shared_ptr<cycle_buffer> send_buffer_ = nullptr;
	vector<SendRequest> vec_send_req_;
	std::mutex send_buffer_mtx_;
	std::condition_variable send_buffer_cv_;
};
