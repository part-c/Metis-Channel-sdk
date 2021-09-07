// file sync_client.cc
#include "sync_client.h"
#include <thread>
#include <chrono>   
using namespace chrono;
extern string g_self_nodeid;
SyncClient::SyncClient(const ViaInfo& via_info, const string& taskid):
  BaseClient(via_info, taskid)
{
  vec_send_req_.clear();
  send_buffer_ = make_shared<cycle_buffer>(1024 * 1024 * 128);
}

SyncClient::SyncClient(const NodeInfo& node_info, const string& taskid):
  BaseClient(node_info, taskid){}

ssize_t SyncClient::send(const SendRequest& req, int64_t timeout) 
{
  RetCode ret_code;
  auto start_time = system_clock::now();
  auto end_time   = start_time;
  int64_t elapsed = 0;
  system_clock::time_point deadline = start_time + std::chrono::milliseconds(timeout);
  do {
    grpc::ClientContext context;
    // 添加注册到via的参数
    // context.AddMetadata("node_id", remote_nodeid);
    context.AddMetadata("task_id", task_id_);
    context.AddMetadata("party_id", server_nid_);

    // 设置阻塞等待和超时时间
    context.set_wait_for_ready(true);
    context.set_deadline(deadline);
    Status status = stub_->Send(&context, req, &ret_code);

    if (status.ok()) 
    {
      break;
    } 
    else 
    {
      end_time = system_clock::now();
      elapsed = duration_cast<duration<int64_t, std::milli>>(end_time - start_time).count();

      if(elapsed >= timeout)
      {
          gpr_log(GPR_ERROR, "Send request timeout:%ld.", timeout);
          return 0;
      }
    }
  } while(true);
	
	return 0;
}

void SyncClient::loop_send()
{
  while (true) 
  {
    std::unique_lock<std::mutex> lck(send_buffer_mtx_);
    send_buffer_cv_.wait(lck, [&](){
      if (vec_send_req_.size() > 0) 
      {
        return true;
      }
      return false;
    });

    for(const SendRequest& seq : vec_send_req_)
    {
      send(seq);
    }
    
    vec_send_req_.clear();
    send_buffer_cv_.notify_all();
  }
}

void SyncClient::flush_send_buffer() 
{
  vec_send_req_.clear();
}

ssize_t SyncClient::put_into_send_buffer(const string& msg_id, const char* data, size_t len) 
{
  std::unique_lock<std::mutex> lck(send_buffer_mtx_);
  SendRequest req_info;
  req_info.set_nodeid(g_self_nodeid);
  req_info.set_id(msg_id);
  req_info.set_data(data, len);
  vec_send_req_.push_back(req_info);
  send_buffer_cv_.notify_all();

  return len;
}

ssize_t SyncClient::send(const string& msg_id, const char* data, uint64_t length, int64_t timeout)
{
  return put_into_send_buffer(msg_id, data, length);
}

/*
ssize_t SyncClient::send(const string& msg_id, const char* data, const size_t nLen, int64_t timeout)
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
  
  RetCode ret_code;

  auto start_time = system_clock::now();
  auto end_time   = start_time;
  int64_t elapsed = 0;
  system_clock::time_point deadline = start_time + std::chrono::milliseconds(timeout);
  do {
    grpc::ClientContext context;
    // 添加注册到via的参数
    // context.AddMetadata("node_id", remote_nodeid);
    context.AddMetadata("task_id", task_id_);
    context.AddMetadata("party_id", server_nid_);

    // 设置阻塞等待和超时时间
    context.set_wait_for_ready(true);
    context.set_deadline(deadline);
    Status status = stub_->Send(&context, req_info, &ret_code);
    if (status.ok()) 
    {
      break;
    } 
    else 
    {
      end_time = system_clock::now();
      elapsed = duration_cast<duration<int64_t, std::milli>>(end_time - start_time).count();

      if(elapsed >= timeout)
      {
          gpr_log(GPR_ERROR, "Send request timeout:%ld.", timeout);
          return 0;
      }
    }
  } while(true);
	
	return nLen;
}
*/
