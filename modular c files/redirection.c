

// gcc ./"modular c files"/redirection.c -o compiled/redirection && ./compiled/redirection
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// dup: system call to duplicate a file descriptor (save in a variable)
// int dup(int oldfd);
// dup2: system call to duplicate a file descriptor to a specific file descriptor (change)
// int dup2(int oldfd, int newfd);
// STDOUT_FILENO: standard output file descriptor (originally is the terminal)

// Global Variable
int terminal_stdout;

// MACROS
#define MAX_TOKENS 100
#define MAX_TOKEN_LENGTH 100

// Helper Functions Prototypes
int redirection_check(const char *inputString);
int redirect_output(const char *filename, int flag);
void default_redirection(int file_fd, int flag);
int redirect_input(const char *filename);

void replaceChar(char *str, char oldChar);
void special_tokenizer(const char *inputString, const char *focus_char, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);

int main()
{
    printf("\n");
    // char myString[1000];
    // printf("\nEnter a string:\n");            // Prompt the user for input
    // fgets(myString, sizeof(myString), stdin); // Read user input from keyboard
    // myString[strcspn(myString, "\n")] = '\0';

    char myString[] = "echo \"hello world\" > file.txt";
    // char myString[] = "echo \"hello world\" \">>\" file.txt";
    // char myString[] = "echo \"hello world\" >> file.txt";
    // char myString[] = "cat < file.txt";

    printf("input: %s\n", myString);

    char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int wordCount;

    int redirection_flag = -1;
    int t;
    redirection_flag = redirection_check(myString);
    if ((redirection_flag == 1) || (redirection_flag == 2) || (redirection_flag == 3)) // ">" or ">>" detection confirmed
    {
        if (redirection_flag == 1)
        {
            special_tokenizer(myString, ">", parsedString, &wordCount);
        }
        else if (redirection_flag == 2)
        {
            special_tokenizer(myString, ">>", parsedString, &wordCount);
        }
        else if (redirection_flag == 3)
        {
            special_tokenizer(myString, "<", parsedString, &wordCount);
        }
        char *filename = strtok(parsedString[wordCount - 1], " "); // removing spaces
        printf("I have saved in the file(=%s) using fd redirection\n", filename);
        if ((redirection_flag == 1) || (redirection_flag == 2))
        {
            t = redirect_output(filename, redirection_flag); // redirect from terminal to the file
        }
        else if (redirection_flag == 3)
        {
            t = redirect_input(filename);
        }
        // changes need to be made for myString, parsedString and wordCount here
        wordCount -= 2; // this takes care of both; wordcount and parsedString array (technically!)
        strcpy(myString, parsedString[0]);

        for (int i = 1; i < wordCount; i++)
        {
            strcat(myString, " ");
            strcat(myString, parsedString[i]);
        }

        // parsing again
        // parseString(myString, parsedString, &wordCount);
    }

    printf("input: %s\n", myString);
    // printing results
    printf("Parsed Words:\n");
    for (int i = 0; i < wordCount; i++)
    {
        printf("%d->%s\n", i + 1, parsedString[i]);
    }
    printf("Total Tokens: %d\n", wordCount);

    if (redirection_flag != -1)
    {
        default_redirection(t, redirection_flag); // redirect from file to the terminal
    }

    printf("Its done, go check it now!! :)\n");

    return 0;
}

// Helper Functions =========================================================================================================

