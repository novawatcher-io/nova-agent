# trace-agent

## 如何构建

目前有支持两种方式构建, 分别对应两种不同的集成第三方库的方式.
1. `package`模式. 这种模式下需要预编译安装第三方库, 该库会默认安装到`.build/install`目录下.
2. `module`模式. 这种模式下会直接从源码编译第三方库, 每次编译的耗时比较长.

### package模式
`package`模式是默认安装模式, 不需要额外设置参数.
1. 首先需要预编译第三方库. 如果已经安装过, 可以跳过这一步. 重复执行脚步不会引起重复安装.
```shell
./build_third_party.sh
```

2. 为当前shell设置环境变量, 使得cmake能够找到第三方库.
```shell
source setup_env.sh
```

3. 使用cmake构建项目.
```shell
mkdir build
cd build
cmake ..
make
```

### `module`模式

`module`模式下需要设置一些CMake参数:
```shell
cmake \
    -Dnova_agent_ABSL_PROVIDER=module \
    -Dnova_agent_PROTOBUF_PROVIDER=module \
    -Dnova_agent_GRPC_PROVIDER=module \
    -Dnova_agent_OPENTELEMETRY_CPP_PROVIDER=module \
    -Dnova_agent_STDUUID_PROVIDER=module \
    -Dnova_agent_LIBCORE_PROVIDER=module \
    -Dlibcore_FMT_PROVIDER=module \
    -Dlibcore_LIBEVENT_PROVIDER=module \
    -DABSL_ENABLE_INSTALL=ON \
    ../
```

### 使用CMakePresets.json

如果你的CMake版本支持CMakePresets.json, 可以直接使用该文件构建项目.
CMakePresets.json 文件可以固定一些CMake参数, 使得构建更加方便.
```shell
cmake --preset=default
```

## 配置文件

```json5
{
  // 集群名称
  "cluster": "default",
  // 上报使用的信息
  "company_uuid": "217057afc5769cbeea96766334f7e7ec",
  // 上报指标时grpc的服务端地址
  "node_report_addr": {
    "host": "81.71.98.26",
    "port": 10050
  },
  // 指标采集周期, 单位为秒
  "sampling_period": 2,
  // 上报oltp指标的服务端地址
  "oltp_exporter_addr": {
    "host": "81.71.98.26",
    "port": 4317
  }
}
```

## TODO

* [ ] fix CI
* [ ] improve code quality
* [ ] add more log for skywalking
* [ ] unify PB report auxiliary struct
