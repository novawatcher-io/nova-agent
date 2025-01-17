#!/bin/sh
echo '/tmp/core.%e.%p' > /proc/sys/kernel/core_pattern
ulimit -c unlimited
export ASAN_OPTIONS="disable_coredump=0:unmap_shadow_on_exit=1:abort_on_error=1"
/usr/bin/trace-agent -c /trace-agent/config.json
