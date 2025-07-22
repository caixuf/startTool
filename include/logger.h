#ifndef LOGGER_H
#define LOGGER_H

#include "process_interface.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 日志器结构体
 */
typedef struct {
    FILE* file;
    LogLevel level;
    char filename[256];
} Logger;

/**
 * 创建日志器
 * @param filename 日志文件名
 * @param level 日志级别
 * @return 日志器指针，失败返回NULL
 */
Logger* logger_create(const char* filename, LogLevel level);

/**
 * 销毁日志器
 * @param logger 日志器指针
 */
void logger_destroy(Logger* logger);

/**
 * 写入日志
 * @param logger 日志器指针
 * @param level 日志级别
 * @param message 消息
 */
void logger_log(Logger* logger, LogLevel level, const char* message);

/**
 * 日志回调函数实现
 * @param level 日志级别
 * @param message 消息
 */
void default_log_callback(LogLevel level, const char* message);

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H
