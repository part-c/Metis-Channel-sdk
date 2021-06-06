#include "io_channel_impl.h"
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
/*
多对一：多个客户端发送，一个服务器接收
*/

bool start_recv(GRpcChannel& channel_)
{
    // 多线程接收
    int max_time = 10000;
    auto recv_f = [&](const string& cid) -> bool {
        // 
        for(int i = 0; i < max_time; ++i)
        {
            string data = "";
            cout << "start recv=========" << endl;
            usleep(3000);
            channel_.Recv(cid, to_string(i), data);
            cout << "recv by nodeid: " << cid << ", data: " << data << endl;
        }
       
        return true;
    };

    vector<string> vec_nid(3);
    vec_nid[0] = "p1";
    vec_nid[1] = "p2";
    vec_nid[2] = "p9";
    
    vector<thread> vec_thread(3);
    cout << "start recv=========" << endl;
    for(int i = 0; i < vec_nid.size(); i++)
    {
        vec_thread[i] = thread(recv_f, vec_nid[i]);
        vec_thread[i].join();
    }
    return true;
}

int main(int argc, char *argv[])
{
    //文件名
    string fn = "config.json";
    string io_config_str = readFileIntoString(fn);
    cout << io_config_str << endl;

    IoChannelImpl io_impl;

    // 启动服务
    string address = "127.0.0.1:11111";
    bool is_start_server = true;
    shared_ptr<BasicIO> io_ = io_impl.CreateChannel("p0", io_config_str, 
            is_start_server, address, nullptr);

    cout << "start sleep!" << endl;
    int64_t sleep_time = 2000;
    if(argc >= 2)
    {
        sleep_time = int64_t(argv[1]);
    }
    // sleep(sleep_time);
    GRpcChannel channel_(io_);
    string data = "";
    while(true)
    {
        // 读到第一条记录开始
        channel_.Recv("p1", "0", data);
        if("" != data)
        {
            break;
        }
        usleep(sleep_time);
    }
    
    start_recv(channel_);

    /*
    cout << "start to call recv========" << endl;
   
    string send_nodeid = "p1";
    string send_msg_id = "0x1111";
    string data = "";
    
    while(true)
    {
        cout << "start sleep!" << endl;
        this_thread::sleep_for(std::chrono::milliseconds(sleep_time/100));
        cout << "start to call recv========" << endl;
        data = "";
        channel_.Recv(send_nodeid, send_msg_id, data);
        cout << "recv from:" << send_nodeid << ", data: " << data << endl;
    }*/
    io_impl.CloseServer();
    if(is_start_server)
    {
        io_impl.Wait_Server();
    }
    
    return 0;
}

