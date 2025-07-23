# StartTool 实战练习项目

> **适合：** 完成快速入门后的进阶练习  
> **难度：** 初级到中级  
> **时间：** 每个项目1-3天

## 🎯 练习项目列表

### 项目1：系统资源监控器 ⭐⭐
**学习目标：** 掌握系统调用和文件操作

**功能要求：**
- 监控CPU使用率、内存使用率、磁盘空间
- 每30秒输出一次统计信息
- 当资源使用率超过阈值时发出警告

**核心技术点：**
- 读取 `/proc/stat`、`/proc/meminfo` 文件
- 配置文件解析
- 定时器实现

**代码框架：**
```c
typedef struct {
    TaskBase base;
    float cpu_threshold;    // CPU警告阈值
    float memory_threshold; // 内存警告阈值
    int check_interval;     // 检查间隔
} SystemMonitorTask;

static int monitor_execute(TaskBase* base) {
    SystemMonitorTask* task = (SystemMonitorTask*)base;
    
    while (!base->should_stop) {
        // 获取CPU使用率
        float cpu_usage = get_cpu_usage();
        
        // 获取内存使用率  
        float memory_usage = get_memory_usage();
        
        // 检查阈值并警告
        if (cpu_usage > task->cpu_threshold) {
            printf("⚠️  CPU使用率过高: %.1f%%\n", cpu_usage);
        }
        
        sleep(task->check_interval);
    }
    return 0;
}
```

---

### 项目2：HTTP服务器任务 ⭐⭐⭐
**学习目标：** 网络编程和多线程处理

**功能要求：**
- 监听指定端口，处理HTTP GET请求
- 支持静态文件服务
- 并发处理多个客户端连接
- 访问日志记录

**核心技术点：**
- Socket编程
- HTTP协议解析
- 线程池或select/epoll
- 文件I/O操作

**代码框架：**
```c
typedef struct {
    TaskBase base;
    int server_socket;
    int port;
    char webroot[256];
    pthread_t* worker_threads;
    int thread_count;
} HttpServerTask;

static int http_server_execute(TaskBase* base) {
    HttpServerTask* task = (HttpServerTask*)base;
    
    // 创建服务器socket
    task->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    // 绑定端口
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(task->port);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(task->server_socket, (struct sockaddr*)&addr, sizeof(addr));
    
    // 监听连接
    listen(task->server_socket, 10);
    
    while (!base->should_stop) {
        int client_socket = accept(task->server_socket, NULL, NULL);
        if (client_socket > 0) {
            // 处理HTTP请求
            handle_http_request(client_socket, task->webroot);
            close(client_socket);
        }
    }
    
    return 0;
}
```

---

### 项目3：定时任务调度器 ⭐⭐⭐⭐
**学习目标：** 复杂的任务管理和调度算法

**功能要求：**
- 支持cron表达式格式的定时任务
- 任务可以是外部命令或内部函数
- 支持任务优先级和依赖关系
- 任务执行历史和失败重试

**核心技术点：**
- 时间处理和cron解析
- 动态任务管理
- 子进程创建和管理
- 数据结构设计(优先队列)

**代码框架：**
```c
typedef struct ScheduledJob {
    char name[64];
    char cron_expr[128];     // "0 */5 * * * *" 每5分钟
    char command[256];
    int priority;
    time_t next_run_time;
    int retry_count;
    struct ScheduledJob* next;
} ScheduledJob;

typedef struct {
    TaskBase base;
    ScheduledJob* job_list;
    pthread_mutex_t job_mutex;
} SchedulerTask;

static int scheduler_execute(TaskBase* base) {
    SchedulerTask* task = (SchedulerTask*)base;
    
    while (!base->should_stop) {
        time_t now = time(NULL);
        
        pthread_mutex_lock(&task->job_mutex);
        
        // 检查所有任务
        ScheduledJob* job = task->job_list;
        while (job) {
            if (now >= job->next_run_time) {
                // 执行任务
                execute_job(job);
                // 计算下次执行时间
                job->next_run_time = calculate_next_run(job->cron_expr, now);
            }
            job = job->next;
        }
        
        pthread_mutex_unlock(&task->job_mutex);
        
        sleep(60);  // 每分钟检查一次
    }
    
    return 0;
}
```

---

