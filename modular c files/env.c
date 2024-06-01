#include <stdio.h>
#include <stdlib.h>    // Include this header for getenv function
#include <unistd.h>    // Include this header for fork and getpid functions
#include <sys/types.h> // Include this header for pid_t type

// gcc ./"modular c files"/env.c -o compiled/env && ./compiled/env

int main()
{
    char *env = getenv("WSLENV");
    printf("WSLENV: %s\n", env);

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        // Child process
        printf("This is the child process (PID: %d)\n", getpid());
    }
    else
    {
        // Parent process
        printf("This is the parent process (PID: %d), child PID: %d\n", getpid(), pid);
    }

    return 0;
}