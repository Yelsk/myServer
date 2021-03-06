<!--
 * @Author: GanShuang
 * @Date: 2020-05-23 20:41:29
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-06-27 19:04:02
 * @FilePath: /myWebServer-master/README.md
--> 
# 问题和改进

**现状**

​		到0.3版为止，WebServer的基本功能都已经具备，压力测试得到的结果为3W+QPS，目前采用的是同步I/O模拟proactor模式，I/O操作全部在主线程进行，当I/O操作完成时，才会将请求对象插入到请求队列中，然后工作线程竞争任务，处理业务逻辑。

**问题：**

1. 考虑使用RAII封装资源，比如套接字，互斥锁，文件句柄和内存，避免编写过程中产生内存泄漏
2. 考虑使用条件变量代替信号量，简化设计
3. 考虑用智能指针代替原始指针，不需要delete
4. ~~考虑加入log功能，方便调试以及运行过程中的错误记载~~
5. ~~考虑某些类使用单例模式，节约系统资源~~
6. ~~HttpConnection中的enum的赋值并不是原子操作，换成其他方式表示状态~~

##### 小笔记：

EPOLLONESHOT (since Linux 2.6.2)
           Sets the one-shot behavior for the associated file descriptor. This means that after an event is pulled out with epoll_wait(2) the associated file descriptor is **internally disabled and no other events will be reported by the epoll interface**.  The user must call epoll_ctl() with EPOLL_CTL_MOD to rearm the file descriptor with a new event mask.



Q6  Will closing a file descriptor cause it to be removed from all epoll sets automatically?

A6  Yes, but be aware of the following point.  A file descriptor is a reference to an open file description (see open(2)).  Whenever a file descriptor is duplicated via dup(2), dup2(2), fcntl(2) F_DUPFD, or fork(2), a new file descriptor referring to the same open file description is created.  An open file description continues to exist until all file descriptors referring to it have been closed.  **A file descriptor is removed from an epoll set only after all the file descriptors referring to the underlying open file description have been closed (or before if the file descriptor is explicitly removed using epoll_ctl(2) EPOLL_CTL_DEL).** This means that even after a file descriptor that is part of an epoll set has been closed, events may be reported for that file descriptor if other file descriptors referring to the same underlying file description remain open.

当调用close()关闭对应的fd时，会使相应的引用计数减一，只有减到0时，epoll才会真的删掉它，所以，比较安全的做法是：先del掉它，再close它(如果不确定close是否真的关闭了这个文件。)。



##### 如何使用RAII

当我们在一个函数内部使用局部变量，当退出了这个局部变量的作用域时，这个变量也就销毁了；当这个变量是类对象时，这个时候，就会自动调用这个类的析构函数，而这一切都是自动发生的，不要程序员显式地去调用完成。

由于系统的资源不具有自动释放的功能，而C++中的类具有自动调用析构函数的功能。如果把资源用类进行封装，对资源操作都封装在类的内部，在析构函数中进行释放资源。当定义的局部变量的生命结束时，它的析构函数就会自动的被调用，如此，就不用程序员显式地去调用释放资源的操作。



##### errno == INTR

碰到EINTR错误的时候，有一些可以重启的系统调用要进行重启，而对于有一些系统调用是不能够重启的。accept、read、write、select和open之类的函数都是可以重启的。connect不能重启，得调用select等待连接完成。



**mysql_query()**

返回0为请求成功，其他为错误。