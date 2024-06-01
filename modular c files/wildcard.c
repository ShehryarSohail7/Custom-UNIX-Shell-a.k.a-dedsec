#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <unistd.h> // Include for getcwd

// gcc ./"modular c files"/wildcard.c -o compiled/wildcard -lreadline -lncurses && ./compiled/wildcard

// ls ??????.c *.txt
// ls *.txt *.c

#define MAX_PATH 4096
#define MAX_TOKENS 100
#define MAX_TOKEN_LENGTH 100

int prompt(const char *input);
void parseString(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void removeString(char arr[][MAX_TOKEN_LENGTH], int *rowCount, const char *target);
void wildcardExpansion(const char *input, char wildcard_input[MAX_TOKENS]);
void addQuotesIfSpace(char *input);

int main()
{
    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);

    // Enable history
    using_history();

    while (1)
    {
        char cwd_display[MAX_PATH]; // A buffer to store the current directory
        if (getcwd(cwd_display, sizeof(cwd_display)) != NULL)
        {
            // printf("ShehryarSohail@shell:%s$ ", cwd_display); // version 1
            printf("\033[0;31mshehryar@Shell:dedsec$ \033[0m"); // version 2
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
        // Perform wildcard expansion
        char wildcard_input[MAX_TOKENS]; // Array to store parsed words
        strcpy(wildcard_input, "");      // Initialize the string
        wildcardExpansion(input, wildcard_input);
        // Print expanded path
        printf("%s\n", wildcard_input);
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

void parseString(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount)
{
    char copyString[1000]; // Make a copy of the input string to avoid modifying the original
    strcpy(copyString, inputString);

    // char focus_char = '\'';
    // replaceChar(copyString, focus_char);

    // Using strtok to tokenize the string based on "" only first
    char *token = strtok(copyString, "\"\'");
    *wordCount = 0;
    while (token != NULL && *wordCount < MAX_TOKENS) // Limit the number of words to MAX_TOKENS
    {
        strcpy(parsedString[*wordCount], token);
        (*wordCount)++;
        token = strtok(NULL, "\"\'");
    }

    // round two (based on white space and negecting strings inside "")
    int index = 0;
    char parsedString2[MAX_TOKENS][MAX_TOKEN_LENGTH];
    for (int i = 0; i < *wordCount; i++)
    {
        if (i % 2 == 0)
        {
            char *token2 = strtok(parsedString[i], " \t\n");
            int wordCount2 = 0;

            while (token2 != NULL && wordCount2 < MAX_TOKENS) // Limit the number of words to MAX_TOKENS
            {
                strcpy(parsedString2[index], token2);
                (wordCount2)++;
                // printf("index: %d, token: %s\n", index, token2);
                index = index + 1;
                token2 = strtok(NULL, " \t\n");
            }
        }
        else
        {
            // strcpy(parsedString2[index], "\""); // parsing the " back too
            // strcpy(parsedString2[index + 1], parsedString[i]);
            // strcpy(parsedString2[index + 2], "\"");
            // index = index + 3;

            strcpy(parsedString2[index], parsedString[i]); // removing the "
            index = index + 1;
        }
    }

    // Copy contents of parsedString2 back to parsedString
    for (int i = 0; i < index; i++)
    {
        strcpy(parsedString[i], parsedString2[i]);
    }
    *wordCount = index;

    // catering ' string here by removing it
    removeString(parsedString, wordCount, "'");
}

void removeString(char arr[][MAX_TOKEN_LENGTH], int *rowCount, const char *target)
{
    int i, j;
    int count = *rowCount;

    for (i = 0; i < count; i++)
    {
        if (strcmp(arr[i], target) == 0)
        {
            // Shift elements to fill the gap
            for (j = i; j < count - 1; j++)
            {
                strcpy(arr[j], arr[j + 1]);
            }
            count--; // Decrease the row count
            i--;     // Adjust index after shifting
        }
    }

    *rowCount = count; // Update the row count
}

void wildcardExpansion(const char *input, char wildcard_input[MAX_TOKENS])
{
    glob_t glob_result[MAX_TOKENS];
    int ret[MAX_TOKENS];
    char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int wordCount;
    parseString(input, parsedString, &wordCount);

    // Perform wildcard expansion for each token
    for (int i = 0; i < wordCount; i++)
    {
        ret[i] = glob(parsedString[i], 0, NULL, &glob_result[i]);
    }

    for (int i = 0; i < wordCount; i++)
    {
        if (ret[i] != 0)
        {
            if (i == 0)
            {
                strcat(wildcard_input, parsedString[i]);
            }
            else
            {
                strcat(wildcard_input, " "); // Add space between tokens
                strcat(wildcard_input, parsedString[i]);
            }
        }
        else
        {

            for (int j = 0; j < glob_result[i].gl_pathc; j++)
            {
                strcat(wildcard_input, " "); // Add space between tokens
                addQuotesIfSpace(glob_result[i].gl_pathv[j]);
                strcat(wildcard_input, glob_result[i].gl_pathv[j]);
            }
        }
    }

    // Free all glob results
    for (int i = 0; i < wordCount; i++)
    {
        globfree(&glob_result[i]);
    }
}

void addQuotesIfSpace(char *input)
{
    if (strchr(input, ' ') != NULL)
    {                                       // Check if string contains a space
        char output[2 * strlen(input) + 3]; // Allocate space for double quotes and null terminator
        sprintf(output, "\"%s\"", input);   // Add double quotes around the string
        strcpy(input, output);              // Copy the modified string back to input
    }
}

// individual arguments assumed for * and ? wildcard
// void wildcardExpansion(const char *input, char wildcard_input[][MAX_TOKEN_LENGTH], int *wildcard_input_count)
// {
//     glob_t glob_result[MAX_TOKENS];
//     int ret[MAX_TOKENS];
//     char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH];
//     int wordCount;
//     parseString(input, parsedString, &wordCount);

//     // Perform wildcard expansion for each token
//     for (int i = 0; i < wordCount; i++)
//     {
//         ret[i] = glob(parsedString[i], 0, NULL, &glob_result[i]);
//     }

//     // Calculate the total number of expanded paths
//     int total = 1;
//     for (int i = 0; i < wordCount; i++)
//     {
//         if (ret[i] == 0)
//         {
//             total *= glob_result[i].gl_pathc;
//         }
//     }

//     // Construct expanded paths
//     int index[MAX_TOKENS] = {0}; // Array to keep track of indices for each token
//     for (int i = 0; i < total; i++)
//     {
//         strcpy(wildcard_input[i], ""); // Initialize the string
//         for (int j = 0; j < wordCount; j++)
//         {
//             if (ret[j] != 0)
//             {
//                 strcat(wildcard_input[i], parsedString[j]);
//             }
//             else
//             {
//                 strcat(wildcard_input[i], glob_result[j].gl_pathv[index[j]]);
//             }
//             strcat(wildcard_input[i], " "); // Add space between tokens
//         }
//         // Increment indices for each token
//         for (int j = wordCount - 1; j >= 0; j--)
//         {
//             if (ret[j] == 0)
//             {
//                 index[j]++;
//                 if (index[j] >= glob_result[j].gl_pathc)
//                 {
//                     index[j] = 0; // Reset index if it exceeds the number of paths
//                 }
//                 else
//                 {
//                     break; // Move to the next string if index is successfully incremented
//                 }
//             }
//         }
//     }

//     // Set the count of expanded paths
//     *wildcard_input_count = total;

//     // Free all glob results
//     for (int i = 0; i < wordCount; i++)
//     {
//         globfree(&glob_result[i]);
//     }
// }
