/*
 * @Author: GanShuang 
 * @Date: 2020-05-06 09:26:41 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-16 19:55:43
 */

#include "HttpConnection.h"

using namespace std;

HttpConnection::HttpConnection(int _epfd,
                                                                        int _client_fd,
                                                                        int _events,
                                                                        Epoll *_epoll,
                                                                        string _path,
                                                                        MutexLock *_lock,
                                                                        TimerQueue *_timerQueue)
                                                                        :epfd(_epfd),
                                                                        client_fd(_client_fd),
                                                                        events(_events),
                                                                        epoll(_epoll),
                                                                        path(_path),
                                                                        lock(_lock),
                                                                        timerQueue(_timerQueue),
                                                                        keep_alive(false),
                                                                        dynamic_flag(false)
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
    post_buffer.clear();
    contentBody.clear();
    check_index = 0;
    status = REQUETION;
    keep_alive = false;
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
    Reset();
    if(keep_alive)
    {
        timer = new Timer(this, KEEP_ALIVE_TIME);
    }
    else
    {
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
        delete this;
        return;
    }
    return;
}

size_t
HttpConnection::myread()
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
                perror("read error");
                return -1;
            }
        }
        else if(nread == 0) break; //可能是对端断开连接，读到EOF，读取端应断开连接
        readCount += nread;
        read_buffer += std::string(buffer, buffer+nread);
    }
    return readCount;
}

