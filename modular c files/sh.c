// gcc ./"modular c files"/sh.c -o compiled/sh && ./compiled/sh

#include <stdio.h>
#include <stdlib.h>

int main()
{
    // Specify the name of the shell script to run
    const char *shell_script = "./virus/iloveyou.sh";

    // Execute the shell script
    printf("Executing %s\n", shell_script);
    int status = system(shell_script);

    // Check if execution was successful
    if (status == 0)
    {
        printf("%s executed successfully.\n", shell_script);
    }
    else
    {
        printf("Error executing %s\n", shell_script);
    }

    return 0;
}
