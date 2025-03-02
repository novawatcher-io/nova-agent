#!/bin/bash
set -e

# library specific options
declare -A LIB_OPTIONS

# library check file, if the file exists, this script will not build the library again.
# this method may not be very robust, if two different libraries have the same file name, it will cause problems.
declare -A LIB_FILES

# universal library options
UNIVERSAL_OPTIONS=""

setup_table() {
    UNIVERSAL_OPTIONS="-DINCLUDE_DIRECTORIES=${THIRD_PARTY_INCLUDE_DIR} -DCMAKE_LIBRARY_PATH=${THIRD_PARTY_LINK_DIR} -DOPENSSL_ROOT_DIR=${THIRD_PARTY_INATALL_DIR} -DOpenSSL_DIR=${THIRD_PARTY_INATALL_DIR}/lib64/cmake/OpenSSL -DCMAKE_INSTALL_PREFIX=$THIRD_PARTY_INATALL_DIR -DCMAKE_CXX_STANDARD=17 -DCMAKE_BUILD_TYPE=Release"

    LIB_OPTIONS["fmt"]="-DFMT_TEST=OFF"
    LIB_OPTIONS["protobuf"]="-Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_ABSL_PROVIDER=package -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    # -DgRPC_SSL_PROVIDER=package"
    LIB_OPTIONS["grpc"]="-DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DgRPC_USE_SYSTEMD=OFF -DgRPC_PROTOBUF_PROVIDER=package -DgRPC_ABSL_PROVIDER=package -DgRPC_ZLIB_PROVIDER=package -DgRPC_SSL_PROVIDER=package -DOPENSSL_ROOT_DIR=$THIRD_PARTY_INATALL_DIR -DOPENSSL_INCLUDE_DIR=$THIRD_PARTY_INATALL_DIR/include -DOPENSSL_LIBRARIES=$THIRD_PARTY_INATALL_DIR/lib64;$THIRD_PARTY_INATALL_DIR/lib"
    LIB_OPTIONS["opentelemetry-cpp"]="-DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DWITH_BENCHMARK=OFF -DWITH_HTTP_CLIENT_CURL=OFF -DWITH_OTLP_GRPC=ON -DWITH_ABSEIL=ON"
    LIB_OPTIONS["stduuid"]="-DUUID_BUILD_TESTS=OFF"
    LIB_OPTIONS["abseil-cpp"]="-DABSL_BUILD_TESTING=OFF -DABSL_PROPAGATE_CXX_STD=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON"
    LIB_OPTIONS["libevent"]="-DEVENT__DISABLE_TESTS=ON -DEVENT__LIBRARY_TYPE=STATIC -DEVENT__DISABLE_MBEDTLS=ON -DOPENSSL_INCLUDE_DIR=$THIRD_PARTY_INATALL_DIR/include -DOPENSSL_LIBRARIES=$THIRD_PARTY_INATALL_DIR/lib64;$THIRD_PARTY_INATALL_DIR/lib"
    LIB_OPTIONS["cpu_features"]="-DBUILD_TESTING=OFF"
    LIB_OPTIONS["spdlog"]="-DSPDLOG_BUILD_PIC=ON -DSPDLOG_FMT_EXTERNAL=ON -DSPDLOG_INSTALL=ON"
    LIB_OPTIONS["rapidjson"]="-DRAPIDJSON_BUILD_DOC=OFF -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF -DRAPIDJSON_BUILD_THIRDPARTY_GTEST=ON -DRAPIDJSON_BUILD_CXX17=ON"
    LIB_OPTIONS["libxml2"]="-DBUILD_SHARED_LIBS=OFF -DLIBXML2_WITH_DEBUG=OFF -DLIBXML2_WITH_HTML=OFF -DLIBXML2_WITH_HTTP=OFF -DLIBXML2_WITH_PROGRAMS=OFF -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_TESTS=OFF"
    LIB_OPTIONS["civetweb"]="-DCIVETWEB_BUILD_TESTING=OFF -DCIVETWEB_ENABLE_SERVER_EXECUTABLE=OFF -DCIVETWEB_DISABLE_CACHING=OFF -DCIVETWEB_ENABLE_ASAN=OFF -DCIVETWEB_INSTALL_EXECUTABLE=OFF -DCMAKE_BUILD_TYPE=Release -DCIVETWEB_ENABLE_CXX=ON"
    LIB_OPTIONS["prometheus-cpp"]="-DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF -DUSE_THIRDPARTY_LIBRARIES=OFF -DTHIRDPARTY_CIVETWEB_WITH_SSL=OFF -DENABLE_PUSH=OFF -DCMAKE_BUILD_TYPE=Release"
    LIB_OPTIONS["curl"]="-DCMAKE_C_FLAGS=-fPIC -DBUILD_SHARED_LIBS=OFF -DCURL_USE_LIBPSL=OFF  -DOPENSSL_ROOT_DIR=${THIRD_PARTY_INATALL_DIR} -DOpenSSL_DIR=${THIRD_PARTY_INATALL_DIR}/lib64/cmake/OpenSSL -DCURL_USE_OPENSSL=ON"
    LIB_OPTIONS["kubernetes-client/kubernetes"]="-DCMAKE_C_FLAGS=-fPIC -DBUILD_SHARED_LIBS=OFF"

    LIB_FILES["abseil-cpp"]="libabsl_base.a"
    LIB_FILES["googletest"]="libgtest.a"
    LIB_FILES["protobuf"]="libprotobuf.a"
    LIB_FILES["grpc"]="libgrpc++.a"
    LIB_FILES["libevent"]="libevent.a"
    LIB_FILES["fmt"]="libfmt.a"
    LIB_FILES["libcgroup"]="libcgroup.a"
    LIB_FILES["opentelemetry-cpp"]="libopentelemetry_common.a"
    LIB_FILES["stduuid"]="cmake/stduuid/stduuid-targets.cmake"
    LIB_FILES["cpu_features"]="libcpu_features.a"
    LIB_FILES["spdlog"]="libspdlog.a"
    LIB_FILES["rapidjson"]="cmake/RapidJSON/RapidJSONConfig.cmake"
    LIB_FILES["libxml2"]="libxml2.a"
    LIB_FILES["civetweb"]="libcivetweb.a"
    LIB_FILES["prometheus-cpp"]="libprometheus-cpp-core.a"
    LIB_FILES["libyaml"]="libyaml.a"
    LIB_FILES["kubernetes-client/kubernetes"]="libkubernetes.a"
    LIB_FILES["curl"]="libcurl.a"
}

