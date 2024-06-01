#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <stdlib.h>

// gcc ./"modular c files"/parser.c -o compiled/parser -lreadline -lncurses && ./compiled/parser

/*
Parser Functionalities:
it tokenizes the input string
it caters to "" and '' quotes as well by tokenizing it separately
*/

// MACROS
#define MAX_TOKENS 100
#define MAX_TOKEN_LENGTH 100

// Helper Funciton Prototypes
void parseString(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void replaceChar(char *str, char oldChar);
void special_tokenizer(const char *inputString, const char *focus_char, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void removeString(char arr[][MAX_TOKEN_LENGTH], int *rowCount, const char *target);
void high_level_parser(const char *inputString, const char *focus_char, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);
void replaceSubstring(char *str, const char *oldSubstr, const char *newSubstr);

// test inputs

// grep " total " Tests/test_output/*.msh.out | sort -r | uniq > Tests/grep.log && echo " Search successful, logged to grep.log " || echo " Failure " ; false || echo " Print " ; rm Tests/grep.log
// echo " One letter ext " && ls ../*/*.? | grep -o '\.[a-zA-Z]$' | uniq | grep -o '[a-zA-Z]'
// cat config.json | grep -oE "easy|medium|advanced" | sort | uniq -c | sort -n
// echo "My password is {12345678}" | tr "{}" "()" | tr -d [:digit:] | tr [:lower:] [:upper:]

int main()
{
    // // char myString[] = " echo \" i am king\"  'text'.txt echo \" pipeline  \"";
    // // char myString[] = "cat ../src/*.c | grep -o return | wc -l > ret_count.log ; cat ret_count.log ; rm ret_count.log";
    // char myString[] = "cat config.json | grep -oE \"easy|medium|advanced\" | sort | uniq -c | sort -n";
    // printf("Input String: %s\n", myString);

    // // char myString[1000];
    // // printf("\nEnter a string:\n");            // Prompt the user for input
    // // fgets(myString, sizeof(myString), stdin); // Read user input from keyboard

    // // Remove the newline character if it exists
    // myString[strcspn(myString, "\n")] = '\0';

    char *userInput = readline("\nEnter a string:\n");
    // Check if userInput is not NULL (i.e., readline was successful)
    if (userInput != NULL)
    {
        // Print the user input
        printf("\nYou entered:\n%s\n", userInput);
        // strcpy(myString, userInput);
    }

    // char segmentedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
    // int inputCount;                                     // Number of words in the parsed string
    // high_level_parser(userInput, "|", segmentedString, &inputCount);

    char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH];  // Array to store parsed words
    int wordCount;                                    // Number of words in the parsed string
    parseString(userInput, parsedString, &wordCount); // Call the parseString function
    // special_tokenizer(myString, " | ", parsedString, &wordCount);

    // printing results
    printf("\nParsed Words:\n");
    for (int i = 0; i < wordCount; i++)
    {
        strcpy(userInput, parsedString[i]);
        printf("%d->%s\n", i + 1, userInput);
    }
    printf("Total Tokens: %d\n", wordCount);

    // Free the memory allocated by readline
    free(userInput);
}

// Helper Functions =========================================================================================================
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