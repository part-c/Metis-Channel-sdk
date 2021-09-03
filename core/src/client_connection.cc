// file client_connection.cc
#include "client_connection.h"
#include <thread>
#include <chrono>   
using namespace chrono;

#if USE_BUFFER
ClientConnection::ClientConnection(const string& nodeid) 
{
  nodeid_ = nodeid;
  buffer_ = make_shared<cycle_buffer>(1024 * 1024 * 10);
}

ssize_t ClientConnection::recv(const string& id, char* data, uint64_t length, int64_t timeout) 
{
  if (timeout < 0)
    timeout = 10 * 1000000;

  int64_t elapsed = 0;
  auto beg = system_clock::now();
  ssize_t ret = 0;
  bool retry = false;
  do {
    auto end = system_clock::now();
    elapsed = duration_cast<duration<int64_t, std::milli>>(end - beg).count();
  
    if (elapsed > timeout) {
      return 0; // timeout
    }

    if (retry)
      std::this_thread::yield();

    unique_lock<mutex> lck(mapbuffer_mtx_);

    auto iter = mapbuffer_.find(id);
    if (iter != mapbuffer_.end()) { // got id
      if (iter->second->can_read(length)) { // got data
        ret = iter->second->read(data, length);
        return ret;
      }
      //std::this_thread::yield();
    }
    
    if (!buffer_->can_read()) {
      // cout << "can not find data id:" << id <<  endl;
      continue;
    }
    string tmp_id;
    string tmp_data;
    ssize_t ret = buffer_->read(tmp_id, tmp_data); 

    if (tmp_id == id) 
    {
      if (tmp_data.size() != length) 
      {
        cout << "get data length:" << tmp_data.size() << " expected:" << length << endl;
      }
      memcpy(data, tmp_data.data(), tmp_data.size());
      return tmp_data.size();
    }

    if (mapbuffer_.find(tmp_id) == mapbuffer_.end()) {
      mapbuffer_[tmp_id] = make_shared<cycle_buffer>(1024 * 100);
    }

    // write the real data
    mapbuffer_[tmp_id]->write(tmp_data.data(), tmp_data.size());
  } while (true);

  return 0;
}

#else
ClientConnection::ClientConnection(const string& nodeid):nodeid_(nodeid){}

bool ClientConnection::write(const string& msgid, const string& data)
{
  std::unique_lock<std::mutex> lck(mutex_);
  auto iter = map_queue_.find(msgid);
  // cout << "ClientConnection::write, nodeid:" << nodeid_ << ",msgid:" << msgid << 
  //   ", data.size:" << data.size() << endl;

  if(iter == map_queue_.end())
  {
    shared_ptr<queue<string>> ptr_data_queue_ = make_shared<queue<string>>();
    ptr_data_queue_->push(data);
    map_queue_.insert(std::pair<string,  shared_ptr<queue<string>>>(msgid, ptr_data_queue_));
  }
  else
  {
    iter->second->push(data);
  }
  cv_.notify_all();
  return true;
}

ssize_t ClientConnection::recv(const string& msgid, char* data, uint64_t length, int64_t timeout)
{
  if (timeout < 0)
    timeout = 10 * 1000000;
  
  auto beg = system_clock::now();
  int64_t elapsed = 0;
  int64_t remain_time = timeout;
  do {
    std::unique_lock<std::mutex> lck(mutex_);
    auto iter = map_queue_.find(msgid);
    if(iter != map_queue_.end() && !iter->second->empty())
    {
      const string& strData = iter->second->front();
      uint64_t nSize = strData.size();
      memcpy(data, strData.data(), nSize);
      iter->second->pop();
      return nSize;
    }

    auto end = system_clock::now();
    elapsed = duration_cast<duration<int64_t, std::milli>>(end - beg).count();
    remain_time = timeout - elapsed;

    if(std::cv_status::timeout == cv_.wait_for(lck, std::chrono::milliseconds(remain_time)))
    {
      return 0;
    }
  } while(true);
  
  return 0;
}
#endif