format_print() {
    printf "%-17s installed, checked with: %s\n" "$1" "$2"
}

build_target_with_cmake() {
    local BUILD_ROOT=$1
    local LIB=$2
    # check if already installed
    if [ -f $INSTALL_PREFIX/lib/${LIB_FILES[$LIB]} ]; then
        format_print "$LIB" "$INSTALL_PREFIX/lib/${LIB_FILES[$LIB]}"
        return
    fi
    echo "Building $LIB..."
    local BUILD_OPTIONS=$UNIVERSAL_OPTIONS
    # check if there are additional options
    if [[ -v LIB_OPTIONS[$LIB] ]]; then
        BUILD_OPTIONS="${BUILD_OPTIONS} ${LIB_OPTIONS[$LIB]}"
    fi

    local LIB_SRC_DIR=$(pwd)/$LIB
    local LIB_BUILD_DIR=$BUILD_ROOT/$LIB-build
    mkdir -p $LIB_BUILD_DIR
    cmake -S $LIB_SRC_DIR -B $LIB_BUILD_DIR $BUILD_OPTIONS
    cmake --build $LIB_BUILD_DIR -j $(nproc)
    cmake --install $LIB_BUILD_DIR
    echo $LIB_BUILD_DIR
}

build_libcgroup() {
    if [ -f $INSTALL_PREFIX/lib/libcgroup.a ]; then
        format_print libcgroup $INSTALL_PREFIX/lib/libcgroup.a
        return
    fi
    echo "Building libcgroup..."
    cd libcgroup
    autoreconf -fi
    ./configure --prefix=$INSTALL_PREFIX --enable-static --disable-shared
    make -j $(nproc)
    make install
}

