// pid_t pid = fork();
// execvp(program, args)
// dup2(original_stdout, STDOUT_FILENO);
// dup2(original_stdin, STDIN_FILENO);

#include "../include/utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <glob.h>

#include <sys/stat.h> //extra
// gcc ./"modular c files"/testing.c -o compiled/testing && ./compiled/testing
int main()
{
    int fd[2];          // file descripters
    if (pipe(fd) == -1) // pipeline is created and opened
    {
        printf("Pipe failed\n");
        // printf("pipe output = %d\n", pipe(fd));
    }
    else
    {
        printf("Pipeline opened\n");
        // printf("pipe output = %d\n", pipe(fd));
    }

    printf("File Descripters:\n");
    printf("fd[0] = %d\n", fd[0]); // read
    printf("fd[1] = %d\n", fd[1]); // write
    // fd[0] = read
    // fd[1] = write

    // lets create 2 processes using fork() and try to communicate between them using the pipeline
    // we always close one end of the pipeline in each process
    // and close the other one also after use, otherwise the process will hang
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("Fork failed\n");
    }
    else if (pid == 0) // child process
    {
        // child process: close read end, use write end and then close it in the end
        printf("Child process\n");
        printf("Child process pid = %d\n", getpid());

        // close the read end of the pipeline
        close(fd[0]);

        // redirect the stdout to the write end of the pipeline
        dup2(fd[1], STDOUT_FILENO);

        // execute the command
        char *args[] = {"ls", "-l", NULL};
        execvp(args[0], args);
    }
    else // parent process
    {
        // parent process: close write end, use read end and then close it in the end
        printf("Parent process\n");
        printf("Parent process pid = %d\n", getpid());

        // close the write end of the pipeline
        close(fd[1]);

        // redirect the stdin to the read end of the pipeline
        dup2(fd[0], STDIN_FILENO);

        // execute the command
        char *args[] = {"wc", "-l", NULL};
        execvp(args[0], args);
    }

    return 0;
}