#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PRIME_NUM 35
#define READ 0
#define WRITE 1

void sieve(int read_fd) {
    int prime;
    // base case:
    // if no more data, close and exit
    if (read(read_fd, &prime, sizeof(prime)) <= 0) {
        close(read_fd);
        exit(0);
    }

    // recursive case:
    // 1. read 1st prime from pipe and print
    printf("prime %d\n", prime);

    // 2. create new pipe for output
    int newp[2];
    pipe(newp);

    // 3. for new process to continue filtering
    if (fork() == 0) {
        // child process
        close(newp[WRITE]);
        sieve(newp[READ]);
    } else {
        // parent process
        close(newp[READ]);
        int n;
        while (read(read_fd, &n, sizeof(n)) > 0) {
            if (n % prime != 0) {
                write(newp[WRITE], &n, sizeof(n));
            } 
        }
        close(read_fd);
        close(newp[WRITE]);
        wait(0);
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    int p[2];
    pipe(p);

    if (fork() == 0) {
        // child process begin to sieve
        close(p[WRITE]);
        sieve(p[READ]);
    } else {
        // parent process writes number
        close(p[READ]);
        for (int i = 2; i <= PRIME_NUM; i++) {
            write(p[WRITE], &i, sizeof(i));
        }
        close(p[WRITE]);
        wait(0);   
    }
    exit(0);
}