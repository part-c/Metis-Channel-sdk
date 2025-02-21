// file sync_server.h
#pragma once
#include "base_server.h"
#include "config.h"
#include "client_connection.h"
#include <iostream>
#include <unordered_map>
#include <mutex>
#include<chrono>
#include<thread>
#include<map>
#include<queue>

using namespace std;

#include "io_channel.grpc.pb.h"
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
using io_channel::IoChannel;
using io_channel::SendRequest;
using io_channel::RetCode;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

// 同步服务器
class SyncServer: public BaseServer, public IoChannel::Service
{
public:
    bool wait();
    ~SyncServer(){close();}

    SyncServer(const NodeInfo& server_info, map<string, shared_ptr<ClientConnection>>* ptr_client_conn_map);
	Status Send(ServerContext* context, const SendRequest* request, RetCode* response);
    
private:
    std::mutex mtx_;
};
