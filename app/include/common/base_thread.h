//
// Created by root on 2024/6/18.
//

#pragma once

#include <functional>
#include <list>
#include <thread>

namespace App::Common {
class BaseThread {
public:
    BaseThread() {
    }

    virtual void addInitCallable(const std::function<void()>& callable) {
        initList.push_back(callable);
    }

    /**
     * @brief 启动线程
     * 做的事情:
     * 1.传入unique_ptr指针
     * 2.创建事件队列
     * 3.创建wakeup子线程的channel
     * 4.启动线程上下文
     *
     * @param object
     * @return true
     * @return false
     */
    bool start() {
        threadHandle = std::make_unique<std::thread>([this] {
            for (auto itr = initList.begin(); itr != initList.end(); itr++) {
                auto& data = *itr;
                data();
            }
        });
        return true;
    }

    /**
     * @brief 停止线程，释放掉资源
     *
     */
    void stop() {
        if (threadHandle && threadHandle->joinable()) {
            threadHandle->join();
            threadHandle.reset();
        }
    }

    virtual ~BaseThread() = default;

private:
    // 线程句柄
    std::unique_ptr<std::thread> threadHandle;

    std::list<std::function<void()>> initList;
};
} // namespace App::Common
