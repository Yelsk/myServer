/*
 * @Author: GanShuang 
 * @Date: 2020-05-06 09:26:41 
 * @Last Modified by: GanShuang
 * @Last Modified time: 2020-05-08 21:02:58
 */

#include "HttpConnection.h"

using namespace std;

//初始化新连接
HttpConnection::HttpConnection(int _epfd,
                                                                        int _client_fd,
                                                                        int _events,
                                                                        Epoll *_epoll)
                                                                        :epfd(_epfd),
                                                                        client_fd(_client_fd),
                                                                        events(_events),
                                                                        epoll(_epoll),
                                                                        timer(nullptr)
{
}

HttpConnection::~HttpConnection()
{
    cout << "~HttpConnection()" << endl;
    struct epoll_event ev;
    epoll->epoll_del(this);
    if (timer != NULL)
    {
        timer->clearConn();
        timer = NULL;
    }
    close(client_fd);
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
HttpConnection::close_coon()
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, 0);
    close(client_fd);
    client_fd = -1;
}

void
HttpConnection::modfd(int epfd, 
                                                    int client_fd, 
                                                    int ev)
{
    epoll_event event;
    event.data.fd = client_fd;
    event.events = ev | EPOLLET | EPOLLHUP |EPOLLONESHOT;
    epoll_ctl(epfd, EPOLL_CTL_MOD, client_fd, &event);
}

//read函数的封装
int
HttpConnection::myread()
{
    bzero(&read_buf, sizeof(read_buf));
    while(true)
    {
        int ret = recv(client_fd, read_buf+read_count, READ_BUF-read_count, 0);
        if(ret == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return 0;
        }
        else if(ret == 0)
        {
            return 0;
        }
        read_count += ret;
    }
    strcpy(post_buf, read_buf);
    return 1;
}

//响应状态的填充
bool
HttpConnection::successful_respond() //200
{
    m_flag = false;
    bzero(request_head_buf, sizeof(request_head_buf));
    sprintf(request_head_buf, "HTTP/1.1 200 ok\r\nConnection: close\r\ncontent-length: %d\r\n\r\n", file_size);
}

bool
HttpConnection::bad_respond() //400
{
    bzero(url, strlen(url));
    strcpy(path_400, "bad_respond.html");
    url = path_400;
    bzero(filename, sizeof(filename));
    sprintf(filename, "/home/gan/linux_net/web_sever/%s", url);
    struct stat my_file;
    if(stat(filename, &my_file) < 0)
    {
        cout << "no file\n";
    }
    file_size = my_file.st_size;
    bzero(request_head_buf, sizeof(request_head_buf));
    sprintf(request_head_buf, "HTTP/1.1 400 BAD_REQUESTION\r\nConnection: close\r\ncontent-length:%d\r\n\r\n", file_size);
}

bool
HttpConnection::forbiden_respond() //403
{
    bzero(url, sizeof(url));
    strcpy(path_404, "not_found_request.html");
    url = path_404;
    bzero(filename, sizeof(filename));
    sprintf(filename,"/home/gan/linux_net/web_sever/%s",url);
    struct stat my_file;
    if(stat(filename,&my_file)<0)
    {
        cout << "草拟\n";
    }
    file_size = my_file.st_size;
    bzero(request_head_buf,sizeof(request_head_buf));
    sprintf(request_head_buf,"HTTP/1.1 404 NOT_FOUND\r\nConnection: close\r\ncontent-length:%d\r\n\r\n",file_size);
}

bool 
HttpConnection::not_found_request()//404
{
    bzero(url, strlen(url));
    strcpy(path_404,"not_found_request.html");
    url = path_404;
    bzero(filename,sizeof(filename));
    sprintf(filename,"/home/gan/linux_net/web_sever/%s",url);
    struct stat my_file;
    if(stat(filename,&my_file)<0)
    {
        cout << "草拟\n";
    }
    file_size = my_file.st_size;
    bzero(request_head_buf,sizeof(request_head_buf));
    sprintf(request_head_buf,"HTTP/1.1 404 NOT_FOUND\r\nConnection: close\r\ncontent-length:%d\r\n\r\n",file_size);
}

