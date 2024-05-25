#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
    int p1[2];
    int p2[2];
    char buf[100];
    pipe(p1); // parent -> child
    pipe(p2); // child -> parent

    int pid = fork();
    if (pid == 0)
    {
        // (2) child: print "<pid>: received ping"
        printf("%d: received ping\n", getpid());

        // (3) child: receive a byte from parent and send back
        int n = read(p1[0], buf, sizeof(buf));
        write(p2[1], buf, n);
    }
    else
    {
        // (1) parent: send a byte to child
        write(p1[1], "A", 1);

        // (4) parent: receive a byte from child
        read(p2[0], buf, sizeof(buf));

        // (5) parent: print "<pid>: received pong"
        printf("%d: received pong\n", getpid());
    }

    exit(0);
}