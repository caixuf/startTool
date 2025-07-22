#include "process_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>

// 进程状态
static ProcessState g_state = PROCESS_STATE_STOPPED;
static bool g_should_stop = false;
static LogCallback g_log_callback = NULL;
static pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;

// 进程信息
static ProcessInfo g_process_info = {
    .name = "example_process",
    .version = "1.0.0",
    .description = "Example process plugin for demonstration",
    .priority = 1,
    .restart_count = 3,
    .auto_restart = true
};

// 统计信息
static ProcessStats g_stats = {0};

/**
 * 日志函数
 */
static void log_message(LogLevel level, const char* format, ...) {
    if (!g_log_callback) {
        return;
    }
    
    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    g_log_callback(level, message);
}

/**
 * 获取进程信息
 */
static const ProcessInfo* get_process_info(void) {
    return &g_process_info;
}

/**
 * 初始化进程
 */
static int initialize(const char* config_data, LogCallback log_callback) {
    pthread_mutex_lock(&g_state_mutex);
    
    if (g_state != PROCESS_STATE_STOPPED) {
        pthread_mutex_unlock(&g_state_mutex);
        return -1;
    }
    
    g_state = PROCESS_STATE_INITIALIZING;
    g_log_callback = log_callback;
    g_should_stop = false;
    
    // 初始化统计信息
    memset(&g_stats, 0, sizeof(g_stats));
    g_stats.start_time = time(NULL);
    
    log_message(LOG_LEVEL_INFO, "Example process initializing...");
    
    // 解析配置数据（这里是一个简单的示例）
    if (config_data && strlen(config_data) > 0) {
        log_message(LOG_LEVEL_INFO, "Config data: %s", config_data);
    }
    
    // 模拟初始化过程
    sleep(1);
    
    g_state = PROCESS_STATE_STOPPED;
    pthread_mutex_unlock(&g_state_mutex);
    
    log_message(LOG_LEVEL_INFO, "Example process initialized successfully");
    return 0;
}

/**
 * 启动进程主循环
 */
static int start(void) {
    pthread_mutex_lock(&g_state_mutex);
    
    if (g_state != PROCESS_STATE_STOPPED) {
        pthread_mutex_unlock(&g_state_mutex);
        return -1;
    }
    
    g_state = PROCESS_STATE_RUNNING;
    g_should_stop = false;
    pthread_mutex_unlock(&g_state_mutex);
    
    log_message(LOG_LEVEL_INFO, "Example process started");
    
    // 主循环
    int cycle_count = 0;
    while (!g_should_stop) {
        // 模拟工作
        sleep(2);
        cycle_count++;
        
        // 更新统计信息
        pthread_mutex_lock(&g_state_mutex);
        g_stats.run_time = time(NULL) - g_stats.start_time;
        g_stats.cpu_usage = 10 + (cycle_count % 20); // 模拟CPU使用率
        g_stats.memory_usage = 1024 * 1024 * (5 + (cycle_count % 10)); // 模拟内存使用
        pthread_mutex_unlock(&g_state_mutex);
        
        if (cycle_count % 10 == 0) {
            log_message(LOG_LEVEL_INFO, "Example process is running, cycle: %d", cycle_count);
        }
    }
    
    pthread_mutex_lock(&g_state_mutex);
    g_state = PROCESS_STATE_STOPPED;
    pthread_mutex_unlock(&g_state_mutex);
    
    log_message(LOG_LEVEL_INFO, "Example process stopped");
    return 0;
}

/**
 * 停止进程
 */
static int stop(void) {
    pthread_mutex_lock(&g_state_mutex);
    
    if (g_state != PROCESS_STATE_RUNNING) {
        pthread_mutex_unlock(&g_state_mutex);
        return -1;
    }
    
    g_state = PROCESS_STATE_STOPPING;
    g_should_stop = true;
    pthread_mutex_unlock(&g_state_mutex);
    
    log_message(LOG_LEVEL_INFO, "Example process stopping...");
    
    // 等待主循环结束
    while (g_state == PROCESS_STATE_STOPPING) {
        usleep(100000); // 100ms
    }
    
    return 0;
}

/**
 * 清理资源
 */
static void cleanup(void) {
    pthread_mutex_lock(&g_state_mutex);
    g_state = PROCESS_STATE_STOPPED;
    g_should_stop = true;
    g_log_callback = NULL;
    pthread_mutex_unlock(&g_state_mutex);
    
    // 清理资源（如果有的话）
}

/**
 * 获取进程状态
 */
static ProcessState get_state(void) {
    pthread_mutex_lock(&g_state_mutex);
    ProcessState state = g_state;
    pthread_mutex_unlock(&g_state_mutex);
    return state;
}

/**
 * 获取进程统计信息
 */
static const ProcessStats* get_stats(void) {
    return &g_stats;
}

/**
 * 处理信号
 */
static void handle_signal(int signal) {
    log_message(LOG_LEVEL_INFO, "Example process received signal: %d", signal);
    
    if (signal == SIGTERM || signal == SIGINT) {
        g_should_stop = true;
    }
}

/**
 * 健康检查
 */
static bool health_check(void) {
    // 简单的健康检查：如果进程应该在运行但状态不是运行，则认为不健康
    pthread_mutex_lock(&g_state_mutex);
    bool healthy = (g_state == PROCESS_STATE_RUNNING && !g_should_stop) ||
                   (g_state == PROCESS_STATE_STOPPED);
    pthread_mutex_unlock(&g_state_mutex);
    
    return healthy;
}

// 进程接口实例
static ProcessInterface g_interface = {
    .get_process_info = get_process_info,
    .initialize = initialize,
    .start = start,
    .stop = stop,
    .cleanup = cleanup,
    .get_state = get_state,
    .get_stats = get_stats,
    .handle_signal = handle_signal,
    .health_check = health_check
};

/**
 * 导出函数：获取进程接口
 */
ProcessInterface* get_process_interface(void) {
    return &g_interface;
}

/**
 * 导出函数：获取接口版本
 */
uint32_t get_interface_version(void) {
    return PROCESS_INTERFACE_VERSION;
}
