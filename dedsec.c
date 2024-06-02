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
#include <signal.h>

// gcc dedsec.c -o dedsec -lreadline -lncurses && ./dedsec
// gcc dedsec.c -o dedsec -lreadline -lncurses && ./dedsec "test (hidden)"/Tests/chaining.test
// gcc dedsec.c -o dedsec -lreadline -lncurses && cd ../Code/test && ../../"unix processes in C"/dedsec ./Tests/chaining.test && cd ../../"unix processes in C"

// MACROS
#define MAX_PATH 4096
#define MAX_TOKENS 500
#define MAX_TOKEN_LENGTH 500
#define MAX_FILE_LINES 500
#define MAX_ALIASES 500

// User-Defined Datatypes
struct Alias
{
    char name[MAX_TOKEN_LENGTH];
    char value[MAX_TOKEN_LENGTH];
};

// Global arrays and variables
struct Alias aliasArray[MAX_ALIASES];
int aliasCount = 0;
int terminal_stdout;
int version = 0;

// Helper Funciton Prototypes
void cycle(char *str);
int face();
int pipeline_execution(char *input, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);
int dedsec_execution(char *input, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
int built_in_functions(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *exitFlag, int wordCount);
int prompt(const char *input);
int script_mode(int *script_flag, int argc, char *argv[], char file_lines[][BUFSIZ], int *file_line_count);
void special_tokenizer(const char *inputString, const char *focus_char, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void parseString(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
void replaceChar(char *str, char oldChar);
int isNumeric(const char *input);
void removeString(char arr[][MAX_TOKEN_LENGTH], int *rowCount, const char *target);
void replaceFirstOccurrence(char *str, const char *oldWord, const char *newWord);
void alias_check(char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount);
int redirection_check(const char *inputString);
int redirect_output(const char *filename, int flag);
void default_redirection(int file_fd, int flag);
int redirect_input(const char *filename);
void high_level_parser(const char *inputString, const char *focus_char, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);
void replaceSubstring(char *str, const char *oldSubstr, const char *newSubstr);
void chaining_parser(const char *inputString, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount);
int chain_execute(int status, char *input);
void wildcardExpansion(const char *input, char wildcard_input[MAX_TOKENS]);
void addQuotesIfSpace(char *input);

// loop termination signal handling
volatile int terminate_loop = 0;
void handle_signal(int signum)
{
    if (signum == SIGUSR1)
    {
        terminate_loop = 1;
    }
}

int main(int argc, char *argv[])
{
    // Check the mode first (Script or Interactive)
    int script_flag = 1;
    char file_lines[MAX_FILE_LINES][BUFSIZ]; // Array to store lines
    int file_line_count = 0;
    if (script_mode(&script_flag, argc, argv, file_lines, &file_line_count) == 1)
    {
        return EXIT_FAILURE;
    }
    int file_track = 0;

    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);

    // Enable history
    using_history();
    face();                        // (test mode)
    printf("\nWe are DedSec!!\n"); // (test mode CORE)
    char *input;
    char *previous_input = NULL;

    // Set up the signal handler
    signal(SIGUSR1, handle_signal);

    char *credentials = "";

    while (1)
    {
        char cwd_display[MAX_PATH]; // A buffer to store the current directory
        if (getcwd(cwd_display, sizeof(cwd_display)) != NULL)
        {
            if (version == 1)
            {
                char conc[MAX_PATH + 36];                                                                    // Temporary buffer for storing concatenated string
                snprintf(conc, sizeof(conc), "\033[0;31mshehryar@Shell:\033[0;32m%s\033[0m$ ", cwd_display); // version 1
                credentials = conc;
            }
            else
            {
                credentials = "\033[0;31mshehryar@Shell:dedsec$ \033[0m"; // version 2 (test mode)
            }
        }
        else
        {
            perror("getcwd");
        }
        // Display prompt and read input
        if (script_flag == 1)
        {
            input = file_lines[file_track]; // Script mode
            printf("%s", credentials);      // printing inputs in script mode
            printf("%s\n", input);
            // comment this 'printf' to pass test "ioredir_one.hidden" and all pipeline tests (test mode CORE)
        }
        else
        {
            input = readline(credentials); // Interactive mode

            if (strlen(input) == 0) // clearing the buffer
            {
                // Free buffer and continue to the next iteration
                free(input);
                continue;
            }
            // prompting till " is even
            while (prompt(input) % 2 != 0)
            {
                printf(">");
                char *input2;
                input2 = readline("");
                input = strcat(input, input2);
            }
        }

        // Check for EOF.
        if (!input)
            break;

        // Add input to readline history. (edit: only if previous input is not same as current input)
        if (previous_input == NULL || strcmp(input, previous_input) != 0)
        {
            // If previous_input is NULL (first input) or current input is different from previous input
            add_history(input);

            // Free memory allocated for previous_input if it's not NULL
            if (previous_input != NULL)
            {
                free(previous_input);
            }

            // Allocate memory for previous_input and copy the current input into it
            previous_input = strdup(input);
        }

        // Do stuff...
        //=============================================================================
        char basic_breakdown_String[MAX_TOKENS][MAX_TOKEN_LENGTH];              // Array to store parsed words
        int basic_breakdown_wordCount;                                          // Number of words in the parsed string
        parseString(input, basic_breakdown_String, &basic_breakdown_wordCount); // Call the parseString function
        alias_check(input, basic_breakdown_String, &basic_breakdown_wordCount);

        if ((strstr(input, " ; ") != NULL || strstr(input, " && ") != NULL || strstr(input, " || ") != NULL) && strcmp(basic_breakdown_String[0], "alias") != 0)
        {
            char segmentedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
            int inputCount;
            chaining_parser(input, segmentedString, &inputCount);
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
                    status = chain_execute(status, segmentedString[i]);
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
        else if (strstr(input, " | ") != NULL && strcmp(basic_breakdown_String[0], "alias") != 0)
        {
            char segmentedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
            int inputCount;                                     // Number of words in the parsed string
            high_level_parser(input, "|", segmentedString, &inputCount);
            pipeline_execution(input, segmentedString, &inputCount);
            if (terminate_loop)
            {
                break; // This will break out of the outer loop
            }
        }
        else
        {
            char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
            int wordCount;                                   // Number of words in the parsed string
            parseString(input, parsedString, &wordCount);    // Call the parseString function
            if (dedsec_execution(input, parsedString, &wordCount) == 2)
            {
                break;
            };
        }
        //=============================================================================

        // Free buffer that was allocated by readline (if in script mode then no need to use free())
        if (script_flag == 0)
        {
            free(input);
        }

        // Exiting through script mode
        file_track++;
        if ((script_flag == 1) && (file_track == file_line_count))
        {
            break;
        }
    }

    return 0;
}

// Helper Functions =========================================================================================================
void cycle(char *str)
{
    // total length of the string
    int length = 0;
    int ascii = 0;
    char display;
    char cycle_string[100] = "";
    while (str[length] != '\0')
    {
        length++;
    }
    for (int i = 0; i < length; i++)
    {
        ascii = (int)str[i];
        if (ascii >= 65 && ascii <= 90) // cater special characters
        {
            for (int j = 65; j <= ascii; j++)
            {
                display = (char)j;
                // add the letter in the cycle_string
                cycle_string[i] = display;
                printf("%s\n", cycle_string);
                // sleep
                usleep(25000);
            }
        }
        else if (ascii >= 97 && ascii <= 122) // cater special characters
        {
            for (int j = 97; j <= ascii; j++)
            {
                display = (char)j;
                // add the letter in the cycle_string
                cycle_string[i] = display;
                printf("%s\n", cycle_string);
                // sleep
                usleep(25000);
            }
        }
        else
        {
            cycle_string[i] = str[i];
            printf("%s\n", cycle_string);
            // sleep
            usleep(25000);
        }
    }
}

int face()
{
    // Replace "your_image.jpg" with the path to your image file
    const char *image_path = "dedsec.png";

    // Construct the command to display the image using imgcat
    char command[100];
    snprintf(command, sizeof(command), "imgcat %s", image_path);

    // Use the system() function to execute the command
    int result = system(command);

    // Check for any errors in executing the command
    if (result == -1)
    {
        perror("Error executing command");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int pipeline_execution(char *input, char segmentedString[][MAX_TOKEN_LENGTH], int *inputCount)
{
    int status = 0;
    int total_commands = *inputCount;
    int total_pipes = total_commands - 1;

    int pid_array[total_pipes - 1]; // will be used for managing the loop children
    int pid3 = -1;

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
        strcpy(input, segmentedString[0]);
        parseString(input, parsedString, &wordCount); // Call the parseString function
        status = dedsec_execution(input, parsedString, &wordCount);
        if (status == 2)
        {
            // terminate_loop = 1;
            kill(getppid(), SIGUSR1);
        }
        else if (status == 1)
        {
            // command failed
            exit(1);
        }
        else
        {
            // command success
            exit(0);
        }
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
            strcpy(input, segmentedString[p + 1]);
            parseString(input, parsedString, &wordCount); // Call the parseString function

            status = dedsec_execution(input, parsedString, &wordCount);
            if (status == 2)
            {
                // terminate_loop = 1;
                kill(getppid(), SIGUSR1);
            }
            else if (status == 1)
            {
                // command failed
                exit(1);
            }
            else
            {
                // command success
                exit(0);
            }
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
            strcpy(input, segmentedString[total_pipes]);
            parseString(input, parsedString, &wordCount); // Call the parseString function

            status = dedsec_execution(input, parsedString, &wordCount);
            if (status == 2)
            {
                // terminate_loop = 1;
                kill(getppid(), SIGUSR1);
            }
            else if (status == 1)
            {
                // command failed
                exit(1);
            }
            else
            {
                // command success
                exit(0);
            }
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
    int child_status;
    waitpid(pid1, &child_status, 0);
    if (WIFEXITED(child_status))
    {
        int exit_status = WEXITSTATUS(child_status);
        if (exit_status != 0)
        {
            status = exit_status; // Update status if the first child process exits with non-zero status
        }
    }

    // Step 6: Wait for the remaining children to finish and update status
    for (int i = 0; i < total_pipes - 1; i++)
    {
        waitpid(pid_array[i], &child_status, 0);
        if (WIFEXITED(child_status))
        {
            int exit_status = WEXITSTATUS(child_status);
            if (exit_status != 0)
            {
                status = exit_status; // Update status if any child process exits with non-zero status
            }
        }
    }

    // If there is at least one pipe, wait for the last child and update status
    if (total_pipes > 0)
    {
        waitpid(pid3, &child_status, 0);
        if (WIFEXITED(child_status))
        {
            int exit_status = WEXITSTATUS(child_status);
            if (exit_status != 0)
            {
                status = exit_status; // Update status if the last child process exits with non-zero status
            }
        }
    }

    return status; // to cater for non-void function
}

int dedsec_execution(char *input, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount)
{
    // io redirection
    int status = 0;
    int redirection_flag = -1;
    int t = -2;
    int io_redirection_error_handling = 0;
    redirection_flag = redirection_check(input);
    if ((redirection_flag == 1) || (redirection_flag == 2) || (redirection_flag == 3)) // ">" or ">>" or "<" detection confirmed
    {
        if (redirection_flag == 1)
        {
            special_tokenizer(input, ">", parsedString, wordCount);
        }
        else if (redirection_flag == 2)
        {
            special_tokenizer(input, ">>", parsedString, wordCount);
        }
        else if (redirection_flag == 3)
        {
            special_tokenizer(input, "<", parsedString, wordCount);
        }
        char *filename = strtok(parsedString[*wordCount - 1], " "); // removing spaces
        if ((redirection_flag == 1) || (redirection_flag == 2))
        {
            t = redirect_output(filename, redirection_flag); // redirect from terminal to the file
        }
        else if (redirection_flag == 3)
        {
            t = redirect_input(filename);
            if (t == -1)
            {
                io_redirection_error_handling = 1;
            }
        }
        // changes need to be made for updating input, parsedString and wordCount here
        *wordCount -= 2;                // this takes care of both; wordcount and parsedString array (technically!)
        strcpy(input, parsedString[0]); // this takes care of the input
        for (int i = 1; i < *wordCount; i++)
        {
            strcat(input, " ");
            strcat(input, parsedString[i]);
        }

        // parsing again
        parseString(input, parsedString, wordCount);
    }
    if (io_redirection_error_handling == 0)
    {
        // First check if the input is alias or not
        alias_check(input, parsedString, wordCount);

        // onto builtins
        int exitFlag = 0; // Boolean flag to indicate whether to exit the loop

        // check if input has wildcard characters
        if (strstr(input, "*") != NULL || strstr(input, "?") != NULL)
        {
            char wildcard_input[MAX_TOKENS]; // Array to store parsed words
            strcpy(wildcard_input, "");      // Initialize the string
            wildcardExpansion(input, wildcard_input);

            char wild_parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH];
            int wild_wordCount;
            parseString(wildcard_input, wild_parsedString, &wild_wordCount);
            status = built_in_functions(wildcard_input, wild_parsedString, &exitFlag, wild_wordCount);
        }
        else
        {
            status = built_in_functions(input, parsedString, &exitFlag, *wordCount);
        }

        if (exitFlag == 1)
        {
            return 2; // exiting loop
        }

        // io redirection defaulted to terminal
        if (redirection_flag != -1 && t != -1)
        {
            default_redirection(t, redirection_flag); // redirect from file to the terminal
        }
    }

    return status;
}

int built_in_functions(const char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *exitFlag, int wordCount)
{
    int status = 0; // Initialize status to indicate successful execution
    if (strcmp(parsedString[0], "dedsec") == 0)
    {
        printf("We are Game!!\n");
    }
    else if (strcmp(parsedString[0], "version") == 0)
    {
        if (version == 0)
        {
            version = 1;
        }
        else
        {
            version = 0;
        }
    }
    else if (strncmp(parsedString[0], "Hello", 5) == 0 || strncmp(parsedString[0], "hello", 5) == 0)
    {
        char *temp = (char *)inputString;
        cycle(temp);
    }
    else if (strcmp(parsedString[0], "exit") == 0 || strcmp(parsedString[0], "kill") == 0) // built in function "exit" implemented
    {
        if (strcmp(parsedString[0], "exit") == 0)
        {
            printf("logout\n\n");
            *exitFlag = 1; // Exit the loop
        }
        else
        {
            printf("killing Dedsec...\n\n");
            *exitFlag = 1; // Exit the loop
        }
    }
    else if (strcmp(parsedString[0], "clear") == 0)
    {
        printf("\033[H\033[J");
    }
    else if (strcmp(parsedString[0], "pwd") == 0)
    {
        char cwd[1000]; // A buffer to store the current directory
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("getcwd");
        }
    }
    else if (strcmp(parsedString[0], "cd") == 0)
    {
        if (wordCount > 2)
        {
            printf("cd: too many arguments\n");
        }
        else if (wordCount == 1) // implemented home directory
        {
            if (chdir(getenv("HOME")) != 0)
            {
                perror("chdir");
            }
        }
        else // implemented ".." and "/something"
        {
            if (chdir(parsedString[1]) != 0)
            {
                perror("chdir");
            }
        }
    }
    else if (strcmp(parsedString[0], "alias") == 0)
    {
        if (wordCount > 3)
        {
            printf("alias: too many arguments\n");
            return 1;
        }
        if (wordCount == 3)
        {
            // adding alias
            for (int i = 0; i < aliasCount; i++)
            {
                if (strcmp(aliasArray[i].name, parsedString[1]) == 0)
                {
                    // Override existing alias
                    strcpy(aliasArray[i].value, parsedString[2]);
                    return 0;
                }
            }

            // If alias does not exist, add a new one
            if (aliasCount < MAX_ALIASES)
            {
                strcpy(aliasArray[aliasCount].name, parsedString[1]);
                strcpy(aliasArray[aliasCount].value, parsedString[2]);
                (aliasCount)++;
            }
            else
            {
                printf("alias: maximum number of aliases reached. cannot add more aliases.\n");
            }
            return 1;
        }
        else if (wordCount == 2)
        {
            // search alias
            for (int i = 0; i < aliasCount; i++)
            {
                if (strcmp(aliasArray[i].name, parsedString[1]) == 0)
                {
                    printf("%s='%s'\n", aliasArray[i].name, aliasArray[i].value);
                    return 0;
                }
            }
            printf("alias: %s: not found.\n", parsedString[1]);
            return 0;
        }
        else
        {
            // print all aliases
            for (int i = 0; i < aliasCount; i++)
            {
                printf("%s='%s'\n", aliasArray[i].name, aliasArray[i].value);
            }
            return 0;
        }
    }
    else if (strcmp(parsedString[0], "unalias") == 0)
    {
        if (wordCount >= 2)
        {
            for (int a = 1; a < wordCount; a++)
            {
                for (int i = 0; i < aliasCount; i++)
                {
                    if (strcmp(aliasArray[i].name, parsedString[a]) == 0)
                    {
                        // Shift elements to remove alias
                        for (int j = i; j < aliasCount - 1; j++)
                        {
                            aliasArray[j] = aliasArray[j + 1];
                        }
                        (aliasCount)--;
                        return 0;
                    }
                }
            }
            printf("%s: ds: not found.\n", parsedString[1]);
        }
        else
        {
            printf("unalias: usage: unalias name [name ...] \n");
        }
    }
    else if (strcmp(parsedString[0], "echo") == 0)
    {
        char echoString[1000]; // Make a copy of the input string to avoid modifying the original
        strcpy(echoString, inputString);
        if (strlen(echoString) > 5)
        {
            memmove(echoString, echoString + 5, strlen(echoString) - 5 + 1);
            int echo_count;
            char parsedEcho[MAX_TOKENS][MAX_TOKEN_LENGTH];
            special_tokenizer(echoString, "\"", parsedEcho, &echo_count);

            for (int i = 0; i < echo_count; i++)
            {
                printf("%s", parsedEcho[i]);
            }
            printf("\n");
        }
        else
        {
            printf("\n"); // print nothing
        }
    }
    else if (strcmp(parsedString[0], "history") == 0)
    {
        HIST_ENTRY **history_entries = history_list();

        // remove consecutive duplicate entries in history

        if (wordCount > 2)
        {
            printf("history: too many arguments\n");
            return 1;
        }
        if (wordCount == 1) // display complete history
        {
            if (history_entries)
            {
                int i = 0;
                while (history_entries[i])
                {
                    printf("%d: %s\n", i + 1, history_entries[i]->line);
                    i++;
                }
            }
            return 0;
        }
        if (isNumeric(parsedString[1]))
        {
            printf("history: numeric argument required\n");
        }
        else // display part of history on the basis of index passed
        {
            int i = history_length - atoi(parsedString[1]);
            if (i < 0)
            {
                i = 0;
            }
            while (history_entries[i])
            {
                printf("%d: %s\n", i + 1, history_entries[i]->line);
                i++;
            }
        }
    }
    else // fork-exec model implemented here.
    {
        pid_t pid = fork();

        if (pid == -1) // fork failed
        {
            perror("fork");
            status = 1; // indicates failure
        }

        if (pid == 0) // Child process
        {
            char *args[wordCount + 1];
            for (int i = 0; i < wordCount; i++)
            {
                args[i] = strdup(parsedString[i]);
            }
            args[wordCount] = NULL;
            execvp(parsedString[0], args);

            // If execvp fails
            // perror("execvp");
            printf("%s: command not found\n", parsedString[0]);
            exit(EXIT_FAILURE);
        }
        else // Parent process (pid > 0) (it is just going to wait for the child process to complete)
        {
            // int status;
            // wait(&status); // Wait for the child process to complete
            int childStatus;
            waitpid(pid, &childStatus, 0); // Wait for the child process to complete
            if (!WIFEXITED(childStatus) || WEXITSTATUS(childStatus) != 0)
            {
                // If the child process did not terminate normally or exited with a non-zero status
                status = 1; // Indicate failure
            }
        }
    }
    return status; // Return the status
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

int script_mode(int *script_flag, int argc, char *argv[], char file_lines[][BUFSIZ], int *file_line_count)
{
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return 1;
    }

    if (argc == 1)
    {
        // printf("No file detected\n");
        *script_flag = 0; // interactive mode
        return 0;         // don't need this for dedsec
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        perror("Error opening file");
        return 1;
    }

    int length;

    while (*file_line_count < MAX_FILE_LINES && fgets(file_lines[*file_line_count], sizeof(file_lines[*file_line_count]), file) != NULL)
    {
        length = strlen(file_lines[*file_line_count]);
        if (file_lines[*file_line_count][length - 2] == '\r') // for .txt which is '\r\n'
        {
            file_lines[*file_line_count][length - 2] = '\0';
            length--;
        }
        if (file_lines[*file_line_count][length - 1] == '\n')
        {
            file_lines[*file_line_count][length - 1] = '\0'; // Remove newline (:/)
        }
        (*file_line_count)++;
    }

    fclose(file);

    return 0;
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

        // printf("\"%s\" was found in the string.\n", focus_char);
    }
    else
    {
        // nothing happens generally
        strcpy(parsedString[0], copyString);
        *wordCount = 1;

        // printf("\"%s\" was NOT found in the string.\n", focus_char);
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

int isNumeric(const char *input)
{
    // Iterate through each character in the input string
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i++)
    {
        // If any character is not a digit, return false
        if (!isdigit(input[i]))
        {
            return 1;
        }
    }
    // If all characters are digits, return true
    return 0;
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

void replaceFirstOccurrence(char *str, const char *oldWord, const char *newWord)
{
    char *pos = strstr(str, oldWord);

    // Check if the word is found
    if (pos != NULL)
    {
        memmove(pos + strlen(newWord), pos + strlen(oldWord), strlen(pos + strlen(oldWord)) + 1);
        memcpy(pos, newWord, strlen(newWord));
    }
}

void alias_check(char *inputString, char parsedString[][MAX_TOKEN_LENGTH], int *wordCount)
{
    for (int i = 0; i < aliasCount; i++)
    {
        if (strcmp(aliasArray[i].name, parsedString[0]) == 0)
        {
            char tempInput[MAX_TOKEN_LENGTH];
            char tempString[MAX_TOKEN_LENGTH];
            strcpy(tempInput, inputString);
            strcpy(tempString, parsedString[0]);

            strcpy(inputString, aliasArray[i].value);
            // save the previous parsedString
            char alias_save[MAX_TOKENS][MAX_TOKEN_LENGTH];
            int alias_wordCount = *wordCount;
            for (int a = 1; a < *wordCount; a++)
            {
                strcpy(alias_save[a - 1], parsedString[a]);
            }

            // Reset parsedString array to its initial state
            for (int j = 0; j < MAX_TOKENS; j++)
            {
                memset(parsedString[j], '\0', sizeof(parsedString[j]));
            }
            parseString(inputString, parsedString, wordCount);

            // append alias_save array to parsedString
            for (int a = 0; a < alias_wordCount; a++)
            {
                strcpy(parsedString[a + *wordCount], alias_save[a]);
            }
            *wordCount = *wordCount + alias_wordCount - 1;

            // changing the input accordingly
            replaceFirstOccurrence(tempInput, tempString, aliasArray[i].value);
            strcpy(inputString, tempInput);

            break;
        }
    }
}

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
    if (strcmp(copyParsedString[copyWordCount - 2], ">") == 0)
    {
        return 1;
    }
    else if (strcmp(copyParsedString[copyWordCount - 2], ">>") == 0)
    {
        return 2;
    }
    else if (strcmp(copyParsedString[copyWordCount - 2], "<") == 0)
    {
        return 3;
    }
    else
    {
        return -1; // no redirection needed
    }
}

int redirect_output(const char *filename, int flag)
{
    // Disable buffering for stdout
    setbuf(stdout, NULL);

    int file_fd;
    terminal_stdout = dup(STDOUT_FILENO); // terminal fd saved
    if (flag == 1)                        // ">" is confirmed
    {
        file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    else // ">>" is confirmed
    {
        file_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
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
        printf("-bash: %s: No such file or directory\n", filename);
        return -1;
    }

    // Redirect stdin to the file
    dup2(file_fd, STDIN_FILENO);

    // Close the file descriptor for the file (stdin is now associated with it)
    close(file_fd);

    return terminal_stdin;
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

void replaceSubstring(char *str, const char *oldSubstr, const char *newSubstr)
{
    char *position = strstr(str, oldSubstr);

    while (position != NULL)
    {
        // Calculate the length of the prefix before the old substring
        size_t prefixLength = position - str;

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

int chain_execute(int status, char *input)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process

        char basic_breakdown_String[MAX_TOKENS][MAX_TOKEN_LENGTH];              // Array to store parsed words
        int basic_breakdown_wordCount;                                          // Number of words in the parsed string
        parseString(input, basic_breakdown_String, &basic_breakdown_wordCount); // Call the parseString function
        alias_check(input, basic_breakdown_String, &basic_breakdown_wordCount);
        if (strstr(input, " | ") != NULL && strcmp(basic_breakdown_String[0], "alias") != 0)
        {
            char segmentedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
            int inputCount;                                     // Number of words in the parsed string
            int output_status;
            high_level_parser(input, "|", segmentedString, &inputCount);
            output_status = pipeline_execution(input, segmentedString, &inputCount); // cater its returns!!
            if (terminate_loop)
            {
                // break; // This will break out of the outer loop
                kill(getppid(), SIGUSR1);
            }
            else if (output_status == 1)
            {
                exit(1);
            }
            else
            {
                exit(0);
            }
        }
        else
        {
            char parsedString[MAX_TOKENS][MAX_TOKEN_LENGTH]; // Array to store parsed words
            int wordCount;                                   // Number of words in the parsed string
            int output_status;
            parseString(input, parsedString, &wordCount); // Call the parseString function
            output_status = dedsec_execution(input, parsedString, &wordCount);
            if (output_status == 2)
            {
                // break;
                // exit(1);
                kill(getppid(), SIGUSR1);
            }
            else if (output_status == 1)
            {
                exit(1);
            }
            else
            {
                exit(0);
            }
        }
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

            for (size_t j = 0; j < glob_result[i].gl_pathc; j++)
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