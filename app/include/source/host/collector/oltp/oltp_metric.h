#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <opentelemetry/metrics/async_instruments.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/observer_result.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/variant.h>
#include <os/unix_util.h>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace App::Source::Host::Collector::Oltp {

struct MetricData {
    std::variant<int64_t, double> data;
    std::map<std::string, std::string> labels;
};

using SingleValue = std::variant<int64_t, double>;
using MultiValue = std::vector<MetricData>;

using SingleValueCallback = std::function<void(SingleValue&)>;
using MultiValueCallback = std::function<void(MultiValue&)>;

using CallbackVariant = std::variant<SingleValueCallback, MultiValueCallback>;

class MetricCollector {
public:
    template <class T>
    static std::unique_ptr<MetricCollector>
    Create(opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter, const std::string& company_uuid,
           std::string_view name, CallbackVariant callback) {
        auto* instance = new MetricCollector(meter, company_uuid, std::move(callback));
        std::unique_ptr<MetricCollector> ret(instance);
        if constexpr (std::is_integral_v<T>) {
            ret->object_ = meter->CreateInt64ObservableGauge(name.data());
        } else if constexpr (std::is_floating_point_v<T>) {
            ret->object_ = meter->CreateDoubleObservableGauge(name.data());
        } else {
            static_assert(false, "Unsupported data type");
        }
        return ret;
    }

    void Start() {
        object_->AddCallback(Callback, this);
    }

    void Stop() {
        object_->RemoveCallback(Callback, this);
    }

    MetricCollector() = delete;

private:
    static void Callback(opentelemetry::metrics::ObserverResult observer_result, void* data) {
        auto ptr = static_cast<MetricCollector*>(data);
        if (ptr == nullptr) {
            return;
        }

        ptr->Report(std::move(observer_result));
    }

    void ReportSingleValue(opentelemetry::metrics::ObserverResult observer_result, SingleValue value) {
        std::visit(
            [&](auto arg) {
                using UnderlyingType = decltype(arg);
                if constexpr (std::is_same_v<int64_t, UnderlyingType>) {
                    opentelemetry::nostd::get<
                        opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<int64_t>>>(
                        observer_result)
                        ->Observe(arg);
                } else if constexpr (std::is_same_v<double, UnderlyingType>) {
                    opentelemetry::nostd::get<
                        opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<double>>>(
                        observer_result)
                        ->Observe(arg);
                } else {
                    static_assert(false, "invalid type");
                }
            },
            value);
    }

    void ReportMultiValue(opentelemetry::metrics::ObserverResult observer_result, MultiValue& values) {
        for (auto& value : values) {
            std::visit(
                [&](auto arg) {
                    using UnderlyingType = decltype(arg);
                    if constexpr (std::is_same_v<int64_t, UnderlyingType>) {
                        opentelemetry::nostd::get<
                            opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<int64_t>>>(
                            observer_result)
                            ->Observe(arg, value.labels);
                    } else if constexpr (std::is_same_v<double, UnderlyingType>) {
                        opentelemetry::nostd::get<
                            opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObserverResultT<double>>>(
                            observer_result)
                            ->Observe(arg, value.labels);
                    } else {
                        static_assert(false, "invalid type");
                    }
                },
                value.data);
        }
    }

    void Report(opentelemetry::metrics::ObserverResult observer_result) {
        std::visit(
            [&](auto callback) {
                using UnderlyingType = decltype(callback);
                if constexpr (std::is_same_v<UnderlyingType, SingleValueCallback>) {
                    SingleValue value;
                    callback(value);
                    ReportSingleValue(observer_result, value);
                } else if constexpr (std::is_same_v<UnderlyingType, MultiValueCallback>) {
                    MultiValue multi_values;
                    callback(multi_values);
                    ReportMultiValue(observer_result, multi_values);
                } else {
                    static_assert(false, "ValueType");
                }
            },
            callback_);
    }

    explicit MetricCollector(opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter,
                             const std::string& company_uuid, CallbackVariant callback)
        : company_uuid_(company_uuid), callback_(std::move(callback)), meter_(std::move(meter)) {
        host_object_id_ = std::to_string(Core::OS::getMachineId());
    }

private:
    std::string host_object_id_;
    std::string company_uuid_;
    CallbackVariant callback_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::Meter> meter_;
    opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> object_;
};

} // namespace App::Source::Host::Collector::Oltp
