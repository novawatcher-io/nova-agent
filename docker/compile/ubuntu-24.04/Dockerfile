FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# This Dockerfile adds a non-root 'vscode' user with sudo access. However, for Linux,
# this user's GID/UID must match your local user UID/GID to avoid permission issues
# with bind mounts. Update USER_UID / USER_GID if yours is not 1000. See
# https://aka.ms/vscode-remote/containers/non-root-user for details.
ARG USER_UID=1000
ARG USER_GID=1000

# update all packages
RUN apt update -y && \
    apt -y install --no-install-recommends apt-utils dialog && \
    apt -y install \
    autoconf \
    automake \
    bison \
    build-essential \
    cmake \
    flex \
    g++ \
    gcc \
    git \
    iproute2 \
    libgtest-dev \
    libjemalloc-dev \
    libpam0g-dev \
    libsystemd-dev \
    libtool \
    nvidia-cuda-toolkit \
    nvidia-driver-535 \
    pkg-config \
    poppler-utils \
    procps \
    python3 \
    python3-pip \
    sudo \
    uuid-dev \
    zlib1g-dev \
    && apt autoremove -y \
    && apt clean -y

# for ubuntu:24.04, there already have a user named `ubuntu` with uid 1000, we need to delete it. see:
# https://askubuntu.com/questions/1513927/ubuntu-24-04-docker-images-now-includes-user-ubuntu-with-uid-gid-1000
RUN touch /var/mail/ubuntu && chown ubuntu /var/mail/ubuntu && userdel -r ubuntu

RUN groupadd --gid $USER_GID devops \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m devops \
    && echo devops ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/devops \
    && chmod 0440 /etc/sudoers.d/devops

COPY . /tmp/third_party

# Waring: we have openssl in the third party package, if we install openssl to `/usr` that will break everything!
RUN cd /tmp && INSTALL_PREFIX=/opt/trace-agent /tmp/third_party/build_third_party.sh

RUN rm -rf /tmp/third_party && rm -rf /tmp/third_party_build

# Switch back to dialog for any ad-hoc use of apt-get
ENV DEBIAN_FRONTEND=dialog

# switch to user `devops`
USER devops
