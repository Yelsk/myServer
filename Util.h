/*
 * @Author: GanShuang
 * @Date: 2020-05-26 10:38:40
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-27 11:36:20
 * @FilePath: /myWebServer-master/Util.h
 */ 
#pragma once

#include <netinet/in.h>
#include <string>
#include <cstdlib>

void handle_for_sigpipe();
int setnonblocking(int fd);
int socket_bind_listen(int port);
void shutdownwr(int fd);
void setsocketnodelay(int fd);