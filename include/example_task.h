#ifndef EXAMPLE_TASK_H
#define EXAMPLE_TASK_H

#include "task_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// 前向声明
typedef struct ExampleTask ExampleTask;

/**
 * 示例任务配置结构体
 */
typedef struct ExampleTaskConfig {
    int work_interval;          // 工作间隔(秒)
    bool use_random_delay;      // 是否使用随机延迟
    char message[256];          // 自定义消息
} ExampleTaskConfig;

/**
 * 创建示例任务
 * @param config 任务配置
 * @param custom_config 自定义配置
 * @return 任务指针，失败返回NULL
 */
ExampleTask* example_task_create(const TaskConfig* config, const ExampleTaskConfig* custom_config);

/**
 * 销毁示例任务
 * @param task 任务指针
 */
void example_task_destroy(ExampleTask* task);

/**
 * 获取任务基类指针 - 用于向上转型
 * @param task 示例任务指针
 * @return 基类指针
 */
TaskBase* example_task_get_base(ExampleTask* task);

#ifdef __cplusplus
}
#endif

#endif // EXAMPLE_TASK_H
