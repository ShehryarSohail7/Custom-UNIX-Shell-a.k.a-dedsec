#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>

// gcc ./"modular c files"/chaining.c -o compiled/chaining -lreadline -lncurses && ./compiled/chaining

//=========================current version=========================
// MACROS
#define MAX_TOKENS 100
#define MAX_TOKEN_LENGTH 100

void removeString(char arr[][MAX_TOKEN_LENGTH], int *rowCount, const char *target);
void high_level_parser(const char *inputString, const char *focus_char, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);
void parseString(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void special_tokenizer(const char *inputString, const char *focus_char, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void replaceSubstring(char *str, const char *oldSubstr, const char *newSubstr);
void chaining_parser(const char *inputString, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);
int chain_execute(int status, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);

int main()
{
    while (1)
    {
        char *userInput = readline("Enter a string:");
        if (strstr(userInput, " ; ") != NULL || strstr(userInput, " && ") != NULL || strstr(userInput, " || ") != NULL)
        {
            char segmentedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
            int inputCount;
            chaining_parser(userInput, segmentedString, &inputCount);
            int status = 0; // Assume success at the beginning
            int execute_flag = 1;
            // print all the segmented strings
            for (int i = 0; i < inputCount; i += 2)
            {
                // printf("status begin: %d\n", status);
                if (execute_flag) // true
                {
                    char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
                    int parseCount;
                    parseString(segmentedString[i], parsedString, &parseCount);
                    status = chain_execute(status, parsedString, &parseCount);
                }

                if (strcmp(segmentedString[i + 1], "&&") == 0 && status == 0)
                {
                    execute_flag = 1;
                }
                else if (strcmp(segmentedString[i + 1], "&&") == 0 && status == 1)
                {
                    execute_flag = 0;
                }
                else if (strcmp(segmentedString[i + 1], "||") == 0 && status == 1)
                {
                    execute_flag = 1;
                }
                else if (strcmp(segmentedString[i + 1], "||") == 0 && status == 0)
                {
                    execute_flag = 0;
                }
                else if (strcmp(segmentedString[i + 1], ";") == 0)
                {
                    execute_flag = 1;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            printf("No chaining operators found in the string.\n");
        }
    }
}

void chaining_parser(const char *inputString, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount)
{
    char copyString[1000];            // Make a copy of the input string to avoid modifying the original
    strcpy(copyString, inputString);  // Number of words in the parsed string
    char copyString2[1000];           // Make a copy of the input string to avoid modifying the original
    strcpy(copyString2, inputString); // Number of words in the parsed string

    // part 1
    char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
    int wordCount;
    special_tokenizer(copyString, "\"", parsedString, &wordCount);
    for (int i = 0; i < wordCount; i = i + 2)
    {
        replaceSubstring(parsedString[i], "||", "`");
        replaceSubstring(parsedString[i], "&&", "`");
        replaceSubstring(parsedString[i], ";", "`");
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
    high_level_parser(finalString, "`", segmentedString, inputCount);

    // part 2
    char parsedString2[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
    int wordCount2;
    char chain_operator[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int operator_count = 0;
    parseString(copyString2, parsedString2, &wordCount2);
    for (int i = 0; i < wordCount2; i++)
    {
        if (strcmp(parsedString2[i], "||") == 0)
        {
            strcpy(chain_operator[operator_count], "||");
            operator_count++;
        }
        else if (strcmp(parsedString2[i], "&&") == 0)
        {
            strcpy(chain_operator[operator_count], "&&");
            operator_count++;
        }
        else if (strcmp(parsedString2[i], ";") == 0)
        {
            strcpy(chain_operator[operator_count], ";");
            operator_count++;
        }
    }

    // // print the chain operators
    // printf("Chain Operators: ");
    // for (int i = 0; i < operator_count; i++)
    // {
    //     printf("%s ", chain_operator[i]);
    // }
    // printf("\n");

    // combining the 2 arrays together
    int total_count = *inputCount + operator_count;
    char combinedString[MAX_TOKENS][MAX_TOKEN_LENGTH];
    int index = 0;
    int index2 = 0;
    for (int i = 0; i < total_count; i++)
    {
        if (i % 2 == 0)
        {
            strcpy(combinedString[i], segmentedString[index]);
            index++;
        }
        else
        {
            strcpy(combinedString[i], chain_operator[index2]);
            index2++;
        }
    }

    // Copy contents of combinedString back to segmentedString
    for (int i = 0; i < total_count; i++)
    {
        strcpy(segmentedString[i], combinedString[i]);
    }
    *inputCount = total_count;
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

int chain_execute(int status, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        char *args[*inputCount + 1];
        for (int i = 0; i < *inputCount; i++)
        {
            args[i] = strdup(segmentedString[i]);
            // printf("args[%d]: %s\n", i, args[i]);
        }
        args[*inputCount] = NULL;
        execvp(segmentedString[0], args);
        // If execvp returns, it means there was an error
        perror("execvp"); // Print the error
        exit(1);          // Indicate failure
    }
    else if (pid < 0)
    {
        // Error occurred in forking
        perror("fork");
        exit(1); // Indicate failure
    }
    else
    {
        // Parent process
        int childStatus;
        waitpid(pid, &childStatus, 0); // Wait for child to finish
        if (WIFEXITED(childStatus))
        {
            // Child process terminated normally
            int exitStatus = WEXITSTATUS(childStatus);
            if (exitStatus == 0)
            {
                // Command executed successfully
                // printf("Command executed successfully\n");
                status = 0;
                // printf("Exit status: %d\n", exitStatus);
            }
            else
            {
                // Command failed to execute
                // printf("Command failed to execute\n");
                status = 1;
                // printf("Exit status: %d\n", exitStatus);
            }
        }
        else
        {
            // Child process terminated abnormally
            // printf("Child process terminated abnormally\n");
            status = 1;
            // printf("Exit status: %d\n", WTERMSIG(childStatus));
        }
    }
    return status;
}