#include <stdio.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>    // Include this header for getenv function
#include <unistd.h>    // Include this header for fork and getpid functions
#include <sys/types.h> // Include this header for pid_t type
#include <sys/wait.h>  // Include this header for wait function

// gcc ./"modular c files"/fork-exec_model.c -o compiled/fork-exec_model && ./compiled/fork-exec_model
int main()
{
    printf("hello world in main() before entering fork\n");
    pid_t pid = fork();

    if (pid == -1) // fork failed
    {
        perror("fork");
        return 1;
    }

    if (pid == 0) // Child process
    {
        printf("This is the child process (PID: %d)\n", getpid());

        // Replace the child process image with a new one
        char *args[] = {"ls", "-l", NULL}; // Example: listing files in the current directory
        execvp("ls", args);

        // If execvp fails
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else // Parent process (pid > 0) (it is just going to wait for the child process to complete)
    {
        printf("This is the parent process (PID: %d), child PID: %d\n", getpid(), pid);

        int status;
        wait(&status); // Wait for the child process to complete

        // // Wait for the child process to complete
        // int status;
        // waitpid(pid, &status, 0);

        // if (WIFEXITED(status))
        // {
        //     printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        // }
        // else
        // {
        //     printf("Child process did not exit normally\n");
        // }
    }

    printf("Outside the fork and back to main()\n");

    return 0;
}