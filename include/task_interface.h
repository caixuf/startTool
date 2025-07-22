#ifndef TASK_INTERFACE_H
#define TASK_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 任务状态枚举
 */
typedef enum {
    TASK_STATE_UNKNOWN = 0,
    TASK_STATE_INITIALIZED,
    TASK_STATE_RUNNING,
    TASK_STATE_STOPPING,
    TASK_STATE_STOPPED,
    TASK_STATE_ERROR
} TaskState;

/**
 * 任务优先级
 */
typedef enum {
    TASK_PRIORITY_LOW = 0,
    TASK_PRIORITY_NORMAL,
    TASK_PRIORITY_HIGH,
    TASK_PRIORITY_CRITICAL
} TaskPriority;

/**
 * 任务统计信息
 */
typedef struct {
    uint64_t start_time;        // 启动时间戳
    uint64_t total_run_time;    // 总运行时间
    uint32_t execution_count;   // 执行次数
    uint32_t error_count;       // 错误次数
    uint32_t cpu_usage;         // CPU使用率(百分比)
    uint64_t memory_usage;      // 内存使用量(字节)
    uint64_t last_heartbeat;    // 最后心跳时间
} TaskStats;

/**
 * 任务配置结构体
 */
typedef struct {
    char name[64];              // 任务名称
    char description[256];      // 任务描述
    TaskPriority priority;      // 任务优先级
    uint32_t max_restart_count; // 最大重启次数
    uint32_t heartbeat_interval; // 心跳间隔(秒)
    bool auto_restart;          // 是否自动重启
    bool enable_stats;          // 是否启用统计
    void* custom_config;        // 自定义配置数据
} TaskConfig;

// 前向声明
struct TaskInterface;

/**
 * 任务基类结构体 - 所有具体任务都应该"继承"这个结构
 * 通过将此结构作为第一个成员放在具体任务结构中来实现"继承"
 */
typedef struct TaskBase {
    // 基类数据成员
    TaskConfig config;              // 任务配置
    TaskState state;                // 当前状态
    TaskStats stats;                // 统计信息
    pthread_t thread;               // 任务线程
    pthread_mutex_t mutex;          // 状态保护互斥锁
    bool should_stop;               // 停止标志
    uint32_t restart_count;         // 已重启次数
    
    // 虚函数表指针 (类似C++的vtable)
    const struct TaskInterface* vtable;
} TaskBase;

/**
 * 任务接口 - 虚函数表 (类似C++的虚函数)
 * 所有具体任务必须实现这些"虚函数"
 */
typedef struct TaskInterface {
    /**
     * 任务初始化 - 纯虚函数
     * @param task 任务基类指针
     * @return 0成功，非0失败
     */
    int (*initialize)(TaskBase* task);
    
    /**
     * 任务执行主循环 - 纯虚函数
     * @param task 任务基类指针
     * @return 0成功，非0失败
     */
    int (*execute)(TaskBase* task);
    
    /**
     * 任务清理 - 虚函数(可选重写)
     * @param task 任务基类指针
     */
    void (*cleanup)(TaskBase* task);
    
    /**
     * 任务暂停 - 虚函数(可选重写)
     * @param task 任务基类指针
     * @return 0成功，非0失败
     */
    int (*pause)(TaskBase* task);
    
    /**
     * 任务恢复 - 虚函数(可选重写)
     * @param task 任务基类指针
     * @return 0成功，非0失败
     */
    int (*resume)(TaskBase* task);
    
    /**
     * 处理信号 - 虚函数(可选重写)
     * @param task 任务基类指针
     * @param signal 信号值
     */
    void (*handle_signal)(TaskBase* task, int signal);
    
    /**
     * 健康检查 - 虚函数(可选重写)
     * @param task 任务基类指针
     * @return true健康，false不健康
     */
    bool (*health_check)(TaskBase* task);
    
    /**
     * 获取自定义状态信息 - 虚函数(可选重写)
     * @param task 任务基类指针
     * @param buffer 输出缓冲区
     * @param size 缓冲区大小
     * @return 写入的字节数
     */
    int (*get_status)(TaskBase* task, char* buffer, size_t size);
} TaskInterface;

/**
 * 任务基类构造函数
 * @param task 任务指针
 * @param vtable 虚函数表指针
 * @param config 任务配置
 * @return 0成功，非0失败
 */
int task_base_init(TaskBase* task, const TaskInterface* vtable, const TaskConfig* config);

/**
 * 任务基类析构函数
 * @param task 任务指针
 */
void task_base_destroy(TaskBase* task);

/**
 * 启动任务 - 创建线程执行任务
 * @param task 任务基类指针
 * @return 0成功，非0失败
 */
int task_start(TaskBase* task);

/**
 * 停止任务
 * @param task 任务基类指针
 * @return 0成功，非0失败
 */
int task_stop(TaskBase* task);

/**
 * 重启任务
 * @param task 任务基类指针
 * @return 0成功，非0失败
 */
int task_restart(TaskBase* task);

/**
 * 获取任务状态
 * @param task 任务基类指针
 * @return 任务状态
 */
TaskState task_get_state(TaskBase* task);

/**
 * 获取任务统计信息
 * @param task 任务基类指针
 * @return 统计信息指针
 */
const TaskStats* task_get_stats(TaskBase* task);

/**
 * 更新心跳时间
 * @param task 任务基类指针
 */
void task_update_heartbeat(TaskBase* task);

/**
 * 检查任务是否应该停止
 * @param task 任务基类指针
 * @return true应该停止，false继续运行
 */
bool task_should_stop(TaskBase* task);

/**
 * 设置任务自定义配置
 * @param task 任务基类指针
 * @param config 自定义配置指针
 */
void task_set_custom_config(TaskBase* task, void* config);

/**
 * 获取任务自定义配置
 * @param task 任务基类指针
 * @return 自定义配置指针
 */
void* task_get_custom_config(TaskBase* task);

// 便利宏 - 用于"调用虚函数"
#define TASK_CALL(task, method, ...) \
    ((task)->vtable && (task)->vtable->method ? \
     (task)->vtable->method((task), ##__VA_ARGS__) : -1)

#define TASK_CALL_VOID(task, method, ...) \
    do { \
        if ((task)->vtable && (task)->vtable->method) { \
            (task)->vtable->method((task), ##__VA_ARGS__); \
        } \
    } while(0)

// 类型转换宏 - 用于"向下转型"
#define TASK_CAST(type, task) ((type*)(task))

#ifdef __cplusplus
}
#endif

#endif // TASK_INTERFACE_H
