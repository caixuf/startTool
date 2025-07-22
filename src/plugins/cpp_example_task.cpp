#include "task_interface.h"
#include "cpp_task_wrapper.h"
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <mutex>

/**
 * C++任务基类 - 提供C++友好的接口
 */
class CppTaskBase {
public:
    CppTaskBase(const std::string& name, const std::string& description) 
        : name_(name), description_(description), should_stop_(false) {}
    
    virtual ~CppTaskBase() = default;
    
    // 纯虚函数 - 子类必须实现
    virtual bool initialize(const std::string& config_data) = 0;
    virtual void execute() = 0;
    
    // 虚函数 - 子类可选重写
    virtual void cleanup() {}
    virtual bool health_check() { return true; }
    virtual void handle_signal(int signal) { (void)signal; }
    virtual std::string get_status() { return "Running"; }
    
    // 工具函数
    bool should_stop() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return should_stop_; 
    }
    
    void set_stop_flag(bool stop) { 
        std::lock_guard<std::mutex> lock(mutex_);
        should_stop_ = stop; 
    }
    
    const std::string& get_name() const { return name_; }
    const std::string& get_description() const { return description_; }
    
    void log(const std::string& message) {
        if (log_callback_) {
            log_callback_(LOG_LEVEL_INFO, message.c_str());
        } else {
            std::cout << "[" << name_ << "] " << message << std::endl;
        }
    }
    
    void set_log_callback(LogCallback callback) { log_callback_ = callback; }

protected:
    std::string name_;
    std::string description_;
    mutable std::mutex mutex_;
    bool should_stop_;
    LogCallback log_callback_ = nullptr;
};

/**
 * C++示例任务 - 演示基本功能
 */
class CppExampleTask : public CppTaskBase {
public:
    CppExampleTask() : CppTaskBase("cpp_example", "C++示例任务"), counter_(0), work_interval_(3) {}
    
    bool initialize(const std::string& config_data) override {
        log("初始化C++示例任务");
        
        // 解析配置（简单示例）
        if (!config_data.empty()) {
            log("配置数据: " + config_data);
        }
        
        counter_ = 0;
        log("C++示例任务初始化完成");
        return true;
    }
    
