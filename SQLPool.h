/*
 * @Author: GanShuang
 * @Date: 2020-05-22 10:03:18
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-02 20:18:00
 * @FilePath: /myWebServer-master/SQLPool.h
 */ 

#pragma once

#include <mysql/mysql.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <string>
#include <list>
#include "Log.h"
#include "MutexLock.h"
#include "Semaphore.h"

class SQLPool
{
public:
    MYSQL *GetConnection();
    bool ReleaseConnection(MYSQL *conn);
    int GetFreeConn();
    void DestroyPool();

    static SQLPool *get_instance()
    {
        static SQLPool sqlinstance;
        return &sqlinstance;
    }
    
    void init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, unsigned int MaxConn); 

    SQLPool();
    ~SQLPool();
private:
    uint32_t m_maxConn;
    uint32_t m_curConn;
    uint32_t m_freeConn;

private:
    MutexLock m_locker;
    Semaphore m_sem;
    std::list<MYSQL *> m_connList;
    std::string m_url;			 //主机地址
	int m_port;		 //数据库端口号
	std::string m_user;		 //登陆数据库用户名
	std::string m_passWord;	 //登陆数据库密码
	std::string m_databaseName; //使用数据库名
};

class connGuard
{
public:
    connGuard(MYSQL **conn, SQLPool *connPool);
    ~connGuard();

private:
    MYSQL *m_conn;
    SQLPool *m_pool;
};