int redirection_check(const char *inputString)
{
    // Copy parameters into separate variables

    // Copy inputString
    const char *copyInputString = strdup(inputString);
    if (copyInputString == NULL)
    {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed for copyInputString\n");
        exit(EXIT_FAILURE);
    }

    char copyParsedString[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int copyWordCount;

    special_tokenizer(copyInputString, " ", copyParsedString, &copyWordCount);
    free((void *)copyInputString);
    printf("==> [%s]\n", copyParsedString[copyWordCount - 2]);
    if (strcmp(copyParsedString[copyWordCount - 2], ">") == 0)
    {
        printf("for file detection i have found (>)\n");
        return 1;
    }
    else if (strcmp(copyParsedString[copyWordCount - 2], ">>") == 0)
    {
        printf("for file detection i have found (>>)\n");
        return 2;
    }
    else if (strcmp(copyParsedString[copyWordCount - 2], "<") == 0)
    {
        printf("for file detection i have found (<)\n");
        return 3;
    }
    else
    {
        printf("no file detection\n");
        return -1;
    }
}

int redirect_output(const char *filename, int flag)
{
    // Disable buffering for stdout
    setbuf(stdout, NULL);

    char temp[50] = "txt_outputs/";
    strcat(temp, filename);

    int file_fd;
    terminal_stdout = dup(STDOUT_FILENO); // terminal fd saved
    if (flag == 1)                        // ">" is confirmed
    {
        file_fd = open(temp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    else // ">>" is confirmed
    {
        file_fd = open(temp, O_WRONLY | O_CREAT | O_APPEND, 0644);
    }
    if (file_fd == -1)
    {
        perror("open");
    }
    dup2(file_fd, STDOUT_FILENO); // redirect from terminal to the file
    return file_fd;
}

void default_redirection(int file_fd, int flag)
{
    if (flag == 1 || flag == 2)
    {
        dup2(terminal_stdout, STDOUT_FILENO); // redirect from file to the terminal
        close(file_fd);
    }
    else if (flag == 3)
    {

        // Restore stdin to the original state (file_fd is actually the terminal_stdout from parameters)
        dup2(file_fd, STDIN_FILENO);
        close(file_fd);
    }
}

int redirect_input(const char *filename)
{
    // Disable buffering for stdin
    setbuf(stdin, NULL);

    // Save the terminal stdin file descriptor
    int terminal_stdin = dup(STDIN_FILENO);

    // Open the file for reading
    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1)
    {
        perror("open");
        return -1;
    }

    // Redirect stdin to the file
    dup2(file_fd, STDIN_FILENO);

    // Close the file descriptor for the file (stdin is now associated with it)
    close(file_fd);

    return terminal_stdin;
}

void special_tokenizer(const char *inputString, const char *focus_char, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount)
{
    char copyString[1000]; // Make a copy of the input string to avoid modifying the original
    strcpy(copyString, inputString);
    *wordCount = 0;

    if (strstr(copyString, focus_char) != NULL)
    {
        char *token = strtok(copyString, focus_char);

        while (token != NULL && *wordCount < MAX_TOKENS) // Limit the number of words to MAX_TOKENS
        {
            strcpy(parsedString[*wordCount], token);
            token = strtok(NULL, focus_char);
            (*wordCount)++;
        }

        // example ASCII---------------------------------------------
        // char example[100];
        // strcpy(example, parsedString[*wordCount - 1]);

        // printf("Enter a string or character: %s\n", parsedString[*wordCount - 1]);
        // // scanf("%s", example);

        // printf("Representation: ");
        // for (int i = 0; example[i] != '\0'; i++)
        // {
        //     printf("%d ", example[i]);
        // }

        // printf("\n");
        // example---------------------------------------------

        // if (strlen(parsedString[*wordCount - 1]) == 1) // needs updating (corrected: that newline thingy with fgets [\n was that bitch])
        // {
        //     // printf("FUCK BHAI NULLL\n");
        //     (*wordCount)--;
        // }

        printf("\"%s\" was found in the string.\n", focus_char);
    }
    else
    {
        // nothing happens generally
        strcpy(parsedString[0], copyString);
        *wordCount = 1;

        printf("\"%s\" was NOT found in the string.\n", focus_char);
    }
}

void replaceChar(char *str, char oldChar)
{
    // Buffer to store the concatenated string
    char replacement[10]; // Adjust the size based on your requirement
    // Concatenate space+variable+space
    snprintf(replacement, sizeof(replacement), " %c ", oldChar);

    char *pos = strchr(str, oldChar);

    while (pos != NULL)
    {
        // Replace oldChar with newString
        memmove(pos + strlen(replacement), pos + 1, strlen(pos));
        memcpy(pos, replacement, strlen(replacement));

        // Search for the next occurrence
        pos = strchr(pos + strlen(replacement), oldChar);
    }
}
// ----------------------------------------------------------------------------------------------------------------------------
// int main()
// {
//     printf("\n");

//     // Disable buffering for stdout
//     setbuf(stdout, NULL);

//     // Save the terminal
//     int terminal_stdout = dup(STDOUT_FILENO);

//     // A low level system call that returns the file descriptor for an open file
//     // it is not the same as "File Handling"
//     int file_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
//     if (file_fd == -1)
//     {
//         perror("open");
//         return 1;
//     }

//     // Redirect to the file
//     dup2(file_fd, STDOUT_FILENO);

//     // Now anything written to stdout will go to "output.txt"
//     printf("This will be written to output.txt file");
//     // fflush(stdout); // Force a flush

//     // Restore the original standard output
//     dup2(terminal_stdout, STDOUT_FILENO);

//     // Now, anything written to stdout will go to the original standard output
//     printf("This will be written to the original standard output\n");

//     close(file_fd);
//     return 0;
// }