//动态请求处理
void
HttpConnection::dynamic(char *filename, char *argv)
{
    int len = strlen(argv);
    int k = 0;
    int number[2];
    int sum = 0;
    m_flag = true;
    bzero(request_head_buf, sizeof(request_head_buf));
    sscanf(argv, "a=%db=%d", &number[0], &number[1]);
    if(strcmp(filename, "/add") == 0)
    {
        sum = number[0] + number[1];
        sprintf(body, "<html><body>\r\n<p>%d + %d = %d </p><hr>\r\n</body></html>\r\n",number[0],number[1],sum);
        sprintf(request_head_buf,"HTTP/1.1 200 ok\r\nConnection: close\r\ncontent-length: %d\r\n\r\n",strlen(body));
    }
    else if(strcmp(filename,"/multiplication")==0)
    {
        cout << "\t\t\t\tmultiplication\n\n";
        sum = number[0]*number[1];
        sprintf(body,"<html><body>\r\n<p>%d * %d = %d </p><hr>\r\n</body></html>\r\n",number[0],number[1],sum);
        sprintf(request_head_buf,"HTTP/1.1 200 ok\r\nConnection: close\r\ncontent-length: %d\r\n\r\n",strlen(body));
    }
}

//POST请求处理
void
HttpConnection::post_respond()
{
    if(fork() == 0)
    {
        dup2(client_fd, STDOUT_FILENO);
        execl(filename, argv, nullptr);
    }
    wait(nullptr);
}

