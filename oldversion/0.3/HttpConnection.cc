/*
 * @Author: GanShuang
 * @Date: 2020-05-21 18:59:39
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-22 23:26:44
 * @FilePath: /myWebServer-master/oldversion/0.3/HttpConnection.cc
 */ 

#include "Log.h"
#include "HttpConnection.h"

using namespace std;

HttpConnection::HttpConnection(int _epfd,
                                                                        int _client_fd,
                                                                        int _events,
                                                                        Epoll *_epoll,
                                                                        string _path,
                                                                        MutexLock *_lock,
                                                                        TimerQueue *_timerQueue,
                                                                        SQLPool *_sqlpool,
                                                                        sockaddr_in address)
                                                                        :epfd(_epfd),
                                                                        client_fd(_client_fd),
                                                                        events(_events),
                                                                        epoll(_epoll),
                                                                        path(_path),
                                                                        lock(_lock),
                                                                        timerQueue(_timerQueue),
                                                                        sqlpool(_sqlpool),
                                                                        m_address(address),
                                                                        keep_alive(false)
{
}

HttpConnection::~HttpConnection()
{
    //cout << "~HttpConnection()" << endl;
    epoll->epoll_del(this);
    if (timer != NULL)
    {
        timer->clearConn();
        timer = NULL;
    }
    close(client_fd);
    client_fd = -1;
}

void
HttpConnection::Reset()
{
    read_buffer.clear();
    request_head_buffer.clear();
    contentBody.clear();
    url.clear();
    method.clear();
    file_name.clear();
    check_index = 0;
    contentLength = 0;
    status = REQUETION;
    keep_alive = false;
    error = false;
}

void
HttpConnection::seperateTimer()
{
    if(timer)
    {
        timer->clearConn();
        timer = nullptr;
    }
}

void
HttpConnection::HandleConn()
{
    if(keep_alive)
    {
        Reset();
        timer = new Timer(this, KEEP_ALIVE_TIME);
    }
    else
    {
        LOG_INFO("close connection");
        Log::get_instance()->flush();
        delete this;
        return;
    }
    lock->lock();
    timerQueue->addTimer(timer);
    lock->unlock();
    events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLET | EPOLLERR;
    int ret = epoll->epoll_mod(this);
    if (ret < 0)
    {
        // 返回错误处理
        LOG_ERROR("epoll error");
        delete this;
        return;
    }
    return;
}

bool
HttpConnection::HandleWrite()
{
    int ret;
    LOG_INFO("request:%s", request_head_buffer);
    Log::get_instance()->flush();
    request_head_buffer += contentBody;
    int to_write = request_head_buffer.size(), have_write = 0;
    while (true)
    {
        ret = send(client_fd,request_head_buffer.c_str() + have_write,request_head_buffer.size() - have_write, 0);
        if(ret < 0)
        {
            return false;
        }
        else if(ret < 0)
        {
            if(ret == -1)
            {
                if(errno == EAGAIN)
                {
                    return true;
                }
                return false;
            }
        }
        else if(ret >= (to_write - have_write))
        {
            return true;
        }
        have_write += ret;
    }
    status = FINISH;
    return true;
}

bool
HttpConnection::HandleRead()
{
    ssize_t nread = 0;
    ssize_t readCount = 0;
    while(true)
    {
        char buffer[MAX_BUFF];
        if((nread = recv(client_fd, buffer, MAX_BUFF, 0)) < 0)
        {
            if(errno == EINTR)  continue;
            else if(errno == EAGAIN)
            {
                return readCount;
            }
            else{
                LOG_ERROR("read error");
                return -1;
            }
        }
        else if(nread == 0) break; //可能是对端断开连接，读到EOF，读取端应断开连接
        readCount += nread;
        read_buffer += std::string(buffer, buffer+nread);
    }
    if (readCount < 0)
    {
        return false;
    }
    else if(readCount == 0)
    {
        // 有请求出现但是读不到数据，可能是Request Aborted，或者来自网络的数据没有达到等原因
        // 最可能是对端已经关闭了，统一按照对端已经关闭处理
        LOG_INFO("close connection");
        return false;
    }
    return true;
}

