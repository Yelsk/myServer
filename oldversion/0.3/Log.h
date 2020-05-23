/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-23 17:45:40
 * @FilePath: /myWebServer-master/oldversion/0.3/Log.h
 */ 

#pragma once

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdarg.h>
#include "BlockQueue.h"

// #define LOG_ENABLE
#define LOG_DISABLE

using namespace std;

class Log
{
public:
    static Log *get_instance()
    {
        static Log instance;
        return &instance;
    }

    static void *flush_log_thread(void *arg)
    {
        Log::get_instance()->async_write_log();
    }

    bool init(const char *file_name, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush();

private:
    Log();
    virtual ~Log();
    
    void *async_write_log();

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    char *m_buffer;
    int m_split_lines;  //日志最大行数
    int m_log_buf_size;  //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;  //记录当前是哪一天
    FILE *m_fp;  //打开log的文件指针
    BlockQueue<string>  *m_log_queue;
    MutexLock m_mutex;
};

#ifdef LOG_ENABLE

#define LOG_DEBUG(format, ...) Log::get_instance()->write_log(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::get_instance()->write_log(1, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Log::get_instance()->write_log(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::get_instance()->write_log(3, format, ##__VA_ARGS__)

#endif

#ifdef LOG_DISABLE

#define LOG_DEBUG(format, ...) {}
#define LOG_INFO(format, ...) {}
#define LOG_WARN(format, ...) {}
#define LOG_ERROR(format, ...) {}

#endif