bool
HttpConnection::HandleWrite()
{
    if(dynamic_flag)//如果是动态请求，返回填充体
    {
        int ret=send(client_fd,request_head_buffer.c_str(),request_head_buffer.size(),0);
        int r = send(client_fd,contentBody.c_str(),contentBody.size(),0);
        if(ret>0 && r>0)
        {
            status = FINISH;
            return true;
        }
    }
    else{
            int ret;
            request_head_buffer += contentBody;
            int to_write = request_head_buffer.size(), have_write = 0;
            while (true)
            {
                ret = write(client_fd,request_head_buffer.c_str() + have_write,request_head_buffer.size() - have_write);
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
    return true;
}

bool
HttpConnection::HandleRead()
{
    int read_num = myread();
    if (read_num < 0)
    {
        return false;
    }
    else if(read_num == 0)
    {
        // 有请求出现但是读不到数据，可能是Request Aborted，或者来自网络的数据没有达到等原因
        // 最可能是对端已经关闭了，统一按照对端已经关闭处理
        return false;
    }
    return true;
}

//响应状态的填充
bool
HttpConnection::successful_respond() //200
{
    dynamic_flag = false;
    if(keep_alive)
    {
        //cout << request_head_buffer << endl;
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
    file_name = path + "bad_respond.html";
    struct stat my_file;
    if(stat(file_name.c_str(), &my_file) < 0)
    {
        cout << "no file\n";
    }
    file_size = my_file.st_size;
    request_head_buffer = "HTTP/1.1 400 BAD_REQUESTION\r\nConnection: close\r\nContent-length:" + to_string(file_size) +"\r\n\r\n";
}

bool
HttpConnection::forbiden_respond() //403
{
    file_name = path + "not_found_request.html";
    struct stat my_file;
    if(stat(file_name.c_str(),&my_file)<0)
    {
        cout << "forbiden\n";
    }
    file_size = my_file.st_size;
    request_head_buffer = "HTTP/1.1 403 FORBIDEN\r\nConnection: close\r\nContent-length:" + to_string(file_size) + "\r\n\r\n";
}

bool 
HttpConnection::not_found_request()//404
{
    file_name = path + "not_found_request.html";
    //cout << file_name << endl;
    struct stat my_file;
    if(stat(file_name.c_str(),&my_file)<0)
    {
        cout << "not found\n";
    }
    file_size = my_file.st_size;
    request_head_buffer = "HTTP/1.1 404 NOT_FOUND\r\nConnection: close\r\nContent-length:" + to_string(file_size) + "\r\n\r\n";
}

//动态请求处理
void
HttpConnection::dynamic(string &filename, string &argv)
{
    int k = 0;
    int number[2];
    int sum = 0;
    dynamic_flag = true;
    sscanf(argv.c_str(), "a=%db=%d", &number[0], &number[1]);
    if(file_name == "add")
    {
        sum = number[0] + number[1];
        contentBody = "<html><body>\r\n<p>" + to_string(number[0])  + "+ "+ to_string(number[1]) + "= " + to_string(sum) +" </p><hr>\r\n</body></html>\r\n";
        if(keep_alive)
        {
            request_head_buffer =  "HTTP/1.1 200 ok\r\nConnection: keep-alive\r\nKeep-Alive: timeout=" + to_string(KEEP_ALIVE_TIME) + "\r\nContent-length: " + to_string(contentBody.size()) + "\r\n\r\n";
        }
        else
        {
            request_head_buffer = "HTTP/1.1 200 ok\r\nConnection: close\r\nContent-length: " + to_string(contentBody.size()) + "\r\n\r\n";
        }
    }
    else if(file_name == "multiplication")
    {
        //cout << "\t\t\t\tmultiplication\n\n";
        sum = number[0]*number[1];
        contentBody = "<html><body>\r\n<p>" + to_string(number[0])  + "* "+ to_string(number[1]) + "= " + to_string(sum) +" </p><hr>\r\n</body></html>\r\n";
        if(keep_alive)
        {
            request_head_buffer =  "HTTP/1.1 200 ok\r\nConnection: keep-alive\r\nKeep-Alive: timeout=" + to_string(5 * 60 * 1000) + "\r\nContent-length: " + to_string(contentBody.size()) + "\r\n\r\n";
        }
        else
        {
            request_head_buffer = "HTTP/1.1 200 ok\r\nConnection: close\r\nContent-length: " + to_string(contentBody.size()) + "\r\n\r\n";
        }
    }
}

//POST请求处理
void
HttpConnection::post_respond()
{
    if((fork()) == 0)
    {
        dup2(client_fd, STDOUT_FILENO);
        execl(file_name.c_str(), argv.c_str(), nullptr);
    }
    wait(nullptr);
    status = FINISH;
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
    //cout << method << endl;
    pos++;
    start = pos;
    pos = line.find(' ', pos);
    if(pos < 0)
    {
        return BAD_REQUESTION;
    }
    if(line[start] == '/') start++;
    url = line.substr(start, pos-start);
    //cout << url << endl;
    pos++;
    start = pos;
    pos = line.find('\r', pos);
    if(pos < 0)
    {
        return BAD_REQUESTION;
    }
    version = line.substr(start, pos-start);
    //cout << version << endl;
    if(method != "GET" && method != "POST")
    {
        return BAD_REQUESTION;
    }
    if(url.empty())
    {
        return BAD_REQUESTION;
    }
    if(version != "HTTP/1.1")
    {
        return BAD_REQUESTION;
    }
    status = HEAD;
    //cout << "return no_requestion" << endl;
    return NO_REQUESTION;
}

//解析头部信息
int
HttpConnection::HeadersParse(string &line)
{
    //cout << "header parse" << endl;
    int pos = 0;
    if(line[0] == '\r')
    {
        //cout << "get requestion" << endl;
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
        //cout << tmp << endl;
        if(tmp == "keep-alive")
        {
            //cout << "keep_alive = true" << endl;
            keep_alive = true;
        }
    }
    else if((pos = line.find("Content-Length:")) != string::npos)
    {
        pos = pos + 16;
        int start = pos;
        pos = line.find('\r', start);
        string tmp = line.substr(start, pos - start);
        //cout << tmp << endl;
        contentLength = stoi(tmp);//content-length需要填充
    }
    else if((pos = line.find("Host:")) != string::npos)
    {
        pos = pos + 6;
        int start = pos;
        pos = line.find('\r', start);
        string tmp = line.substr(start, pos - start);
        //cout << tmp << endl;
        host = tmp;
    }
    else{
        //cout << "can't handle it's hand\n";
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

int
HttpConnection::Analyse()
{
    status = REQUETION;
    int flag;
    int start_line = 0;
    check_index = 0;
    while(flag = JudgeLine(check_index) == 1)
    {
        string line = read_buffer.substr(start_line, check_index - start_line - 1);
        //cout << line << endl;
        start_line = check_index;
        switch (status)
        {
            case REQUETION:
            {
                //cout << "parse requestion line" << endl;
                int ret = RequestionLineParse(line);
                if(ret==BAD_REQUESTION)
                {
                    //cout << "ret == BAD_REQUESTION\n";
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
                    if(method == "GET")
                    {
                        return HandleGet();//GET请求文件名分离函数     
                    }
                    else if(method == "POST")
                    {
                        return HandlePost();//POST请求参数分离函数
                    }
                    else{
                        return BAD_REQUESTION;
                    }
                }
            }
            default:
                break;
        }
    }
    if(error) {
        //cout << "internal error" << endl;
        return INTERNAL_ERROR;
    }
    //请求不完整需要继续读入
    return NO_REQUESTION;
}

//GET方法请求，对其请求行进行解析，存写资源路径
int
HttpConnection::HandleGet()
{
    int pos = url.find('?');
    if(pos >= 0)
    {
        //cout << "dynamic_file" << endl;
        argv = url.substr(pos+1);
        file_name = url.substr(0, pos);
        return DYNAMIC_FILE;
    }
    else
    {
        file_name = url;
        //cout << "file_name" << file_name << endl;
        struct stat m_file_stat;
        if(stat(file_name.c_str(), &m_file_stat))
        {
            return NOT_FOUND;
        }
        if(!(m_file_stat.st_mode & S_IROTH))
        {
            return FORBIDDEN_REQUESTION;
        }
        if(S_ISDIR(m_file_stat.st_mode))
        {
            return BAD_REQUESTION;
        }
        int fd = open(file_name.c_str(), O_RDONLY);
        m_file_address = string((char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
        close(fd);
        contentBody = m_file_address;
        munmap((void *)m_file_address.c_str(), m_file_stat.st_size);
        file_size = m_file_stat.st_size;
        return FILE_REQUESTION;
    }
}

//POST方法请求，分解并且存入参数
int
HttpConnection::HandlePost()
{
    file_name = url;
    argv = post_buffer.substr(read_buffer.size() - contentLength);
    //cout << "argv: " << argv << endl;
    //cout << "file_name" << file_name << endl;
    if(!file_name.empty() && !argv.empty())
    {
        return POST_FILE;
    }
    return BAD_REQUESTION;
}

//线程取出工作任务的接口函数
void 
HttpConnection::doit()
{
   //cout << "doit" << endl;
    int choice = Analyse();//根据解析请求头的结果做选择
    events = EPOLLRDHUP | EPOLLONESHOT | EPOLLET | EPOLLERR;
    switch(choice)
    {
        case NO_REQUESTION://请求不完整
        {
            //cout << "NO_REQUESTION\n";
            //将fd属性再次改为可读，让epoll进行监听
            events |= EPOLLIN;
            epoll->epoll_mod(this);
            return;
        }
        case BAD_REQUESTION: //400
        {
            //cout << "BAD_REQUESTION\n";
            bad_respond();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case FORBIDDEN_REQUESTION://403
        {
            //cout << "forbiden_respond\n";
            forbiden_respond();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case NOT_FOUND://404
        {
            //cout<<"not_found_request"<< endl;
            not_found_request();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case FILE_REQUESTION://GET文件资源无问题
        {
            //cout << "文件file request\n";
            successful_respond();
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case DYNAMIC_FILE: //动态请求处理
        {
            //cout << "动态请求处理\n";
            //cout << file_name << " " << argv << endl;
            dynamic(file_name, argv);
            events |= EPOLLOUT;
            epoll->epoll_mod(this);
            break;
        }
        case POST_FILE: //POST 方法处理
        {
            //cout << "post_respond\n";
            post_respond();
            break;
        }
        case FINISH: //长连接短连接处理
        {
            //cout << "handle connection" << endl;
            HandleConn();
            break;
        }
        default:
        {
            delete this;
        }
    }
}