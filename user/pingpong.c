#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ 0
#define WRITE 1

int main(int argc, char *argv[])
{
    int p1[2]; // parent write child read
    int p2[2]; // child write parent read
    pipe(p1);
    pipe(p2);

    int pid = fork();
    if (pid == 0)
    {
        // child process
        char buf[1];
        // close unused end
        close(p1[WRITE]);
        close(p2[READ]);

        // receive msg from parent and close read end
        if (read(p1[READ], buf, 1) != 1)
        {
            fprintf(2, "Failed to read from parent\n");
            exit(1);
        };
        close(p1[READ]);
        printf("%d: received ping\n", getpid());

        // send msg to parent and close write end
        if (write(p2[WRITE], buf, 1) != 1)
        {
            fprintf(2, "Failed to write to parent\n");
            exit(1);
        };
        close(p2[WRITE]);

        exit(0);
    }
    else if (pid > 0)
    {
        // parent process
        char buf[1];
        // close unused end
        close(p1[READ]);
        close(p2[WRITE]);

        // send msg to child and close write end
        if (write(p1[WRITE], "x", 1) != 1)
        {
            fprintf(2, "Failed to send to child\n");
            exit(1);
        };
        close(p1[WRITE]);

        // wait for the child to complete
        wait(0);

        // receive msg from child and close read end
        if (read(p2[READ], buf, 1) != 1)
        {
            fprintf(2, "Failed to read from child\n");
            exit(1);
        }
        close(p2[READ]);
        printf("%d: received pong\n", getpid());

        exit(0);
    }
    else
    {
        // handle fork failure
        fprintf(2, "fork failed\n");
        exit(1);
    }
}