build_openssl() {
    # check if INSTALL_PREFIX is /usr or empty
    if [ -z "$INSTALL_PREFIX" ]; then
        echo "INSTALL_PREFIX is empty, please specify a directory."
        exit 1
    fi
    if [ "$INSTALL_PREFIX" == "/usr" ]; then
        echo "Cannot install openssl to /usr, that will break everything, please specify a different directory."
        exit 1
    fi
    if [ -f $INSTALL_PREFIX/lib64/libssl.a ]; then
        format_print openssl $INSTALL_PREFIX/lib64/libssl.a
        return
    fi
    echo "Building openssl... $INSTALL_PREFIX"
    cd openssl
    echo "./config --prefix=$INSTALL_PREFIX --threads --openssldir=$INSTALL_PREFIX --static -debug –g3"
    ./config --prefix=$INSTALL_PREFIX --openssldir=$INSTALL_PREFIX --static -static -d -fPIC
    make -j $(nproc)
    make install
}

build_hwloc() {
    if [ -f $INSTALL_PREFIX/lib/libhwloc.a ]; then
        format_print hwloc $INSTALL_PREFIX/lib/libhwloc.a
        return
    fi
    echo "Building hwloc..."
    cd hwloc
    ./autogen.sh
    CFLAGS="-fPIC $CFLAGS" CXXFLAGS="-fPIC $CXXFLAGS" \
    ./configure --prefix=$INSTALL_PREFIX --enable-static --disable-shared --disable-opencl --disable-cuda --disable-nvml --disable-cairo --disable-libudev --disable-pci
    make -j $(nproc)
    make install
}

build_all() {
    local WORKING_DIR=$1
    local BUILD_ROOT=$2
    cd ${WORKING_DIR}/third_party/libcore/third_party
    (build_openssl)
    (build_libcgroup)
    (build_target_with_cmake $BUILD_ROOT "libevent")
    (build_target_with_cmake $BUILD_ROOT "fmt")
    (build_target_with_cmake $BUILD_ROOT "spdlog")

    cd ${WORKING_DIR}/third_party
    (build_target_with_cmake $BUILD_ROOT "abseil-cpp")
    (build_target_with_cmake $BUILD_ROOT "protobuf")
    (build_target_with_cmake $BUILD_ROOT "grpc")
    (build_target_with_cmake $BUILD_ROOT "opentelemetry-cpp")
    (build_target_with_cmake $BUILD_ROOT "stduuid")
    (build_target_with_cmake $BUILD_ROOT "cpu_features")
    (build_target_with_cmake $BUILD_ROOT "googletest")
    (build_target_with_cmake $BUILD_ROOT "rapidjson")
    (build_target_with_cmake $BUILD_ROOT "libxml2")
    (build_target_with_cmake $BUILD_ROOT "civetweb")
    (build_target_with_cmake $BUILD_ROOT "prometheus-cpp")
    (build_target_with_cmake $BUILD_ROOT "libyaml")
    (build_target_with_cmake $BUILD_ROOT "curl")
    (build_target_with_cmake $BUILD_ROOT "kubernetes-client/kubernetes")
    (build_hwloc)
}

main() {
    # get location of current script
    local SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

    # backup original arguments
    local original_args=("$@")
    # clear argument for setup_env.sh
    set --
    source $SCRIPT_DIR/setup_env.sh
    set -- "${original_args[@]}"
    setup_table

    local WORKING_DIR=$(pwd)

    # specify where project is built
    export BUILD_ROOT=/tmp/third_party_build
    mkdir -p $BUILD_ROOT

    # this script accept one argument, the library name to build
    # this will make CI log more readable
    if [ $# -eq 1 ]; then
        case "$1" in
        "fmt" | "libevent")
            cd $WORKING_DIR/third_party/libcore/third_party
            build_target_with_cmake $BUILD_ROOT $1
            ;;
        "abseil-cpp" | "protobuf" | "grpc" | "opentelemetry-cpp" | "stduuid" | "googletest" | "cpu_features")
            cd $WORKING_DIR/third_party
            build_target_with_cmake $BUILD_ROOT $1
            ;;
        "libcgroup")
            cd $WORKING_DIR/third_party/libcore/third_party
            build_libcgroup
            ;;
        "openssl")
            echo "building openssl"
            cd $WORKING_DIR/third_party/libcore/third_party
            build_openssl
            ;;
        *)
            echo "unsupported library: $1"
            exit 1
            ;;
        esac
    else
        build_all $WORKING_DIR $BUILD_ROOT
    fi
}

main "$@"
