/*
 * @Author: GanShuang
 * @Date: 2020-05-25 11:35:49
 * @LastEditors: GanShuang
 * @LastEditTime: 2020-05-25 19:33:25
 * @FilePath: /myWebServer-master/CurrentThread.h
 */ 

#pragma once
#include <stdint.h>

namespace CurrentThread {
// internal
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;
void cacheTid();
inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char* tidString()  // for logging
{
  return t_tidString;
}

inline int tidStringLength()  // for logging
{
  return t_tidStringLength;
}

inline const char* name() { return t_threadName; }
}
