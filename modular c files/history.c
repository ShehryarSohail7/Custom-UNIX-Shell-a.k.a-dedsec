#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <unistd.h> // Include for getcwd

// gcc ./"modular c files"/history.c -o compiled/history -lreadline -lncurses && ./compiled/history

#define MAX_PATH 4096

int prompt(const char *input);
int main()
{
    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);

    // Enable history
    using_history();
    printf("\nWe are DedSec!! \n");

    while (1)
    {
        char cwd_display[MAX_PATH]; // A buffer to store the current directory
        if (getcwd(cwd_display, sizeof(cwd_display)) != NULL)
        {
            // printf("ShehryarSohail@shell:%s$ ", cwd_display); // version 1
            printf("shehryar@Shell:dedsec$ "); // version 2
        }
        else
        {
            perror("getcwd");
        }
        // Display prompt and read input
        char *input;
        input = readline("");

        // Update (prompting till " is even)
        while (prompt(input) % 2 != 0)
        {
            printf(">");
            char *input2;
            input2 = readline("");
            input = strcat(input, input2);
        }

        // Check for EOF.
        if (!input)
            break;

        // Add input to readline history.
        add_history(input);

        // Do stuff...
        //=============================================================================

        //=============================================================================

        // Free buffer that was allocated by readline
        free(input);
    }

    return 0;
}

int prompt(const char *input)
{
    int count = 0;
    while (*input)
    {
        if (*input == '"')
        {
            count++;
        }
        input++;
    }
    return count;
}
