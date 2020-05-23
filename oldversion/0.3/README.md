<!--
 * @Author: GanShuang
 * @Date: 2020-05-23 20:41:29
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-23 20:43:45
 * @FilePath: /myWebServer-master/oldversion/0.3/README.md
--> 
# 0.3版 C++ Web Server

#### 0.3版增加功能实现：

- POST方法实现登录，注册
- 单例模式实现MYSQL连接池，同步进行用户信息检验
- 完善长连接

##### 其他：

- 单例模式使用的线程安全懒汉模式。
- 和线程池一样，数据库连接池使用的也是一个list来存储连接，同样使用的信号量机制来对连接进行竞争。
- 互斥锁的使用：因为会给每一个线程分配一个MYSQL连接，所以不存在同时会有两个线程同时向一个连接发送请求的情况，mysql_query不需要加锁。
- 长连接的实现就是延长HttpConnection对象的定时器时间。

**测试：**

![image](https://github.com/Yelsk/myServer/blob/master/data/ServerWithoutLog_0.3.png)

在关闭Log功能的情况下进行短连接压力测试，QPS为34073。

![image](https://github.com/Yelsk/myServer/blob/master/data/ServerWithLog_0.3.png)

在开启Log功能的情况下进行短连接压力测试，QPS为9549。

