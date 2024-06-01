#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

// gcc ./"modular c files"/pipeline.c -o compiled/pipeline -lreadline -lncurses && ./compiled/pipeline

// ================== Simpler version ==================

// test : cat file.txt | sort | uniq -c
// test : cat config.json | grep -oE "easy|medium|advanced" | sort | uniq -c | sort -n

//=========================current version=========================
// MACROS
#define MAX_TOKENS 100
#define MAX_TOKEN_LENGTH 100

void high_level_parser(const char *inputString, const char *focus_char, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);
void special_tokenizer(const char *inputString, const char *focus_char, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void parseString(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void replaceChar(char *str, char oldChar);
void removeString(char arr[][MAX_TOKEN_LENGTH], int *rowCount, const char *target);
void replaceSubstring(char *str, const char *oldSubstr, const char *newSubstr);

int main()
{
    while (1)
    {
        char *userInput = readline("\nEnter a string:\n");
        printf("\n");
        // Check if userInput is not NULL (i.e., readline was successful)
        // if (userInput != NULL)
        // {
        //     // Print the user input
        //     printf("You entered: %s\n", userInput);
        // }
        char segmentedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
        int inputCount;                                     // Number of words in the parsed string
        high_level_parser(userInput, "|", segmentedString, &inputCount);

        int total_commands = inputCount;
        int total_pipes = total_commands - 1;

        int pid_array[total_pipes - 1]; // will be used for managing the loop children
        int pid3;                       // will be used for the last child (its inside if)

        // step 1 : create pipe
        int fd[total_pipes][2]; // fd[0] is read end, fd[1] is write end
        for (int i = 0; i < total_pipes; i++)
        {
            if (pipe(fd[i]) == -1)
            {
                fprintf(stderr, "Pipe Failed");
                return 1;
            }
        }

        // step 2: fork 1st child
        int pid1 = fork();
        if (pid1 < 0)
        {
            fprintf(stderr, "1st Fork Failed");
            return 1;
        }

        if (pid1 == 0) // child of 1st fork
        {
            for (int i = 0; i < total_pipes; i++)
            {
                if (i != 0)
                {
                    close(fd[i][1]);
                }
                close(fd[i][0]);
            }

            dup2(fd[0][1], STDOUT_FILENO); // redirect stdout to write end of pipe
            close(fd[0][1]);               // close write end of pipe

            // execlp("cat", "cat", "file.txt", NULL);
            char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
            int wordCount;                                   // Number of words in the parsed string
            strcpy(userInput, segmentedString[0]);
            parseString(userInput, parsedString, &wordCount); // Call the parseString function

            char *args[wordCount + 1];
            for (int i = 0; i < wordCount; i++)
            {
                args[i] = strdup(parsedString[i]);
                // printf("args[%d]: %s\n", i, args[i]);
            }
            args[wordCount] = NULL;
            execvp(parsedString[0], args);
        }

        for (int p = 0; p < total_pipes - 1; p++)
        {
            // step 3: fork 2nd child
            pid_array[p] = fork();
            if (pid_array[p] < 0)
            {
                fprintf(stderr, "2nd Fork Failed");
                return 1;
            }

            if (pid_array[p] == 0) // child of 2nd fork
            {
                for (int i = 0; i < total_pipes; i++)
                {
                    if (i != p)
                    {
                        close(fd[i][0]);
                    }
                    if (i != p + 1)
                    {
                        close(fd[i][1]);
                    }
                }

                dup2(fd[p][0], STDIN_FILENO);      // redirect stdin to read end of pipe
                dup2(fd[p + 1][1], STDOUT_FILENO); // redirect stdout to write end of pipe
                close(fd[p][0]);
                close(fd[p + 1][1]);

                // execlp("sort", "sort", NULL);
                char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
                int wordCount;                                   // Number of words in the parsed string
                strcpy(userInput, segmentedString[p + 1]);
                parseString(userInput, parsedString, &wordCount); // Call the parseString function

                char *args[wordCount + 1];
                for (int i = 0; i < wordCount; i++)
                {
                    args[i] = strdup(parsedString[i]);
                    // printf("args[%d]: %s\n", i, args[i]);
                }
                args[wordCount] = NULL;
                execvp(parsedString[0], args);
            }
        }

        // step 3: fork 2nd child
        if (total_pipes > 0)
        {
            pid3 = fork();
            if (pid3 < 0)
            {
                fprintf(stderr, "2nd Fork Failed");
                return 1;
            }

            if (pid3 == 0) // child of 2nd fork
            {
                for (int i = 0; i < total_pipes; i++)
                {
                    if (i != total_pipes - 1)
                    {
                        close(fd[i][0]);
                    }
                    close(fd[i][1]);
                }

                dup2(fd[total_pipes - 1][0], STDIN_FILENO); // redirect stdin to read end of pipe
                close(fd[total_pipes - 1][0]);

                // execlp("uniq", "uniq", "-c", NULL);
                char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
                int wordCount;                                   // Number of words in the parsed string
                strcpy(userInput, segmentedString[total_pipes]);
                parseString(userInput, parsedString, &wordCount); // Call the parseString function

                char *args[wordCount + 1];
                for (int i = 0; i < wordCount; i++)
                {
                    args[i] = strdup(parsedString[i]);
                    // printf("args[%d]: %s\n", i, args[i]);
                }
                args[wordCount] = NULL;
                // printf("parsedString[0]: %s\n", parsedString[0]);
                execvp(parsedString[0], args);
            }
        }

        // parent process (back to main())
        // step 4: close both ends of the pipe in the parent process as none are needed here
        for (int i = 0; i < total_pipes; i++)
        {
            close(fd[i][0]);
            close(fd[i][1]);
        }

        // // step 5:  wait for both children to finish
        // for (int i = 0; i <= total_pipes; i++)
        // {
        //     wait(NULL);
        // }

        // Step 5: Wait for the first child to finish
        waitpid(pid1, NULL, 0);

        // Step 6: Wait for the remaining children to finish
        for (int i = 0; i < total_pipes - 1; i++)
        {
            waitpid(pid_array[i], NULL, 0);
        }

        // If there is at least one pipe, wait for the last child
        if (total_pipes > 0)
        {
            waitpid(pid3, NULL, 0);
        }
    }
}

void high_level_parser(const char *inputString, const char *focus_char, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount)
{
    char copyString[1000]; // Make a copy of the input string to avoid modifying the original
    strcpy(copyString, inputString);

    // damper code start
    char *damper = "~";
    char to_be_replace[10];
    snprintf(to_be_replace, sizeof(to_be_replace), " %c ", *focus_char);

    // code to deal with multiple " " pairs
    char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
    int wordCount;
    special_tokenizer(copyString, "\"", parsedString, &wordCount);
    for (int i = 0; i < wordCount; i = i + 2)
    {
        replaceSubstring(parsedString[i], to_be_replace, damper);
        replaceSubstring(parsedString[i], focus_char, damper);
    }
    // Initialize finalString with an empty string
    char finalString[1000] = ""; // Adjust the size as needed
    for (int i = 0; i < wordCount; i++)
    {
        strcat(finalString, parsedString[i]);
        if (wordCount % 2 == 0)
        {
            strcat(finalString, "\"");
        }
        else if (i < wordCount - 1)
        {
            strcat(finalString, "\"");
        }
    }
    strcpy(copyString, finalString);
    // damper code end

    // Using strtok to tokenize the string based on "" only first
    char *token = strtok(copyString, damper);
    *inputCount = 0;
    while (token != NULL && *inputCount < MAX_TOKENS) // Limit the number of words to MAX_TOKENS
    {
        strcpy(segmentedString[*inputCount], token);
        (*inputCount)++;
        token = strtok(NULL, damper);
    }
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

        // printf("\"%s\" was found in the string.\n", focus_char); <---commenting this out!
    }
    else
    {
        // nothing happens generally
        strcpy(parsedString[0], copyString);
        *wordCount = 1;

        // printf("\"%s\" was NOT found in the string.\n", focus_char); <---commenting this out!
    }
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

void replaceSubstring(char *str, const char *oldSubstr, const char *newSubstr)
{
    char *position = strstr(str, oldSubstr);

    while (position != NULL)
    {
        // Calculate the length of the prefix before the old substring
        size_t prefixLength = position - str;

        // Calculate the length of the suffix after the old substring
        size_t suffixLength = strlen(position + strlen(oldSubstr));

        // Create a new string with the updated substring
        char updatedString[strlen(str) - strlen(oldSubstr) + strlen(newSubstr) + 1];

        // Copy the prefix to the new string
        strncpy(updatedString, str, prefixLength);

        // Copy the new substring to the new string
        strcpy(updatedString + prefixLength, newSubstr);

        // Copy the suffix to the new string
        strcpy(updatedString + prefixLength + strlen(newSubstr), position + strlen(oldSubstr));

        // Update the original string
        strcpy(str, updatedString);

        // Find the next occurrence of the old substring
        position = strstr(str, oldSubstr);
    }
}

// ================== 1st version ==================

// int main()
// {
//     // step 1: create pipe
//     int fd[2];
//     // fd[0] is read end, fd[1] is write end
//     if (pipe(fd) == -1)
//     {
//         fprintf(stderr, "Pipe Failed");
//         return 1;
//     }

//     // step 2: fork 1st child
//     int pid1 = fork();
//     if (pid1 < 0)
//     {
//         fprintf(stderr, "1st Fork Failed");
//         return 1;
//     }
//     else if (pid1 == 0) // child of 1st fork
//     {
//         // close the read end of the pipe as
//         // only write end is needed to save the output of process 1
//         close(fd[0]);

//         // redirect the output (stdout) of process 1
//         // from terminal to the write end of the pipe fd[1]
//         close(1);   // close stdout (the terminal)
//         dup(fd[1]); // duplicate fd[1] to stdout

//         // the close the write end of the pipe and execute the process 1
//         close(fd[1]); //
//         execlp("ping", "ping", "-c", "5", "google.com", NULL);
//     }
//     else // parent of 1st fork
//     {
//         // step 3: in the parent process of the first fork: fork 2nd child
//         int pid2 = fork();
//         if (pid2 < 0)
//         {
//             fprintf(stderr, "2nd Fork Failed");
//             return 1;
//         }
//         else if (pid2 == 0) // child of 2nd fork
//         {
//             // close the write end of the pipe as
//             // only read end is needed to read the output of process 1 as an input
//             close(fd[1]);

//             // redirect the input (stdin) of process 2
//             // from terminal to the read end of the pipe fd[0]
//             close(0);
//             dup(fd[0]);

//             // the close the read end of the pipe and execute the process 2
//             close(fd[0]);
//             execlp("grep", "grep", "rtt", NULL);
//         }
//         else // parent of 2nd fork
//         {
//             // step 4: close both ends of the pipe in the parent process as none are needed here
//             close(fd[0]);
//             close(fd[1]);

//             // step 5:  wait for both children to finish
//             wait(NULL); // wait for 1st child
//             wait(NULL); // wait for 2nd child
//         }
//     }
// }