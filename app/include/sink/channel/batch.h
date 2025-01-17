#pragma once

#include <component/api.h>
#include <memory>

namespace App {
namespace Sink {
namespace Channel {
class Batch : public Core::Component::Batch {
public:
    explicit Batch(std::vector<std::unique_ptr<Core::Component::EventData>>& events) : events_{std::move(events)} {
    }

    Batch() {
    }

    std::map<std::string, std::string> meta() final {
        return {};
    }
    std::vector<std::unique_ptr<Core::Component::EventData>>& events() final {
        return events_;
    }
    void fill(std::map<std::string, std::string> ,
              std::vector<std::unique_ptr<Core::Component::EventData>>& events) override {
        events_.swap(events);
    }
    void Release() final {
        events_.clear();
    }

private:
    std::vector<std::unique_ptr<Core::Component::EventData>> events_;
};
} // namespace Channel
} // namespace Sink
} // namespace App
