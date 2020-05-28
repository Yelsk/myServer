/*
 * @Author: GanShuang
 * @Date: 2020-05-27 16:51:56
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-27 20:10:00
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
    LOG_INFO("connect pool success");
    Log::get_instance()->flush();
    EventLoop mainloop;
    WebServer myserver(&mainloop, sqlpool, path, threadNum, port);
    myserver.start();
    mainloop.loop();
    return 0;
}