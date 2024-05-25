#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        const char *str = "expect 1 argument, but find none\n";
        write(1, str, strlen(str));
        exit(1);
    }
    int t = atoi(argv[1]);
    sleep(t);
    exit(0);
}