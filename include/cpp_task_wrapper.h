#ifndef CPP_TASK_WRAPPER_H
#define CPP_TASK_WRAPPER_H

#include "task_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

// 前向声明
typedef struct CppTaskWrapper CppTaskWrapper;

/**
 * 创建C++示例任务
 * @param config 任务配置
 * @return 任务包装器指针，失败返回NULL
 */
CppTaskWrapper* cpp_example_task_create(const TaskConfig* config);

/**
 * 销毁C++示例任务
 * @param wrapper 任务包装器指针
 */
void cpp_example_task_destroy(CppTaskWrapper* wrapper);

/**
 * 获取任务基类指针 - 用于向上转型
 * @param wrapper 任务包装器指针
 * @return 基类指针
 */
TaskBase* cpp_example_task_get_base(CppTaskWrapper* wrapper);

#ifdef __cplusplus
}
#endif

#endif // CPP_TASK_WRAPPER_H
