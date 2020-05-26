/*
 * @Author: GanShuang
 * @Date: 2020-05-25 17:25:41
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-25 17:28:39
 * @FilePath: /myWebServer-master/NonCopyable.h
 */ 

#pragma once

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}
private:
    NonCopyable(const NonCopyable&);
    const NonCopyable& operator=(const NonCopyable&);
};
