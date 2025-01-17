add_library(
    intercept
    chain.cc opentelemetry/log/processor.cc opentelemetry/metric/skywalking/processor.cc
    opentelemetry/trace/skywlking/processor.cc opentelemetry/trace/skywlking/skywalkingproto_to_traces_util.cc)