### 项目4：数据库连接池 ⭐⭐⭐⭐⭐
**学习目标：** 高级资源管理和并发控制

**功能要求：**
- 管理多个数据库连接
- 连接复用和超时处理
- 支持事务和连接健康检查
- 连接统计和监控

**核心技术点：**
- 连接池设计模式
- 条件变量和信号量
- 数据库API使用(SQLite/MySQL)
- 内存管理优化

**代码框架：**
```c
typedef struct Connection {
    void* db_handle;        // 数据库连接句柄
    bool in_use;            // 是否正在使用
    time_t last_used;       // 最后使用时间
    bool is_healthy;        // 连接是否健康
} Connection;

typedef struct {
    TaskBase base;
    Connection* connections;
    int pool_size;
    int active_count;
    pthread_mutex_t pool_mutex;
    pthread_cond_t pool_cond;
    char connection_string[256];
} ConnectionPoolTask;

// 获取连接
Connection* pool_get_connection(ConnectionPoolTask* pool, int timeout_ms) {
    pthread_mutex_lock(&pool->pool_mutex);
    
    // 等待可用连接
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_nsec += timeout_ms * 1000000;
    
    while (pool->active_count >= pool->pool_size) {
        if (pthread_cond_timedwait(&pool->pool_cond, 
                                   &pool->pool_mutex, &timeout) != 0) {
            pthread_mutex_unlock(&pool->pool_mutex);
            return NULL;  // 超时
        }
    }
    
    // 查找可用连接
    for (int i = 0; i < pool->pool_size; i++) {
        if (!pool->connections[i].in_use) {
            pool->connections[i].in_use = true;
            pool->connections[i].last_used = time(NULL);
            pool->active_count++;
            pthread_mutex_unlock(&pool->pool_mutex);
            return &pool->connections[i];
        }
    }
    
    pthread_mutex_unlock(&pool->pool_mutex);
    return NULL;
}
```

---

## 🛠️ 开发工具和调试技巧

### 推荐开发环境
```bash
# 安装必要工具
sudo apt-get install build-essential cmake gdb valgrind
sudo apt-get install libsqlite3-dev  # 如果做数据库项目

# VS Code插件推荐
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
```

### 调试技巧
```bash
# 编译调试版本
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# 使用GDB调试
gdb ./your_program
(gdb) break main
(gdb) run
(gdb) next

# 内存泄漏检查
valgrind --leak-check=full ./your_program

# 性能分析
perf record ./your_program
perf report
```

### 单元测试框架
```c
// 简单的单元测试
#include <assert.h>

void test_system_monitor() {
    SystemMonitorTask task = {0};
    
    // 测试初始化
    TaskConfig config = {"test", TASK_PRIORITY_NORMAL, 0, 0, false, false, NULL};
    assert(monitor_initialize((TaskBase*)&task, &config) == 0);
    
    // 测试CPU获取
    float cpu = get_cpu_usage();
    assert(cpu >= 0.0 && cpu <= 100.0);
    
    printf("✅ 系统监控测试通过\n");
}
```

---

## 📊 项目评估标准

### 代码质量检查清单
- [ ] 内存管理正确（无泄漏）
- [ ] 线程安全（正确使用锁）
- [ ] 错误处理完整
- [ ] 资源清理彻底
- [ ] 配置参数可调
- [ ] 日志输出清晰
- [ ] 代码注释充分

### 性能测试
```bash
# 压力测试HTTP服务器
ab -n 1000 -c 10 http://localhost:8080/

# 监控系统资源
top -p $(pgrep your_program)

# 网络连接测试  
netstat -an | grep :8080
```

---

## 🎯 进阶挑战

### 挑战1：插件热重载
实现不停机的插件更新机制。

### 挑战2：分布式任务
让多个StartTool实例协同工作。

### 挑战3：Web管理界面
创建基于HTTP的管理界面。

### 挑战4：容器化部署
支持Docker和Kubernetes部署。

---

## 📚 相关学习资源

### 系统编程
- 《UNIX网络编程》- 网络编程经典
- 《Linux系统编程》- 系统调用详解

### 并发编程  
- 《并发编程实战》- 多线程设计模式
- POSIX Threads文档

### 网络编程
- HTTP/1.1 RFC文档
- Socket编程教程

---

**完成这些项目后，你将具备独立开发复杂系统级应用的能力！** 🚀
