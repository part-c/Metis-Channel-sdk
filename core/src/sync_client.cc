// file sync_client.cc
#include "sync_client.h"
#include <thread>
#include <chrono>   
using namespace chrono;
extern string g_self_nodeid;
SyncClient::SyncClient(const ViaInfo& via_info, const string& taskid):
  BaseClient(via_info, taskid)
{
  send_buffer_ = make_shared<cycle_buffer>(1024 * 1024 * 128);
}

SyncClient::SyncClient(const NodeInfo& node_info, const string& taskid):
  BaseClient(node_info, taskid){}

ssize_t SyncClient::send(const char* data, size_t len, int64_t timeout) 
{
  if (len > 1024 * 1024 * 100) 
  {
    gpr_log(GPR_ERROR, "client will send %ld B, >100M!.", len);
  }

  ssize_t n = 0;
  // n = writen(fd_, data, len);
  SendRequest req_info;
  // 发送客户端的nodeid到服务器
  req_info.set_nodeid(g_self_nodeid);

#if USE_BUFFER
  req_info.set_data(data, len);
#else
  // req_info.set_id(msg_id);
  // req_info.set_data(data, nLen);
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
	
	return len;
}

void SyncClient::loop_send()
{
  while (true) 
  {
    char *buffer = nullptr;
    int n = 0;
    {
      // bool stop_send = false;
      std::unique_lock<std::mutex> lck(send_buffer_mtx_);
      send_buffer_cv_.wait(lck, [&](){
        if (send_buffer_->size() > 0) 
        {
          return true;
        }
        return false;
      });

      // if (stop_send) 
      // {
      //   break;
      // }
      n = send_buffer_->size();
      buffer = new char[n];
      send_buffer_->read(buffer, n);
    }
    {
      ssize_t ret = send(buffer, n);
      if (ret != n) 
      {
        gpr_log(GPR_ERROR, "loop_send: send data error.");
      }
      delete []buffer;
    }
  }
}

void SyncClient::flush_send_buffer() {
  int remain_size = send_buffer_->size();
  if (remain_size > 0) 
  {
    char* buffer = new char[remain_size];
    send_buffer_->read(buffer, remain_size);
    ssize_t ret = send(buffer, remain_size);
    if (ret != remain_size) 
    {
      gpr_log(GPR_ERROR, "flush_send_buffer: send data error.");
    }
    delete []buffer;
  }
}

ssize_t SyncClient::put_into_send_buffer(const char* data, size_t len, int64_t timeout) 
{
  std::unique_lock<std::mutex> lck(send_buffer_mtx_);
  ssize_t ret = send_buffer_->write(data, len);
  send_buffer_cv_.notify_all();
  return ret;
}

ssize_t SyncClient::send(const string& msg_id, const char* data, uint64_t length, int64_t timeout)
{
  simple_buffer buffer(msg_id, data, length);
  return put_into_send_buffer((const char*)buffer.data(), buffer.len(), timeout);
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