//响应状态的填充
bool
HttpConnection::successful_respond() //200
{
    if(keep_alive)
    {
        request_head_buffer =  "HTTP/1.1 200 ok\r\nConnection: keep-alive\r\nKeep-Alive: timeout=" + to_string(KEEP_ALIVE_TIME) + "\r\nContent-length: " + to_string(file_size) + "\r\n\r\n";
    }
    else
    {
        request_head_buffer = "HTTP/1.1 200 ok\r\nConnection: close\r\nContent-length: " + to_string(file_size) + "\r\n\r\n";
    }
}

bool
HttpConnection::bad_respond() //400
{
    request_head_buffer = "HTTP/1.1 400 BAD_REQUESTION\r\nConnection: close\r\nContent-length:" + to_string(file_size) +"\r\n\r\n";
}

bool
HttpConnection::forbiden_respond() //403
{
    request_head_buffer = "HTTP/1.1 403 FORBIDEN\r\nConnection: close\r\nContent-length:" + to_string(file_size) + "\r\n\r\n";
}

bool 
HttpConnection::not_found_request()//404
{
    request_head_buffer = "HTTP/1.1 404 NOT_FOUND\r\nConnection: close\r\nContent-length:" + to_string(file_size) + "\r\n\r\n";
}

//解析请求行
int
HttpConnection::RequestionLineParse(string &line)
{
    int pos = line.find(' ');
    int start = 0;
    if(pos < 0)
    {
        return BAD_REQUESTION;
    }
    method = line.substr(start, pos-start);
    pos++;
    start = pos;
    pos = line.find(' ', pos);
    if(pos < 0)
    {
        return BAD_REQUESTION;
    }
    if(line[start] == '/') start++;
    url = line.substr(start, pos-start);
    pos++;
    start = pos;
    pos = line.find('\r', pos);
    if(pos < 0)
    {
        return BAD_REQUESTION;
    }
    version = line.substr(start, pos-start);
    if(method != "GET" && method != "POST")
    {
        return BAD_REQUESTION;
    }
    if(version != "HTTP/1.1")
    {
        return BAD_REQUESTION;
    }
    status = HEAD;
    return NO_REQUESTION;
}

//解析头部信息
int
HttpConnection::HeadersParse(string &line)
{
    int pos = 0;
    if(line[0] == '\r')
    {
        if(contentLength != 0){
            status = CONTENT;
            return NO_REQUESTION;
        }
        status = FINISH;
        //获得一个完整http请求
        return GET_REQUESTION;
    }
    //处理其他头部
    else if((pos = line.find("Connection")) != string::npos)
    {
        pos = pos + 12;
        int start = pos;
        pos = line.find('\r', start);
        string tmp = line.substr(start, pos - start);
        if(tmp == "keep-alive")
        {
            keep_alive = true;
        }
    }
    else if((pos = line.find("Content-Length:")) != string::npos)
    {
        pos = pos + 16;
        int start = pos;
        pos = line.find('\r', start);
        string tmp = line.substr(start, pos - start);
        contentLength = stoi(tmp);//content-length需要填充
    }
    else if((pos = line.find("Host:")) != string::npos)
    {
        pos = pos + 6;
        int start = pos;
        pos = line.find('\r', start);
        string tmp = line.substr(start, pos - start);
        host = tmp;
    }
    else{
        LOG_INFO("can't handle its header");
        Log::get_instance()->flush();
    }
    return NO_REQUESTION;
}

int
HttpConnection::ContentParse(string &line)
{
    if (read_buffer.size() >= (contentLength + check_index))
    {
        requestContent = read_buffer.substr(check_index);
        //POST请求中最后为输入的用户名和密码
        status = FINISH;
        return GET_REQUESTION;
    }
    return NO_REQUESTION;
}

//判断一行是否读取完整
int
HttpConnection::JudgeLine(int &check_index)
{
    int n = 0;
    n = read_buffer.find('\r', check_index);
    if(n == string::npos || n < 0)
    {
        return 0;
    }
    check_index = n;
    if(check_index+1 < read_buffer.size() && read_buffer[check_index+1] == '\n')
    {
        check_index += 2;
        return 1;
    }
    else if(check_index+1 >= read_buffer.size())
    {
        return 0;
    }
    else{
        //出现语法错误
        error = true;
        return 0;
    }
    return 0;
}