//判断一行是否读取完整
int
HttpConnection::judge_line(int &check_index, 
                                                            int &read_buf_len)
{
    cout << read_buf << endl;
    char ch;
    for(; check_index < read_buf_len; check_index++)
    {
        ch = read_buf[check_index];
        if(ch == '\r' && check_index+1 < read_buf_len && read_buf[check_index+1] == '\n')
        {
            read_buf[check_index++] = '\0';
            read_buf[check_index++] = '\0';
            return 1; //完整读入一行
        }
        if(ch == '\r' && check_index+1 == read_buf_len)
        {
            return 0;
        }
        if(ch == '\n')
        {
            if(check_index > 1 && read_buf[check_index-1] == '\r')
            {
                read_buf[check_index-1] = '\0';
                read_buf[check_index++] = '\0';
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    return 0;
}

//解析请求行
HttpConnection::HTTP_CODE
HttpConnection::requestion_analyse(char *temp)
{
    char *p = temp;
    cout << "p=" << p << endl;
    for(int i = 0; i < 2; i++)
    {
        if(i == 0)
        {
            method = p; //请求方法保存
            int j = 0;
            while((*p != ' ')&&(*p != '\r'))
            {
                p++;
            }
            p[0] = '\0';
            p++;
            cout << "method:" << method <<endl;
        }
        if(i == 1)
        {
            url = p; //文件路径保存
            while((*p != ' ')&&(*p != '\r'))
            {
                p++;
            }
            p[0] = '\0';
            p++;
            cout << "url:" << url << endl;
        }
    }
    version = p;
    while(*p != '\r')
    {
        p++;
    }
    p[0] = '\0';
    p++;
    p[0] = '\0';
    p++;
    cout << version << endl;
    if(strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0)
    {
        return BAD_REQUESTION;
    }
    if(!url || url[0] != '/')
    {
        return BAD_REQUESTION;
    }
    if(strcmp(version, "HTTP/1.1") != 0)
    {
        return BAD_REQUESTION;
    }
    status = HEAD;
    return NO_REQUESTION;
}

//解析头部信息
HttpConnection::HTTP_CODE
HttpConnection::head_analyse(char *temp)
{
    if(temp[0] == '\0')
    {
        //获得一个完整http请求
        return GET_REQUESTION;
    }
    //处理其他头部
    else if(strncasecmp(temp, "Connection:", 11) == 0)
    {
        temp = temp+11;
        while(*temp == ' ')
        {
            temp++;
        }
        if(strcasecmp(temp, "keep-alive") == 0)
        {
            m_linger = true;
        }
    }
    else if(strncasecmp(temp, "Content-Length:", 15) == 0)
    {
        temp = temp + 15;
        while(*temp == ' ')
        {
            cout << *temp << endl;
            temp++;
        }
        m_http_count = atol(temp);//content-length需要填充
    }
    else if(strncasecmp(temp,"Host:",5)==0)
    {
        temp = temp+5;
        while(*temp==' ')
        {
            temp++;
        }
        m_host = temp;
    }
    else{
        cout << "can't handle it's hand\n";
    }
    return NO_REQUESTION;
}

//GET方法请求，对其请求行进行解析，存写资源路径
HttpConnection::HTTP_CODE
HttpConnection::do_file()
{
        char path[40] = "/home/gan/linux_net/web_sever";
        char *ch;
        if(ch = strchr(url, '?'))
        {
            argv = ch + 1;
            *ch = '\0';
            strcpy(filename, url);
            return DYNAMIC_FILE;
        }
        else
        {
            strcpy(filename, path);
            strcat(filename, url);
            struct stat m_file_stat;
            if(stat(filename, &m_file_stat))
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
            file_size = m_file_stat.st_size;
            return FILE_REQUESTION;
        }
}

//POST方法请求，分解并且存入参数
HttpConnection::HTTP_CODE
HttpConnection::do_post()
{
    int k = 0;
    int star;
    char path[34] = "/home/gan/linux_net/web_sever";
    strcpy(filename, path);
    strcat(filename, url);
    star = read_buf_len - m_http_count;
    argv = post_buf + star;
    argv[strlen(argv)+1] = '\0';
    if(filename != nullptr && argv != nullptr)
    {
        return POST_FILE;
    }
    return BAD_REQUESTION;
}

/*http请求解析*/
HttpConnection::HTTP_CODE 
HttpConnection::analyse()
{
    status = REQUESTION;
    int flag;
    char *temp = read_buf;
    int star_line = 0;
    check_index = 0;
    int star = 0;
    read_buf_len = strlen(read_buf);
    int len = read_buf_len;
    while((flag=judge_line(check_index, len))==1)
    {
        temp = read_buf + star_line;
        star_line = check_index;
        switch(status)
        {
            case REQUESTION://请求行分析，包括文件名称和请求方法
            {
                cout << "requestion\n";
                int ret;
                ret = requestion_analyse(temp);
                if(ret==BAD_REQUESTION)
                {
                    cout << "ret == BAD_REQUESTION\n";
                    //请求格式不正确
                    return BAD_REQUESTION;
                }
                break;
            }
            case HEAD://请求头的分析
            {
                int ret;
                ret = head_analyse(temp);
                if(ret==GET_REQUESTION)//获取完整的HTTP请求
                {
                    if(strcmp(method,"GET")==0)
                    {
                        return do_file();//GET请求文件名分离函数     
                    }
                    else if(strcmp(method,"POST")==0)
                    {
                        return do_post();//POST请求参数分离函数
                    }
                    else{
                        return BAD_REQUESTION;
                    }
                }
                break;
            }
            default:
            {
                return INTERNAL_ERROR;
            }
        }
    }
    return NO_REQUESTION;//请求不完整，需要继续读入
}
 
/*线程取出工作任务的接口函数*/
void 
HttpConnection::doit()
{
    int choice = analyse();//根据解析请求头的结果做选择
    switch(choice)
    {
        case NO_REQUESTION://请求不完整
        {
            cout << "NO_REQUESTION\n";
            //将fd属性再次改为可读，让epoll进行监听
            modfd(epfd, client_fd, EPOLLIN);
            return;
        }
        case BAD_REQUESTION: //400
        {
            cout << "BAD_REQUESTION\n";
            bad_respond();
            modfd(epfd, client_fd, EPOLLOUT);
            break;
        }
        case FORBIDDEN_REQUESTION://403
        {
            cout << "forbiden_respond\n";
            forbiden_respond();
            modfd(epfd, client_fd, EPOLLOUT);
            break;
        }
        case NOT_FOUND://404
        {
            cout<<"not_found_request"<< endl;
            not_found_request();
            modfd(epfd, client_fd, EPOLLOUT);
            break;   
        }
        case FILE_REQUESTION://GET文件资源无问题
        {
            cout << "文件file request\n";
            successful_respond();
            modfd(epfd, client_fd, EPOLLOUT);
            break;
        }
        case DYNAMIC_FILE://动态请求处理
        {
            cout << "动态请求处理\n";
            cout << filename << " " << argv << endl;
            dynamic(filename, argv);
            modfd(epfd, client_fd, EPOLLOUT);
            break;
        }
        case POST_FILE://POST 方法处理
        {
            cout << "post_respond\n";
            post_respond();
            break;
        }
        default:
        {
            close_coon();
    }
 
    }
}

bool 
HttpConnection::mywrite()
{
    if(m_flag)//如果是动态请求，返回填充体
    {
        int ret=send(client_fd,request_head_buf,strlen(request_head_buf),0);
        int r = send(client_fd,body,strlen(body),0);
        if(ret>0 && r>0)
        {
            return true;
        }
    }
    else{
            //多线程只读不会出错
            int fd = open(filename,O_RDONLY);
            assert(fd != -1);
            int ret;
            ret = write(client_fd,request_head_buf,strlen(request_head_buf));
            if(ret < 0)
            {
                close(fd);
                return false;
            }
            //使用sendfile能直接在内核内进行拷贝
            ret = sendfile(client_fd, fd, NULL, file_size);
            if(ret < 0)
            {
                close(fd);
                return false;
            }
            close(fd);
            return true;
    }
    return false;
}