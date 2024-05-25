#include "kernel/types.h"
#include "user/user.h"

void child_process(int *left_pipe)
{
    close(left_pipe[1]); // close write endian will unblock read

    // for each prime number, create one process that reads
    // from its left neighboor over a pipe and writes to its
    // right neighboor over another pipe
    int prime;
    read(left_pipe[0], &prime, sizeof(int));
    printf("prime %d\n", prime);

    int num;
    if (read(left_pipe[0], &num, sizeof(int)))
    {
        // declare right_pipe before fork()
        // because it must share with child process
        int right_pipe[2];
        pipe(right_pipe);

        int pid = fork();
        if (pid == 0)
        {
            // child process
            child_process(right_pipe);
        }
        else
        {
            // parent process
            close(right_pipe[0]);

            // push back the first prime into child pipe
            if (num % prime != 0)
            {
                write(right_pipe[1], &num, sizeof(int));
            }

            // push back remain numbers into child pipe
            while (read(left_pipe[0], &num, sizeof(int)))
            {
                if (num % prime != 0)
                {
                    write(right_pipe[1], &num, sizeof(int));
                }
            }

            close(left_pipe[0]);
            close(right_pipe[1]); // unblock child process
            wait(0);
        }
    }

    exit(0);
}

int main(void)
{
    // create a pipe that shared with child process
    int left_pipe[2];
    pipe(left_pipe);

    int pid = fork();
    if (pid == 0)
    {
        // child process will be blocked
        // until parent process closes write endian
        child_process(left_pipe);
    }
    else
    {
        // parent process
        close(left_pipe[0]); // close read endian

        // feeds the numbers 2 through 35 into the pipeline
        for (int n = 2; n <= 35; n++)
        {
            write(left_pipe[1], &n, sizeof(int));
        }

        close(left_pipe[1]); // close write endian

        wait(0); // wait child process to be finished
    }
    exit(0);
}