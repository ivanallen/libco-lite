# 轻量级协程框架

##  编译

```
$ make
```

## 特性

- pthread 风格接口，简单易用
- 支持多线程

## 快速入门

```c
#include "coroutine.h"
void *fun(void *arg) {
    int i = 10;
    while(i--) {
        printf("%s, I'm fun\n", (char*)arg);
        co_sleep(1);
    }
}

int main() {
    int cid;
    int i = 5;
    co_create(&cid, fun, "Hello");
    while(i--) {
        printf("hello, I'm main\n");
        co_sleep(1);
    }
    co_join(cid);
    printf("over!\n");
    return 0;
}
```

## 协程基础

看代码吃力的同学，可以参考下面的文章：

- [《协程基本原理及实现》](https://blog.csdn.net/q1007729991/article/details/60104151)

## 贡献

开源项目，欢迎大家多多贡献，bug 现在也很多，大家一起发现解决！我们的 QQ 群：610441700
