/*
 * @Author: GanShuang
 * @Date: 2020-05-26 10:38:40
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-26 17:34:37
 * @FilePath: /myWebServer-master/Util.h
 */ 
#pragma once

#include <functional>
#include <netinet/in.h>
#include <string>
#include <cstdlib>

void handle_for_sigpipe();
int setnonblocking(int fd);
int socket_bind_listen(int port, sockaddr_in &address);
void shutDownWR(int fd);