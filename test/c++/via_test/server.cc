#include "api_manager.h"
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
using namespace std;
//从文件读入到string里
string readFileIntoString(const string& filename)
{
    ifstream ifile(filename);
    if(!ifile.good())
    {
        cout << "file is not exist:" << filename << endl;
        return "";
    }
    //将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while(buf&&ifile.get(ch))
    buf.put(ch);
    //返回与流对象buf关联的字符串
    return buf.str();
}


const string self_nid = "p1";

int main(int argc, char *argv[])
{
   //文件名
    string fn = "config.json";
    string io_config_str = readFileIntoString(fn);
    int64_t timeout = 1000 * 1000000;
    // 启动服务
    APIManager api;
    IChannel* channel = api.create_channel(self_nid, io_config_str);

    string send_data = "hello world, I'm p1.";
    string serNodeId = "p0";
    string send_msg_id = "";

    uint64_t msg_len = send_data.size();
    // recv
    string recv_msg_id = "";
    for(int i = 0; i < 1; ++i)
    {
        string str(msg_len, 0);
        recv_msg_id = to_string(i);
        channel->Recv(serNodeId.c_str(), recv_msg_id.c_str(), &str[0], msg_len, timeout);
        cout << "recv by nodeid: " << serNodeId << ", data: " << str << endl;
    }

    for(int i = 0; i < 1; ++i)
    {
        send_msg_id = to_string(i);
        channel->Send(serNodeId.c_str(), send_msg_id.c_str(), send_data.c_str(), 
            send_data.size(), timeout);
    }
    sleep(10);
    
    return 0;
}

