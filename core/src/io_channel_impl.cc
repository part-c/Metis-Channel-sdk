#include "io_channel_impl.h"
#include "net_io.h"
#include "common.h"
#include <set>

const char* encode_string(const string& str) {
  int len = sizeof(int) + str.size();
  char* res = new char[len];
  memcpy(res, &len, sizeof(int));
  memcpy(res + sizeof(int), str.data(), str.size());
  return res;
}
string decode_string(const char* str) {
  int len = (*(int*)str);
  string res;
  res.resize(len - sizeof(int));
  memcpy(&res[0], str + sizeof(int), len - sizeof(int));
  return res;
}
const char* encode_vector(const vector<string>& vec) {
  int len = sizeof(int);
  for (int i = 0; i < vec.size(); i++) {
    len += sizeof(int) + vec[i].size();
  }
  char* res = new char[len];
  int offset = 0;
  int num = vec.size();
  memcpy(res + offset, &num, sizeof(int));
  offset += sizeof(int);
  for (int i = 0; i < vec.size(); i++) {
    int len2 = sizeof(int) + vec[i].size();
    memcpy(res + offset, &len2, sizeof(int));
    memcpy(res + offset + sizeof(int), vec[i].data(), vec[i].size());
    offset += len2;
  }
  return res;
}
vector<string> decode_vector(const char* str) {
  int num = *(int*)str;
  int offset = sizeof(int);
  vector<string> res;
  res.resize(num);
  for (int i = 0; i < num; i++){
    int len2 = (*(int*)(str + offset));
    string s(str + offset + sizeof(int), len2 - sizeof(int));
    res[i] = s;
    offset += len2;
  }
  return res;
}
const char* encode_map(const map<string, int>& m) {
  int len = sizeof(int);
  for (auto iter = m.begin(); iter != m.end(); iter++) {
    len += sizeof(int) + iter->first.size() + sizeof(int);
  }
  char* res = new char[len];
  int offset = 0;
  int num = m.size();
  memcpy(res + offset, &num, sizeof(int));
  offset += sizeof(int);
  for (auto iter = m.begin(); iter != m.end(); iter++) {
    int len2 = sizeof(int) + iter->first.size();
    memcpy(res + offset, &len2, sizeof(int));
    memcpy(res + offset + sizeof(int), iter->first.data(), iter->first.size());
    offset += len2;
    memcpy(res + offset, &iter->second, sizeof(int));
    offset += sizeof(int);
  }
  return res;
}
map<string, int> decode_map(const char* str) {
  int num = *(int*)str;
  int offset = sizeof(int);
  map<string, int> res;
  for (int i = 0; i < num; i++) {
    int len2 = *(int*)(str + offset);
    string s(str + offset + sizeof(int), len2 - sizeof(int));
    offset += len2;
    int val = *(int*)(str + offset);
    offset += sizeof(int);
    res.insert(std::pair<string, int>(s, val));
  }
  return res;
}

static shared_ptr<IoChannelImpl> gs_impl = make_shared<IoChannelImpl>();

bool IoChannelImpl::StartServer(const string& server_addr)
{
  auto start_server_f = [&](const string& _addr) -> bool {
    // // 创建io通道
    server_ = make_shared<IoChannelServer>(_addr);
    return true;
  };
  
  thread server_thread = thread(start_server_f, server_addr);
  server_thread.join();
  return true;
}

shared_ptr<IChannel> IoChannelImpl::CreateViaChannel(const NodeInfo& node_info, 
      shared_ptr<ChannelConfig> config, const vector<ViaInfo>& serverInfos, 
      map<string, string>* share_data_map_, error_callback error_callback) 
{      
  shared_ptr<BasicIO> net_io =  nullptr;
  net_io = make_shared<ViaNetIO>(node_info, serverInfos, share_data_map_, error_callback);
  if (net_io->init(config->task_id_)) 
  {
    shared_ptr<GRpcChannel> grpc_channel = make_shared<GRpcChannel>(net_io, config, node_info);
    return std::dynamic_pointer_cast<IChannel>(grpc_channel);
  }
 
  error_callback(node_info.id.c_str(), "", -1, "init io failed!", (void*)"user_data");
  return nullptr;
}

shared_ptr<IChannel> CreateChannel(const string& node_id, const string &config_str, 
      const bool& is_start_server, error_callback error_cb) 
{
  return gs_impl->CreateIoChannel(node_id, config_str, is_start_server, error_cb);
}

