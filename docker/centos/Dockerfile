FROM centos:centos7.9.2009 as builder

RUN yum -y update

RUN yum -y install gcc

RUN yum -y install gcc-c++

RUN yum -y install wget

RUN wget https://cmake.org/files/v3.26/cmake-3.26.4.tar.gz

RUN tar -xzvf cmake-3.26.4.tar.gz

RUN yum install -y make

RUN yum install -y openssl-devel

RUN cd cmake-3.26.4 && ./configure --prefix=/usr && make -j4 && make install

RUN yum install -y rh-varnish6-jemalloc-devel.x86_64
RUN yum install -y autoconf
RUN yum install -y automake
RUN yum install -y libtool
RUN yum install -y python3
RUN yum install -y byacc
RUN yum install -y flex
RUN yum install -y pam-devel
RUN yum install -y systemd-devel
RUN yum install -y libuuid-devel
RUN yum install -y gdb
RUN yum -y install centos-release-scl
RUN yum -y install devtoolset-9-gcc devtoolset-9-gcc-c++ devtoolset-9-binutils
    RUN scl enable devtoolset-9 bash
RUN source /opt/rh/devtoolset-9/enable