# MIT6.S081-2020

This repository is a fork of the official repository.

Official Website: <https://pdos.csail.mit.edu/6.S081/2020/>

The official lectures are in English. For Chinese students, please refer to the following resources:
<https://github.com/huihongxiao/MIT6.S081>

## Setting Up Environments

```bash
sudo cp /etc/apt/sources.list /etc/apt/sources.list.bak
cat <<EOF | sudo tee /etc/apt/sources.list
# The source code repositories are commented out by default to speed up `apt update`. Uncomment if needed.
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse
# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse
# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-updates main restricted universe multiverse
deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse
# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-backports main restricted universe multiverse

deb http://security.ubuntu.com/ubuntu/ focal-security main restricted universe multiverse
# deb-src http://security.ubuntu.com/ubuntu/ focal-security main restricted universe multiverse

# Pre-release software sources are not recommended to be enabled.
# deb https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-proposed main restricted universe multiverse
# deb-src https://mirrors.tuna.tsinghua.edu.cn/ubuntu/ focal-proposed main restricted universe multiverse
EOF
```

Update the package list:

```bash
sudo apt-get clean all
sudo apt-get update
```

## Starting the Machine

```bash
make qemu
```

## Running Test Cases

```bash
make grade
```

## Debugging the Code

Start the debugging environment:

```bash
make qemu-gdb
```

Then, open a new terminal and run the following commands:

```bash
echo "set auto-load safe-path /" >> ~/.gdbinit
cd MIT6.S081-2020
gdb-multiarch
```