shared_ptr<IChannel> IoChannelImpl::CreateIoChannel(const string& node_id, const string &config_str, 
      const bool& is_start_server, error_callback error_cb) 
{
  shared_ptr<ChannelConfig> config = make_shared<ChannelConfig>(config_str);
  NodeInfo node_info;
  vector<ViaInfo> serverInfos;
  // 根据nodeid获取数据节点或计算节点或接收结果节点信息
  const Node& node = config->GetNode(node_id);

  vector<NODE_TYPE> node_types = config->GetNodeType(node_id);
  // 获取节点信息
  CopyNodeInfo(node_info, node);

  // 获取本节点对应的via地址
  string via_name = config->nodeid_to_via_[node_info.id];
  node_info.via_address = config->via_to_address_[via_name];

  // 启动服务器
  if(is_start_server)
  {
    if("" == node_info.via_address)
      throw ("The service node " + node_info.id + " does not have a VIA address!");

    if("" == node.ADDRESS)
      throw ("The address corresponding to the " + node_info.id + " node server is empty!");

    cout << "start server, node.ADDRESS: " << node.ADDRESS << endl;
    StartServer(node.ADDRESS);
  }
  
  set<string> nodeid_set;
  if(isNodeType(node_types, NODE_TYPE_DATA) || isNodeType(node_types, NODE_TYPE_COMPUTE))
  {
    // 遍历计算节点
    for (int i = 0; i < config->compute_config_.P.size(); i++) 
    {
      // 获取计算节点的nodeid
      string nid = config->compute_config_.P[i].NODE_ID;
      string via = config->nodeid_to_via_[nid];
      if (node_id != nid && nodeid_set.find(nid) == nodeid_set.end()) 
      {
        ViaInfo viaTmp;
        viaTmp.id = nid;
        viaTmp.via = via;
        viaTmp.address = config->via_to_address_[via];
        cout << "id: " << nid << ", via: " << viaTmp.via << ", address: " << viaTmp.address << endl;
        serverInfos.push_back(viaTmp);
        // 保存除自身外的计算节点
        nodeid_set.insert(nid);
      }
    }
  }
  
  if(isNodeType(node_types, NODE_TYPE_COMPUTE))
  {
    cout << "is compute node!" << endl;
    // 遍历结果接收节点
    for (int i = 0; i < config->result_config_.P.size(); i++) 
    {
      cout << "handle compute node" << endl;
      string nid = config->result_config_.P[i].NODE_ID;
      string via = config->nodeid_to_via_[nid];
      if (node_id != nid && nodeid_set.find(nid) == nodeid_set.end()) 
      {
        ViaInfo viaTmp;
        viaTmp.id = nid;
        // 节点所在via
        viaTmp.via = via;
        // via信息
        viaTmp.address = config->via_to_address_[viaTmp.via];
        cout << "id: " << nid << ", via: " << viaTmp.via << ", address: " << viaTmp.address << endl;
        serverInfos.push_back(viaTmp);
        nodeid_set.insert(nid);
      }
    }  
  }

  string strNodeInfo = "address: " + node_info.address + ", id:" + node_info.id;
  cout << "node_info=========:" << strNodeInfo << endl;

  string strServerInfo = "";
  string nodeid = "nodeid: ";
  string via = "via: ";
  string address = "address: ";
  for(int i=0; i < serverInfos.size(); ++i)
  {
      ViaInfo tmp = serverInfos[i];
      nodeid = nodeid + " " + tmp.id;
      via = via + " " + tmp.via;
      address = address + " \n" + tmp.address;
  }
  strServerInfo = nodeid + ", " + via + ", " + address;
  cout << "server info=========:" << strServerInfo << endl;
  // 服务器模型, 设置共享内存
  map<string, string>* share_data_map_ = nullptr;
  
  if(server_)
  {
    share_data_map_ = &(server_->get_data_map());
  }
  
  return CreateViaChannel(node_info, config, serverInfos, share_data_map_, error_cb);
}

// GRpcChannel
GRpcChannel::~GRpcChannel() {
  delete []p_node_id_;
  delete []p_data_nodes_;
  delete []p_computation_nodes_;
  delete []p_result_nodes_;
  delete []p_connected_nodes_;
}

ssize_t GRpcChannel::Recv(const char* node_id, const char* id, char* data, uint64_t length, int64_t timeout) {
  // return _net_io->recv(node_id, data, get_binary_string(id), timeout); 
  if(nullptr == _net_io){cout << "create io failed!" << endl; return 0;}
  if(nullptr == data){cout << "data is nullptr!" << endl; return 0;}
  string strData = "";
  uint16_t nLen =  _net_io->recv(node_id, strData, id, timeout);
  memcpy(data, strData.c_str(), length);
  return nLen;
}

ssize_t GRpcChannel::Send(const char* node_id, const char* id, const char* data, uint64_t length, int64_t timeout) {
  // return _net_io->send(node_id, data, get_binary_string(id), timeout);
  if(nullptr == _net_io){cout << "create io failed!" << endl; return 0;}
  return _net_io->send(node_id, data, id, timeout);
}

/*
const char* GRpcChannel::GetCurrentVia()
{
  return self_node_info_.via_address.c_str();
}

const char* GRpcChannel::GetCurrentAddress()
{
  return self_node_info_.address.c_str();
}

const char* GRpcChannel::GetTaskId()
{
  return channel_config_->task_id_.c_str();
}
*/

const char* GRpcChannel::GetDataNodeIDs()
{
  if (p_data_nodes_ == nullptr) {
    p_data_nodes_ = encode_vector(channel_config_->data_nodes_);
  }
  return p_data_nodes_;
}

const char* GRpcChannel::GetComputationNodeIDs()
{
  if (p_computation_nodes_ == nullptr) {
    p_computation_nodes_ = encode_map(channel_config_->compute_nodes_);
  }
  return p_computation_nodes_;
}

const char* GRpcChannel::GetResultNodeIDs()
{
  if (p_result_nodes_ == nullptr) {
    p_result_nodes_ = encode_vector(channel_config_->result_nodes_);
  }
  return p_result_nodes_;
}

const char* GRpcChannel::GetCurrentNodeID()
{
  if (p_node_id_ == nullptr) {
    p_node_id_ = encode_string(self_node_info_.id);
  }
  return p_node_id_;
}

const char* GRpcChannel::GetConnectedNodeIDs()
{
  if (p_connected_nodes_ == nullptr) {
    p_connected_nodes_ = encode_vector(_net_io->get_connected_nodeids());
  }
  return p_connected_nodes_;
}

