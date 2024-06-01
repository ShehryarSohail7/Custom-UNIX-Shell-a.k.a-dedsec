#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// gcc ./"modular c files"/execvp.c -o compiled/execvp && ./compiled/execvp

int main()
{
    char *test[100][100];
    int total_pipes = 1;
    // Setting up the commands in the array
    test[0][0] = "ls";
    test[0][1] = "-l";
    test[0][2] = NULL;

    test[1][0] = "grep";
    test[1][1] = "-oE";
    test[1][2] = "easy|medium|advanced";
    test[1][3] = NULL;

    test[2][0] = "sort";
    test[2][1] = NULL;

    test[3][0] = "uniq";
    test[3][1] = "-c";
    test[3][2] = NULL;

    test[4][0] = "sort";
    test[4][1] = "-n";
    test[4][2] = NULL;

    // Execute the ls command
    execvp(test[0][0], test[0]);

    // If execvp fails, print an error message
    perror("Exec failed");

    // This line will only be reached if execvp fails
    return EXIT_FAILURE;
}
