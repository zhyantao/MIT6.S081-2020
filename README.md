# MIT6.S081-2020

This repository is a fork of the official repository <https://pdos.csail.mit.edu/6.S081/2020/>.

## Resources

- [MIT 6.S081: Operating System Engineering](https://csdiy.wiki/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/MIT6.S081/#xv6)
- [PKUFlyingPig at GitHub](https://github.com/PKUFlyingPig/MIT6.S081-2020fall)
- [Miigon's blog](https://blog.miigon.net/categories/mit6-s081/)
- [huihongxiao at GitHub](https://github.com/huihongxiao/MIT6.S081)

## Environments

All guides is based on Ubuntu-20.04.

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

Install the necessory packages:

```bash
sudo apt-get install git build-essential gdb-multiarch qemu-system-misc \
    gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

## How to run?

Run the OS:

```bash
make qemu
```

Exit the OS:

```bash
Ctrl + A
x
```

Check results:

```bash
make grade
```

## Debugging

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
