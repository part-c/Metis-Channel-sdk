
// file base_client.h
#pragma once
#include "config.h"
#include <string>
#include <functional>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <cmath>
#include "assert.h"
#include "unistd.h"

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <thread>

#include "io_channel.grpc.pb.h"
#include "via.grpc.pb.h"
using grpc::Status;
using grpc::Channel;
using grpc::ClientContext;
using io_channel::IoChannel;
using io_channel::SendRequest;
using io_channel::RetCode;

// Register
using via::VIAService;
using via::SignupReq;
using via::Boolean;
using namespace std;

class BaseClient
{
private:
    bool MakeCredentials(const ViaInfo& via_info);
public:
    BaseClient(const ViaInfo& via_info, const string& taskid);
    // Create a connection between the node where the server is located and VIA to register the interface.
    BaseClient(const NodeInfo& node_info, const string& taskid);
    // Register the interface where the server node resides to via
    bool SignUpToVia(const NodeInfo& server_info);

    /*
    virtual ssize_t send(const string& msg_id, const char* data, 
        const size_t nLen, int64_t timeout = -1L) = 0;
    */

    virtual ssize_t send(const string& msg_id, const char* data, uint64_t length, int64_t timeout = -1L) = 0;

    virtual ~BaseClient(){}
public:
	string task_id_;
    string server_nid_;
protected:
    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<IoChannel::Stub> stub_ = nullptr;
    std::unique_ptr<VIAService::Stub> via_stub_ = nullptr;
    shared_ptr<grpc::ChannelCredentials> creds_ = nullptr;
};

