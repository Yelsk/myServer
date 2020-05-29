/*
 * @Author: GanShuang
 * @Date: 2020-05-27 16:51:56
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-29 20:13:11
 * @FilePath: /myWebServer-master/main.cc
 */ 

#include <string>
#include "EventLoop.h"
#include "WebServer.h"
#include "Log.h"

int main(int argc, char *argv[]) {
    int threadNum = 4;
    int port = 8888;
    std::string path = "./WEB/";
    Log::get_instance()->init("ServerLog", 2000, 800000, 100);
    SQLPool *sqlpool = SQLPool::get_instance();
    sqlpool->init("localhost", "gan", "123", "gandb", 3306, 8);
    EventLoop mainloop;
    WebServer myserver(&mainloop, sqlpool, path, threadNum, port);
    myserver.start();
    mainloop.loop();
    return 0;
}