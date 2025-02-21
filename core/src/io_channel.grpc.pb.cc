// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: io_channel.proto

#include "io_channel.pb.h"
#include "io_channel.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace io_channel {

static const char* IoChannel_method_names[] = {
  "/io_channel.IoChannel/Send",
};

std::unique_ptr< IoChannel::Stub> IoChannel::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< IoChannel::Stub> stub(new IoChannel::Stub(channel, options));
  return stub;
}

IoChannel::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_Send_(IoChannel_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status IoChannel::Stub::Send(::grpc::ClientContext* context, const ::io_channel::SendRequest& request, ::io_channel::RetCode* response) {
  return ::grpc::internal::BlockingUnaryCall< ::io_channel::SendRequest, ::io_channel::RetCode, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Send_, context, request, response);
}

void IoChannel::Stub::async::Send(::grpc::ClientContext* context, const ::io_channel::SendRequest* request, ::io_channel::RetCode* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::io_channel::SendRequest, ::io_channel::RetCode, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Send_, context, request, response, std::move(f));
}

void IoChannel::Stub::async::Send(::grpc::ClientContext* context, const ::io_channel::SendRequest* request, ::io_channel::RetCode* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Send_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::io_channel::RetCode>* IoChannel::Stub::PrepareAsyncSendRaw(::grpc::ClientContext* context, const ::io_channel::SendRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::io_channel::RetCode, ::io_channel::SendRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Send_, context, request);
}

::grpc::ClientAsyncResponseReader< ::io_channel::RetCode>* IoChannel::Stub::AsyncSendRaw(::grpc::ClientContext* context, const ::io_channel::SendRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncSendRaw(context, request, cq);
  result->StartCall();
  return result;
}

IoChannel::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      IoChannel_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< IoChannel::Service, ::io_channel::SendRequest, ::io_channel::RetCode, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](IoChannel::Service* service,
             ::grpc::ServerContext* ctx,
             const ::io_channel::SendRequest* req,
             ::io_channel::RetCode* resp) {
               return service->Send(ctx, req, resp);
             }, this)));
}

IoChannel::Service::~Service() {
}

::grpc::Status IoChannel::Service::Send(::grpc::ServerContext* context, const ::io_channel::SendRequest* request, ::io_channel::RetCode* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace io_channel

