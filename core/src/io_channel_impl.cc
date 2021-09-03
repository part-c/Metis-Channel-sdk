#include "io_channel_impl.h"
#include "grpc_channel.h"
#include <set>

extern string g_self_nodeid;

string IoChannelImpl::recv_msg(const string& node_id, const string& msg_id, uint64_t msg_len, 
        uint64_t timeout) 
{
  string str(msg_len, 0);
  if(timeout <= 0)
    timeout = 10 * 1000000;
  if(!io_channel_){cout << "io channel is nullptr." << endl; return "";}
  io_channel_->Recv(node_id.c_str(), msg_id.c_str(), &str[0], msg_len, timeout);
  return str;
}

ssize_t IoChannelImpl::send_msg(const string& node_id, const string& msg_id, const string& data, 
        uint64_t msg_len, uint64_t timeout)
{
  if(msg_len > data.size())
        msg_len = data.size();
  if(timeout <= 0)
    timeout = 10 * 1000000;
  if(!io_channel_){cout << "io channel is nullptr." << endl; return 0;}
  return io_channel_->Send(node_id.c_str(), msg_id.c_str(), data.c_str(), msg_len, timeout);
}

IChannel* IoChannelImpl::CreateViaChannel(const NodeInfo& node_info, 
      shared_ptr<ChannelConfig> config, const vector<ViaInfo>& serverInfos, 
      const vector<string>& clientNodeIds, error_callback error_callback) 
{
  shared_ptr<BasicIO> net_io =  nullptr;
  net_io = make_shared<ViaNetIO>(node_info, serverInfos, clientNodeIds, error_callback);
  net_io->SetLogLevel(config->log_level_);
  if (net_io->init(config->task_id_)) 
  { 
    GRpcChannel* grpc_channel = new GRpcChannel(net_io, config, node_info);
    grpc_channel->SetConnectedNodeIDs(clientNodeIds);
    io_channel_ = grpc_channel;

    return io_channel_;
  }
 
  error_callback(node_info.id.c_str(), "", -1, "init io failed!", (void*)"user_data");
  return nullptr;
}

IChannel* IoChannelImpl::CreateIoChannel(const string& node_id, const string &config_str, 
        error_callback error_cb) 
{
  shared_ptr<ChannelConfig> config = make_shared<ChannelConfig>(config_str);

  NodeInfo node_info;
  vector<ViaInfo> serverInfos;
  // 根据nodeid获取数据节点或计算节点或接收结果节点信息
  const Node& node = config->GetNode(node_id);
  // 获取节点信息
  config->CopyNodeInfo(node_info, node);

  // 获取本节点对应的via地址
  string via_name = config->nodeid_to_via_[node_info.id];
  node_info.via_address = config->via_to_address_[via_name];
  if("" == node_info.via_address)
  {
    string strErrMsg = "The service node " + node_info.id + " does not have a VIA address!";
    cout << strErrMsg << endl;
    throw (strErrMsg);
  }
      
  if("" == node.ADDRESS)
  {
    string strErrMsg = "The address corresponding to the " + node_info.id + " node server is empty!";
    cout << strErrMsg << endl;
    throw (strErrMsg);
  }
    
  vector<string> clientNodeIds; 
  config->GetNodeInfos(clientNodeIds, serverInfos, node_id);
  g_self_nodeid = node_id;
  return CreateViaChannel(node_info, config, serverInfos, clientNodeIds, error_cb);
}