    void execute() override {
        log("C++示例任务开始执行");
        
        while (!should_stop()) {
            counter_++;
            
            std::string message = "执行第 " + std::to_string(counter_) + " 次 - 使用C++特性!";
            log(message);
            
            // 演示C++特性：使用STL容器和算法
            std::vector<int> data;
            for (int i = 0; i < 5; ++i) {
                data.push_back(i * counter_);
            }
            
            // 使用C++11的范围for循环
            std::string data_str = "数据: ";
            for (const auto& value : data) {
                data_str += std::to_string(value) + " ";
            }
            log(data_str);
            
            // 使用智能指针演示
            auto smart_data = std::make_unique<std::string>("智能指针数据 " + std::to_string(counter_));
            log(*smart_data);
            
            // 休眠 - 使用C++11的chrono库
            for (int i = 0; i < work_interval_ && !should_stop(); ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        
        log("C++示例任务执行完成，总执行次数: " + std::to_string(counter_));
    }
    
    void cleanup() override {
        log("清理C++示例任务");
        counter_ = 0;
        log("C++示例任务清理完成");
    }
    
    bool health_check() override {
        // 简单的健康检查
        return counter_ < 1000; // 防止无限循环
    }
    
    std::string get_status() override {
        std::lock_guard<std::mutex> lock(mutex_);
        return "C++任务状态: 运行中, 执行次数: " + std::to_string(counter_) + 
               ", 工作间隔: " + std::to_string(work_interval_) + "秒";
    }

private:
    int counter_;
    int work_interval_;
};

// ==============================================================================
// C接口包装 - 将C++类包装为C接口
// ==============================================================================

struct CppTaskWrapper {
    TaskBase base;                    // C基类
    std::unique_ptr<CppTaskBase> cpp_task; // C++任务对象
};

// 静态实例（演示用）
static std::unique_ptr<CppExampleTask> g_cpp_task;

// C接口实现
static int cpp_task_initialize(TaskBase* base_task) {
    CppTaskWrapper* wrapper = reinterpret_cast<CppTaskWrapper*>(base_task);
    
    // 创建C++任务对象
    g_cpp_task = std::make_unique<CppExampleTask>();
    wrapper->cpp_task.reset(g_cpp_task.get());
    
    // 设置日志回调
    g_cpp_task->set_log_callback([](LogLevel level, const char* message) {
        std::cout << "[C++ LOG] " << message << std::endl;
    });
    
    // 调用C++的初始化方法
    std::string config_data;
    if (base_task->config.custom_config) {
        config_data = static_cast<const char*>(base_task->config.custom_config);
    }
    
    return g_cpp_task->initialize(config_data) ? 0 : -1;
}

static int cpp_task_execute(TaskBase* base_task) {
    CppTaskWrapper* wrapper = reinterpret_cast<CppTaskWrapper*>(base_task);
    
    if (!g_cpp_task) {
        return -1;
    }
    
    try {
        g_cpp_task->execute();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "C++任务执行异常: " << e.what() << std::endl;
        return -1;
    }
}

static void cpp_task_cleanup(TaskBase* base_task) {
    CppTaskWrapper* wrapper = reinterpret_cast<CppTaskWrapper*>(base_task);
    
    if (g_cpp_task) {
        g_cpp_task->cleanup();
        g_cpp_task.reset();
    }
}

static bool cpp_task_health_check(TaskBase* base_task) {
    CppTaskWrapper* wrapper = reinterpret_cast<CppTaskWrapper*>(base_task);
    
    return g_cpp_task ? g_cpp_task->health_check() : false;
}

static void cpp_task_handle_signal(TaskBase* base_task, int signal) {
    CppTaskWrapper* wrapper = reinterpret_cast<CppTaskWrapper*>(base_task);
    
    if (g_cpp_task) {
        g_cpp_task->handle_signal(signal);
        if (signal == SIGTERM || signal == SIGINT) {
            g_cpp_task->set_stop_flag(true);
        }
    }
}

static int cpp_task_get_status(TaskBase* base_task, char* buffer, size_t size) {
    CppTaskWrapper* wrapper = reinterpret_cast<CppTaskWrapper*>(base_task);
    
    if (!g_cpp_task || !buffer || size == 0) {
        return -1;
    }
    
    std::string status = g_cpp_task->get_status();
    strncpy(buffer, status.c_str(), size - 1);
    buffer[size - 1] = '\0';
    
    return static_cast<int>(status.length());
}

// C虚函数表
static const TaskInterface cpp_task_vtable = {
    .initialize = cpp_task_initialize,
    .execute = cpp_task_execute,
    .cleanup = cpp_task_cleanup,
    .pause = nullptr,       // 使用默认实现
    .resume = nullptr,      // 使用默认实现
    .handle_signal = cpp_task_handle_signal,
    .health_check = cpp_task_health_check,
    .get_status = cpp_task_get_status
};

// ==============================================================================
// C导出函数 - 供动态库加载使用
// ==============================================================================

extern "C" {

CppTaskWrapper* cpp_example_task_create(const TaskConfig* config) {
    if (!config) {
        return nullptr;
    }
    
    // 分配包装器
    CppTaskWrapper* wrapper = static_cast<CppTaskWrapper*>(malloc(sizeof(CppTaskWrapper)));
    if (!wrapper) {
        return nullptr;
    }
    
    // 初始化C基类
    if (task_base_init(&wrapper->base, &cpp_task_vtable, config) != 0) {
        free(wrapper);
        return nullptr;
    }
    
    // 初始化C++部分
    new(&wrapper->cpp_task) std::unique_ptr<CppTaskBase>();
    
    return wrapper;
}

void cpp_example_task_destroy(CppTaskWrapper* wrapper) {
    if (!wrapper) {
        return;
    }
    
    // 销毁C++对象
    wrapper->cpp_task.~unique_ptr<CppTaskBase>();
    
    // 销毁C基类
    task_base_destroy(&wrapper->base);
    
    free(wrapper);
}

TaskBase* cpp_example_task_get_base(CppTaskWrapper* wrapper) {
    return wrapper ? &wrapper->base : nullptr;
}

// 插件接口 - 用于动态加载
ProcessInterface* get_process_interface(void);
uint32_t get_interface_version(void);

} // extern "C"
