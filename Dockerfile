FROM ubuntu:20.04

USER root

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

# Set mirrors to Aliyun for faster package downloads
RUN cat <<EOF | tee /etc/apt/sources.list
deb http://mirrors.aliyun.com/ubuntu-ports/ focal main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu-ports/ focal main restricted universe multiverse

deb http://mirrors.aliyun.com/ubuntu-ports/ focal-security main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu-ports/ focal-security main restricted universe multiverse

deb http://mirrors.aliyun.com/ubuntu-ports/ focal-updates main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu-ports/ focal-updates main restricted universe multiverse

# deb http://mirrors.aliyun.com/ubuntu-ports/ focal-proposed main restricted universe multiverse
# deb-src http://mirrors.aliyun.com/ubuntu-ports/ focal-proposed main restricted universe multiverse

deb http://mirrors.aliyun.com/ubuntu-ports/ focal-backports main restricted universe multiverse
deb-src http://mirrors.aliyun.com/ubuntu-ports/ focal-backports main restricted universe multiverse
EOF

# Install necessary packages
RUN apt-get update && apt-get install -y \
    git \
    build-essential \
    gdb-multiarch \
    qemu-system-misc \
    gcc-riscv64-linux-gnu \
    binutils-riscv64-linux-gnu \
    ca-certificates \
    curl \
    && apt-get clean
