#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdio.h>
#include "util.h"

// 示例，打印的结果类似这样
// [2017-10-10 12:34:23.345] [DEBUG] [thread.cc:74][~thread] thread mars_thread destroy
#define PRINT(TARGET, TYPE, COLOR, format, ...) do { \
    fprintf(TARGET, "\x1b[K"); \
    fprintf(TARGET, COLOR); \
    char time_str[64]; \
    now(time_str);\
    fprintf(TARGET, "[%s] [%s] [%s:%d][%s] - ", time_str, TYPE, __FILE__, __LINE__, __FUNCTION__); \
    fprintf(TARGET, "\x1b[0m");\
    fprintf(TARGET, format, ##__VA_ARGS__);\
} while(0)


#define LOG_RED "\x1b[31m"
#define LOG_GREEN "\x1b[32m"
#define LOG_YELLOW "\x1b[33m"
#define LOG_BLUE "\x1b[36m"

// 2 个 ## 表示可以有，可以没有 
#ifdef _DEBUG
#define LOG_DEBUG(format,...) PRINT(stdout, "DEBUG", LOG_BLUE, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format,...)
#endif

#define LOG_INFO(format,...) PRINT(stdout, "INFO", LOG_GREEN, format, ##__VA_ARGS__)
#define LOG_ERROR(format,...) PRINT(stderr, "ERROR", LOG_RED, format, ##__VA_ARGS__)
#define LOG_WARN(format,...) PRINT(stderr, "WARN", LOG_YELLOW, format, ##__VA_ARGS__)


#endif //__LOGGER_H__
