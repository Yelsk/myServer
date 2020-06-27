<!--
 * @Author: GanShuang
 * @Date: 2020-05-21 11:44:44
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-24 15:54:23
 * @FilePath: /myWebServer-master/oldversion/0.2/README.md
--> 
# 0.2版 C++ Web Server

#### 0.2版增加功能实现：

##### Log功能：

- 使用单例模式创建日志系统，对服务器运行状态、错误信息等进行记录
- 实现按天分类、超行分类
- 实现异步写入

##### Log异步模型：

![image](https://github.com/Yelsk/myServer/blob/master/data/log.jpg)

##### 其他：

- 单例模式使用的线程安全懒汉模式
- 当其他线程调用Log写入方法时，写入信息会先放在阻塞队列，阻塞队列使用的是条件变量实现的生产者消费者模式。Log写入线程会异步的读取阻塞队列中的信息，并将其写入文件流中。
- 互斥锁的使用：
  - 阻塞队列的任何操作都需要加锁。
  - 文件流的写入也需要加锁。

**测试：**

CPU: i5-4210M

内存: 8G

![image](https://github.com/Yelsk/myServer/blob/master/data/ServerWithoutLog_0.2.png)

在关闭Log功能的情况下进行短连接压力测试，QPS为30585。

![ServerWithLog](https://github.com/Yelsk/myServer/blob/master/data/ServerWithLog_0.2.png)

在开启Log功能的情况下进行短连接压力测试，QPS为9398。
