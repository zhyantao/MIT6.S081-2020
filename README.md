# MIT6.S081-2020

This repository is a fork of the official repository <https://pdos.csail.mit.edu/6.S081/2020/>.

## Resources

- [MIT 6.S081: Operating System Engineering](https://csdiy.wiki/%E6%93%8D%E4%BD%9C%E7%B3%BB%E7%BB%9F/MIT6.S081/#xv6)
- [PKUFlyingPig at GitHub](https://github.com/PKUFlyingPig/MIT6.S081-2020fall)
- [Miigon's blog](https://blog.miigon.net/categories/mit6-s081/)
- [huihongxiao at GitHub](https://github.com/huihongxiao/MIT6.S081)

## Environments

```bash
docker build -t mit-os-lab .
```

## How to run?

```bash
docker run -it \
    --name mit-os-lab \
    -v ~/.gitconfig:/root/.gitconfig:ro \
    -v ~/.ssh/:/root/.ssh/:ro \
    -v $(pwd):/workspace \
    mit-os-lab \
    /bin/bash
```

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
file kernel/kernel
b main
r
```
