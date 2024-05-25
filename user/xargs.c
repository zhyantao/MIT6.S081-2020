#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/stat.h"

int process_extra_args(int *extra_argc, char *extra_argv[])
{
    *extra_argc = 0;

    // read extra arguments from standard input
    char buf[1024];
    int buflen = 0;
    while (read(0, buf + buflen, 1))
    {
        if (buf[buflen] == '\n')
        {
            break;
        }
        buflen++;
    }
    buf[buflen] = '\0';
    if (buflen == 0)
    {
        return 0; // cannot read anything from standard input
    }

    // process extra arguments
    int argvlen = 0;
    extra_argv[0] = malloc(100 * sizeof(char));
    for (int i = 0; i < buflen; i++)
    {
        char c = buf[i];
        if (c == ' ')
        {
            (*extra_argc)++;
            argvlen = 0;
            extra_argv[*extra_argc] = malloc(100 * sizeof(char));
            if (*extra_argc + 1 > MAXARG)
            {
                printf("too many arguments\n");
                exit(1);
            }
        }
        else
        {
            extra_argv[*extra_argc][argvlen++] = c;
        }
    }
    (*extra_argc)++;
    return *extra_argc;
}

int main(int argc, char *argv[])
{
    if (argc > MAXARG || argc < 2)
    {
        printf("error used\n");
        exit(1);
    }

    int extra_argc = 0;
    char *extra_argv[MAXARG - argc]; // pointer array

    // while - process multi-line inputs
    while (process_extra_args(&extra_argc, extra_argv) > 0)
    {
        // pending extra arguments
        char *full_argv[MAXARG];
        for (int i = 0; i < argc; i++)
        {
            full_argv[i] = malloc(100 * sizeof(char));
            strcpy(full_argv[i], argv[i]);
        }
        for (int i = argc; i < argc + extra_argc; i++)
        {
            full_argv[i] = malloc(100 * sizeof(char));
            strcpy(full_argv[i], extra_argv[i - argc]);
        }
        // for (int i = 0; i < argc + extra_argc; i++)
        // {
        //     printf("full argv[%d]: %s\n", i, full_argv[i]);
        // }

        // fork() and exec()
        int pid = fork();
        if (pid == 0)
        {
            // child process
            exec(full_argv[1], &full_argv[1]);
            fprintf(2, "exec: failed\n");
            exit(1);
        }
        else
        {
            // parent process
            wait(0); // wait child process to finish
        }
    }

    exit(0);
}