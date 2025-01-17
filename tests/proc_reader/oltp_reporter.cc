#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/resources/resource.h>
#include <opentelemetry/sdk/metrics/export/ostream_metric_exporter.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h>
#include <opentelemetry/metrics/provider.h>
#include <chrono>
#include <thread>
#include <iostream>

// Use the OpenTelemetry SDK namespaces
namespace metrics = opentelemetry::metrics;
namespace sdkmetrics = opentelemetry::sdk::metrics;
namespace resource = opentelemetry::sdk::resources;

// Simulate a function to get CPU usage (in practice, you'd replace this with real monitoring code)
double GetCpuUsage()
{
    // Mock value for CPU usage percentage
    return 75.5;
}

int main()
{
    // Create a resource with attributes to identify the instance
    auto resource_attributes = resource::Resource::Create({
        {"host.name", "client-hostname"},             // Unique host name
        {"service.instance.id", "instance-id-123"},   // Unique instance ID
        {"cloud.region", "us-west-2"}                // Region identifier
    });

    // Set up the metric exporter
    auto exporter = std::make_unique<sdkmetrics::OStreamMetricExporter>();

    // Set up the periodic metric reader (flushes data periodically)
    auto periodic_reader = std::make_unique<sdkmetrics::PeriodicExportingMetricReader>(
        std::move(exporter), std::chrono::milliseconds(5000));

    // Create the MeterProvider with resource attributes and the periodic reader
    auto provider = std::make_shared<sdkmetrics::MeterProvider>();
    provider->AddMetricReader(std::move(periodic_reader));

    // Get a meter from the MeterProvider
    auto meter = provider->GetMeter("cpu_usage_meter", "1.0.0");

    // Create a DoubleGauge instrument to report CPU usage
    auto cpu_usage_gauge = meter->CreateDoubleObservableGauge(
        "cpu_usage", "CPU usage percentage", "percentage",
        [](metrics::ObserverResult<double> &result) {
            result.Observe(GetCpuUsage());  // Record the current CPU usage
        }
    );

    // Register the provider (this step makes it global so that all reported metrics will use it)
    metrics::Provider::SetMeterProvider(provider);

    // Simulate the application running and reporting CPU usage
    for (int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
