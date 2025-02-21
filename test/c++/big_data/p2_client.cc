#include "include/io_channel_impl.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
using namespace std;


//从文件读入到string里
string readFileIntoString(const string& filename)
{
    ifstream ifile(filename);
    //将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while(buf&&ifile.get(ch))
    buf.put(ch);
    //返回与流对象buf关联的字符串
    return buf.str();
}


int main(int argc, char *argv[])
{
    //文件名
    string fn = "config.json";
    string io_config_str = readFileIntoString(fn);
    cout << io_config_str << endl;

    IoChannelImpl io_impl;

    // 启动服务
    string address = "";
    bool is_start_server = false;
    // 创建io
    shared_ptr<BasicIO> io_ = io_impl.CreateChannel("p2", io_config_str, 
            is_start_server);

    cout << "start send to p0========" << endl;
    GRpcChannel channel_(io_);
    int max_time = 1;
    string send_nodeid = "p0";
    string send_msg_id = "0x1111";
    for(int i = 0; i < max_time; ++i)
    {
        send_msg_id = to_string(i);
        string data = "this is send from p2 client, msg_id is ======= " + send_msg_id;
        channel_.Send(send_nodeid, send_msg_id, data);
        usleep(2000);
    }
        
    if(is_start_server)
    {
        io_impl.WaitServer();
    }
    
    return 0;
}

