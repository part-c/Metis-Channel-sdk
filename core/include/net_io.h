#pragma once

#include "base_server.h"
#include "base_client.h"
#include "config.h"
#include "IChannel.h"
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <iomanip>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <functional>
using namespace std;

/**
 * Users only need to include this one header file.
 * 
 * Provides ViaNetIO
 * 
 * Note:
 * 
 * Only the receiving of the server and the sending of the client are totally supported. 
 * 
 * The receiving of the client and the sending of the server are not totally supported.
 */

/**
 * This is the basic class of Network IO.
 */
class BasicIO {
 public:
  virtual ~BasicIO()
  {
#if ASYNC_SERVER
    for(auto& _thread : handle_threads_)
    {
      _thread.detach();
    }
    for(auto& _thread : handle_data_threads_)
    {
      _thread.detach();
    }
#endif

#if ASYNC_CLIENT
    for(auto& _thread : clients_thread_)
    {
      _thread.detach();
    }
#else
    for(auto& _thread : vec_send_thread_)
    {
      _thread.detach();
    }
#endif
    // server_->close();
  };

  BasicIO(const NodeInfo &node_info, const vector<ViaInfo>& server_infos, 
    const vector<string>& client_nodeids,
    error_callback error_callback=nullptr);

 public:
  /**
   * Initialize the client connection.
   */
  virtual void SetLogLevel(uint8_t log_level){gpr_set_log_verbosity((gpr_log_severity)log_level);};
  virtual bool init(const string& taskid) = 0;
  virtual ssize_t recv(const string& remote_nodeid, const char* id, char* data, 
      uint64_t length, int64_t timeout) = 0;
  virtual ssize_t send(const string& remote_nodeid, const char* id, const char* data, 
      uint64_t length, int64_t timeout) = 0;

 protected:
  NodeInfo node_info_;
  vector<ViaInfo> via_server_infos_;
  vector<string> client_nodeids_;

#if ASYNC_CLIENT
  vector<thread> clients_thread_;
#else
  vector<thread> vec_send_thread_;
#endif

  map<string, shared_ptr<BaseClient>> nid_to_server_map_;
  map<string, shared_ptr<ClientConnection>> client_conn_map;
  error_callback handler;

#if ASYNC_SERVER
  vector<std::thread> handle_threads_;
  vector<std::thread> handle_data_threads_;
#endif
  shared_ptr<BaseServer> server_ = nullptr;
  shared_ptr<BaseClient> via_client_ = nullptr;
};


/**
 * Via Net IO.
 */
class ViaNetIO : public BasicIO {
 public:
  using BasicIO::BasicIO;
  virtual ~ViaNetIO(){}  
  bool StartServer(const string& taskid, const NodeInfo& server_info,
       map<string, shared_ptr<ClientConnection>>* ptr_client_conn_map);

  bool init(const string& taskid);
  ssize_t recv(const string& remote_nodeid, const char* id, char* data, uint64_t length, 
      int64_t timeout=-1);
  ssize_t send(const string& remote_nodeid, const char* id, const char* data, uint64_t length, 
      int64_t timeout=-1);
};

