#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 进程配置项
 */
typedef struct {
    char name[64];           // 进程名称
    char library_path[256];  // 动态库路径
    char config_data[1024];  // 配置数据
    int priority;            // 优先级
    bool auto_start;         // 是否自动启动
} ProcessConfig;

/**
 * 启动器配置
 */
typedef struct {
    char log_file[256];      // 日志文件路径
    int log_level;           // 日志级别
    int monitor_interval;    // 监控间隔(秒)
    bool enable_monitor;     // 是否启用监控
    ProcessConfig* processes; // 进程配置数组
    int process_count;       // 进程数量
} LauncherConfig;

/**
 * 加载配置文件
 * @param config_file 配置文件路径
 * @return 配置结构体指针，失败返回NULL
 */
LauncherConfig* config_load(const char* config_file);

/**
 * 释放配置
 * @param config 配置结构体指针
 */
void config_free(LauncherConfig* config);

/**
 * 保存配置文件
 * @param config 配置结构体指针
 * @param config_file 配置文件路径
 * @return 0成功，非0失败
 */
int config_save(const LauncherConfig* config, const char* config_file);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_MANAGER_H
