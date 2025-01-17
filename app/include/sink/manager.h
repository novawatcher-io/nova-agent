/**
******************************************************************************
* @file           : manager.h
* @author         : zhanglei
* @brief          : None
* @attention      : None
* @date           : 2024/3/3
******************************************************************************
*/
//
// Created by zhanglei on 2024/3/3.
//

#pragma once
#include <memory>
#include <functional>
#include <non_copyable.h>
#include <non_moveable.h>

namespace App {
namespace Trace {
namespace Sink {
namespace OpenTelemetry {
typedef std::pair<std::shared_ptr<Core::Component::BaseConfig>, std::shared_ptr<Core::Component::Consumer>> functionMap;
typedef std::map<std::string, functionMap> ContainerMap;
typedef std::function<void(const functionMap&)> OnForeachMapClosure;
class Manager :public Core::Noncopyable, Core::Nonmoveable {
public:
    /**
     * @brief 向容器中注册功能
     *
     * @param id
     * @param object
     * @return true
     * @return false
     */
    bool bind(const std::string &id, const functionMap &object) {
        if (id.empty()) {
            return false;
        }
        auto instance = containerMap.find(id);
        if (instance != containerMap.end()) {
            return false;
        }
        containerMap[id] = object;
        return true;
    }

    void foreachContainer(const OnForeachMapClosure& closure) {
        for (auto iter = containerMap.begin(); iter != containerMap.end(); ) {
            closure(iter->second);
            iter++;
        }
    }

    functionMap& get(const std::string &id) {
        if (id.empty()) {
            return emptyObject;
        }

        auto instance = containerMap.find(id);
        if (instance == containerMap.end()) {
            return emptyObject;
        }

        return instance->second;
    }
private:
    /**
     * @brief 容器列表
     *
     */
    ContainerMap containerMap;

    /**
     * @brief 空的对象
     *
     */
    functionMap emptyObject = {};
};
}
}
}
}