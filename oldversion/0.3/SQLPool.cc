/*
 * @Author: GanShuang
 * @Date: 2020-05-22 10:24:10
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-27 11:57:27
 * @FilePath: /myWebServer-master/oldversion/0.3/SQLPool.cc
 */ 

#include "SQLPool.h"

SQLPool::SQLPool()
{
    m_curConn = 0;
    m_freeConn = 0;
}

SQLPool::~SQLPool()
{
    DestroyPool();
}

void
SQLPool::DestroyPool()
{
    m_locker.lock();
    if(!m_connList.empty())
    {
        list<MYSQL *>::iterator it;
        for(it = m_connList.begin(); it != m_connList.end(); it++)
        {
            MYSQL *conn = *it;
            mysql_close(conn);
        }
        m_curConn = 0;
        m_freeConn = 0;
        m_connList.clear();
    }
    m_locker.unlock();
}

void
SQLPool::init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, unsigned int MaxConn)
{
    m_url = url;
    m_user = User;
    m_passWord = PassWord;
    m_databaseName = DataBaseName;
    m_port = Port;

    m_locker.lock();
    for(int i = 0; i < MaxConn; i++)
    {
        MYSQL *conn = nullptr;
        conn = mysql_init(conn);

        if(conn == nullptr)
        {
            LOG_ERROR("SQL init Error:%s", mysql_error(conn));
            exit(1);
        }
        conn = mysql_real_connect(conn, m_url.c_str(), m_user.c_str(), m_passWord.c_str(), m_databaseName.c_str(), m_port, nullptr, 0);
        if(conn == nullptr)
        {
            LOG_ERROR("SQL connection Error:%s", mysql_error(conn));
            exit(1);
        }
        m_connList.push_back(conn);
        m_freeConn++;
    }

    m_sem = Semaphore(m_freeConn);
    m_maxConn = m_freeConn;
    m_locker.unlock();
}

MYSQL *
SQLPool::GetConnection()
{
    MYSQL *conn = nullptr;
    if(m_connList.empty()) return nullptr;
    m_sem.wait();
    m_locker.lock();
    conn = m_connList.front();
    m_connList.pop_front();
    m_freeConn--;
    m_curConn++;
    m_locker.unlock();
    return conn;
}

bool
SQLPool::ReleaseConnection(MYSQL *conn)
{
    if(conn == nullptr) return false;
    m_locker.lock();
    m_connList.push_back(conn);
    m_freeConn++;
    m_curConn--;
    m_locker.unlock();
    m_sem.post();
    return true;
}

connGuard::connGuard(MYSQL **conn, SQLPool *connPool)
{
    *conn = connPool->GetConnection();
    m_conn = *conn;
    m_pool = connPool;
}

connGuard::~connGuard()
{
    m_pool->ReleaseConnection(m_conn);
}