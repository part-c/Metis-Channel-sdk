#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
using namespace std;

#include <rapidjson/document.h>
using rapidjson::Document;
extern string g_self_nodeid;
struct Node 
{
  string DESC;
  string NODE_ID;
  string NAME;
  string ADDRESS;
  string VIA;
  string CA_CERT_PATH;

#if(1 == SSL_TYPE)
  string SERVER_KEY_PATH;
  string SERVER_CERT_PATH;
  string CLIENT_KEY_PATH;
  string CLIENT_CERT_PATH;
#elif(2 == SSL_TYPE)
  string SERVER_SIGN_KEY_PATH;
  string SERVER_SIGN_CERT_PATH;
  string SERVER_ENC_KEY_PATH;
  string SERVER_ENC_CERT_PATH;
  string CLIENT_SIGN_KEY_PATH;
  string CLIENT_SIGN_CERT_PATH;
  string CLIENT_ENC_KEY_PATH;
  string CLIENT_ENC_CERT_PATH;
#endif

  public:
    void copy_from(const Node& node) 
    {
      DESC.assign(node.DESC);
      NODE_ID.assign(node.NODE_ID);
      NAME.assign(node.NAME);
      ADDRESS.assign(node.ADDRESS);
      VIA.assign(node.VIA);
      CA_CERT_PATH.assign(node.CA_CERT_PATH);

      #if(1==SSL_TYPE)
      {
        SERVER_KEY_PATH.assign(node.SERVER_KEY_PATH);
        SERVER_CERT_PATH.assign(node.SERVER_CERT_PATH);
        CLIENT_KEY_PATH.assign(node.CLIENT_KEY_PATH);
        CLIENT_CERT_PATH.assign(node.CLIENT_CERT_PATH);
      }  
      #elif(2 == SSL_TYPE)
      {
        SERVER_SIGN_KEY_PATH.assign(node.SERVER_SIGN_KEY_PATH);
        SERVER_SIGN_CERT_PATH.assign(node.SERVER_SIGN_CERT_PATH);
        SERVER_ENC_KEY_PATH.assign(node.SERVER_ENC_KEY_PATH);
        SERVER_ENC_CERT_PATH.assign(node.SERVER_ENC_CERT_PATH);
        CLIENT_SIGN_KEY_PATH.assign(node.CLIENT_SIGN_KEY_PATH);
        CLIENT_SIGN_CERT_PATH.assign(node.CLIENT_SIGN_CERT_PATH);
        CLIENT_ENC_KEY_PATH.assign(node.CLIENT_ENC_KEY_PATH);
        CLIENT_ENC_CERT_PATH.assign(node.CLIENT_ENC_CERT_PATH);
      } 
      #endif
    }
};
class DataNodeConfig {
  public:
    vector<Node> P;
    std::string to_string();
};

class ResultNodeConfig {
  public:
    vector<Node> P;
    std::string to_string();
};
class NodeInfoConfig {
  public:
    Node node_;
    std::string to_string();
};

class ComputeNodeConfig {
 public:
  ComputeNodeConfig();
  std::string to_string();
  int GetNodeIndex(const string& node_id);

 public:
  vector<Node> P;
};

struct NodeInfo {
  string id;
  string address;
  string via_address;
  string ca_cert_path_;
#if(1==SSL_TYPE) 
  string server_key_path_;
  string server_cert_path_;
  // Used as a client certificate to invoke the VIA server registration interface
  string client_key_path_;
  string client_cert_path_;
#elif(2==SSL_TYPE) 
  string server_sign_key_path_;
  string server_sign_cert_path_;
  string server_enc_key_path_;
  string server_enc_cert_path_;
  // Used as a client certificate to invoke the VIA server registration interface
  string client_sign_key_path_;
  string client_sign_cert_path_;
  string client_enc_key_path_;
  string client_enc_cert_path_;
#endif

  NodeInfo() = default;
  NodeInfo(const string& node_id, const string& addr, const string& via_addr) 
    : id(node_id), address(addr), via_address(via_addr){}
};

struct ViaInfo {
  string id;
  string address;
  string via;
  string server_cert_path_;

  #if(1==SSL_TYPE)  
    string client_key_path_;
    string client_cert_path_;
  #elif(2==SSL_TYPE)  
    string client_sign_key_path_;
    string client_sign_cert_path_;
    string client_enc_key_path_;
    string client_enc_cert_path_;
  #endif

  ViaInfo() = default;
  ViaInfo(const string& node_id, const string& addr, const string& via_) 
    : id(node_id), address(addr), via(via_){}
};

enum NODE_TYPE {
  NODE_TYPE_INVALID,
  NODE_TYPE_DATA,
  NODE_TYPE_COMPUTE,
  NODE_TYPE_RESULT
};

class ChannelConfig {
 public:
  ChannelConfig() {}
  ChannelConfig(const string& node_id, const string& config_json);
  ChannelConfig(const string& config_json); // use NODE_ID in config_json
  vector<NODE_TYPE> GetNodeType(const string& node_id);
  const Node& GetNode(const string& node_id);

  void CopyNodeInfo(NodeInfo& node_info, const Node& nodeInfo);
  bool isNodeType(const vector<NODE_TYPE>& vec_node_types, const NODE_TYPE nodeType);
  bool GetNodeInfos(vector<string>& clientNodeIds, vector<ViaInfo>& serverInfos, const string& node_id);

 private:
  bool load(const string& node_id, const string& config_file);
  bool parse(Document& doc);
  bool parse_node_info(Document& doc, bool pass_via=true);
  bool parse_data(Document& doc);
  bool parse_compute(Document& doc);
  bool parse_result(Document& doc);

 public:
  void fmt_print();

 public:
  string node_id_ = ""; // NOT USE AT PRESENT
  bool pass_via_ = "true"; 
  string task_id_ = "";
  string root_cert_ = "";
  uint8_t log_level_ = 0; // 0:Debug, 1:Info, 2: Error
  NODE_TYPE node_type_ = NODE_TYPE_INVALID;
  map<string, NodeInfoConfig> node_info_config_;
  map<string, string> nodeid_to_via_;
  map<string, string> via_to_address_;
  vector<string> data_nodes_;
  map<string, int> compute_nodes_;
  vector<string> result_nodes_;
  DataNodeConfig data_config_;
  ComputeNodeConfig compute_config_;
  ResultNodeConfig result_config_;
};

static string get_file_contents(const string& fpath)
{
    ifstream ifile(fpath);
    if(!ifile.good())
    {
        cerr << "file is not exist:" << fpath << endl;
        return "";
    }
    ostringstream buf;
    char ch;
    while(buf&&ifile.get(ch))
    buf.put(ch);
    return buf.str();
}