//对其请求行进行解析，存写资源路径
int
HttpConnection::HandleRequest()
{
    int retval = FILE_REQUESTION;
    if(url.empty()){
        url = "judge.html";
        file_name = path + url;
    }
    if(method == "POST" && (url[0] == '2' || url[0] == '3'))
    {
        int pos = requestContent.find('&');
        string user = requestContent.substr(0, pos);
        string password = requestContent.substr(pos+1, contentLength - pos - 1);
        pos = user.find('=');
        user = user.substr(pos+1);
        pos = password.find('=');
        password = password.substr(pos+1);
        {
            connGuard connguard(&mysql, sqlpool);
            string sql_search = "SELECT username,passwd FROM user WHERE username='" + user + "' AND passwd='" + password + "'";
            lock->lock();
            int ret = mysql_query(mysql, sql_search.c_str());
            lock->unlock();
            if(url[0] == '2')
            {
                if(ret) file_name = path + "logError.html";
                else file_name = path + "welcome.html";
            }
            else if(url[0] == '3')
            {
                if(ret)
                {
                    string sql_insert = "INSERT INTO user(username, passwd) VALUES('" + user + "', '" + password + "')";
                    lock->lock();
                    int res = mysql_query(mysql, sql_insert.c_str());
                    lock->unlock();
                    if(res)
                    {
                        file_name = path + "registerError.html";
                    }
                    else
                    {
                        file_name = path + "log.html";
                    }
                }
                else
                {
                    file_name += "registerError.html";
                }
            }
        }
    }
    if(url[0] == '0')
    {
        file_name = path + "register.html";
    }
    else if(url[0] == '1')
    {
        file_name = path + "log.html";
    }
    struct stat m_file_stat;
    if(stat(file_name.c_str(), &m_file_stat))
    {
        file_name = path + "not_found_request.html";
        retval = NOT_FOUND;
    }
    if(!(m_file_stat.st_mode & S_IROTH))
    {
        file_name = path + "not_found_request.html";
        retval = FORBIDDEN_REQUESTION;
    }
    if(S_ISDIR(m_file_stat.st_mode))
    {
        file_name = path + "bad_respond.html";
        retval = BAD_REQUESTION;
    }
    stat(file_name.c_str(), &m_file_stat);
    int fd = open(file_name.c_str(), O_RDONLY);
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    contentBody = string(m_file_address);
    munmap((void *)m_file_address, m_file_stat.st_size);
    file_size = m_file_stat.st_size;
    return retval;
}

int
HttpConnection::Analyse()
{
    status = REQUETION;
    int flag;
    int start_line = 0;
    check_index = 0;
    while(status == CONTENT || (flag = JudgeLine(check_index)) == 1)
    {
        string line = read_buffer.substr(start_line, check_index - start_line - 1);
        start_line = check_index;
        switch (status)
        {
            case REQUETION:
            {
                int ret = RequestionLineParse(line);
                if(ret==BAD_REQUESTION)
                {
                    //请求格式不正确
                    return BAD_REQUESTION;
                }
                break;
            }
            case HEAD:
            {
                int ret = HeadersParse(line);
                if(ret==GET_REQUESTION)//获取完整的HTTP请求
                {
                    return HandleRequest();
                }
                break;
            }
            case CONTENT:
            {
                int ret = ContentParse(line);
                if(ret==GET_REQUESTION)//获取完整的HTTP请求
                {
                    return HandleRequest();
                }
                break;
            }
            default:
                return INTERNAL_ERROR;
                break;
        }
    }
    if(error) {
        LOG_ERROR("internal error");
        return INTERNAL_ERROR;
    }
    //请求不完整需要继续读入
    return NO_REQUESTION;
}

//线程取出工作任务的接口函数
void 
HttpConnection::doit()
{
    int choice = Analyse();//根据解析请求头的结果做选择
    events = EPOLLRDHUP | EPOLLONESHOT | EPOLLET | EPOLLERR;
    switch(choice)
    {
        case NO_REQUESTION://请求不完整
        {
            //将fd属性再次改为可读，让epoll进行监听
            events |= EPOLLIN;
            epoll->epoll_mod(this);
            return;
        }
        case BAD_REQUESTION: //400
        {
            bad_respond();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case FORBIDDEN_REQUESTION://403
        {
            forbiden_respond();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case NOT_FOUND://404
        {
            not_found_request();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case FILE_REQUESTION://GET文件资源无问题
        {
            successful_respond();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case FINISH: //长连接短连接处理
        {
            HandleConn();
            break;
        }
        default:
        {
            delete this;
        }
    }
}