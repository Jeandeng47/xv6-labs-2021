#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLINE 100
#define READ 0

// Function to copy a string from source to destination with dynamic memory allocation
void copy(char **dest, char *src)
{
    *dest = malloc(strlen(src) + 1);
    strcpy(*dest, src);
}

int read_line(char *args[], int startIndex)
{
    char buf[MAXLINE];  // Buffer to read the line
    int n = 0;           // Index for buffer


    // read character by character until newline or EOF
    while (read(READ, &buf[n], 1) == 1) {
        if (buf[n] == '\n') {  // Stop reading at newline
            buf[n] = '\0';     // Null-terminate the line
            break;
        }
        n++;
        if (n >= MAXLINE - 1) {  // Prevent buffer overflow
            fprintf(2, "Input line too long\n");
            exit(1);
        }
    }

    // Check if nothing was read (EOF)
    if (n == 0) {
        printf("End of file or no input detected.\n");
        return -1;  // EOF reached
    }


    // Tokenize the line by spaces and add each token to args
    int k = 0;  // Index for parsing the buffer
    while (k < n) {
        if (startIndex >= MAXARG) {
            fprintf(2, "Too many parameters!\n");
            exit(1);
        }

        // Skip leading spaces
        while (k < n && buf[k] == ' ') {
            k++;
        }

        if (k >= n) break;  // Break if we reach the end

        int l = k;  // Start of the token

        // Find the end of the token
        while (k < n && buf[k] != ' ') {
            k++;
        }

        buf[k] = '\0';  // Null-terminate the token
        copy(&args[startIndex], buf + l);  // Add the token to args 
        startIndex++;
        k++;  // Move past the null terminator we just added
    }

    args[startIndex] = 0;  // Null-terminate the args array
    return startIndex;  // Return the next index position in args
}

int main(int argc, char *argv[]) {
    char *args[MAXARG]; // store xargs params: cmd, arg[0], arg[1]...arg[k-1]
    int pid;

    // check arguments number
    if (argc < 2) {
        fprintf(2, "Usage: xargs command\n");
        exit(1);
    }

    // read command to args
    // echo hello world | xargs echo bye
    // argv[] = {"xargs", "echo", "bye"}
    // args[] = {"echo", "bye"}
    for (int i = 1; i < argc; i++) {
        copy(&args[i-1], argv[i]);
    }
  
    int end;
    while ((end = read_line(args, argc - 1)) != -1) {
        pid = fork();

        if (pid == 0)
        {
            // child process: execute the command
            exec(args[0], args);
            // if exec fails
            fprintf(2, "exec failed\n");
            exit(1);
        }
        else if (pid > 0)
        {
            // parent process: wait for the child to process
            wait(0);
        }
        else
        {
            fprintf(2, "fork failed\n");
            exit(1);
        }
    }

    // free allocated memory in args to prevent memory leaks
    for (int i = 0; i < argc - 1; i++)
    {
        free(args[i]);
    }

    exit(0);
}