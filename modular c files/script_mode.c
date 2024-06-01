#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // For Boolean datatype

#include <readline/readline.h>
#include <readline/history.h>

// gcc ./"modular c files"/script_mode.c -o compiled/script_mode && ./compiled/script_mode txt_outputs/simple.txt
// gcc ./"modular c files"/script_mode.c -o compiled/script_mode && ./compiled/script_mode txt_outputs/simple.test
// gcc ./"modular c files"/script_mode.c -o compiled/script_mode && ./compiled/script_mode
// gcc ./"modular c files"/script_mode.c -o compiled/script_mode && ./compiled/script_mode txt_outputs/simple.test example

int script_mode(bool *script_flag, int argc, char *argv[], char file_lines[][BUFSIZ], int *file_line_count);

#define MAX_FILE_LINES 1000

int main(int argc, char *argv[])
{
    bool script_flag = true;
    char file_lines[MAX_FILE_LINES][BUFSIZ]; // Array to store lines
    int file_line_count = 0;
    if (script_mode(&script_flag, argc, argv, file_lines, &file_line_count) == 1)
    {
        return EXIT_FAILURE;
    }

    int file_track = 0;
    printf("Contents of the file are:\n");
    char *input;
    while (1) // simulating shell
    {
        if (script_flag = true) // shell readline implemented here
        {
            input = file_lines[file_track];
        }
        else
        {
            input = readline("");
        }
        printf("%d:%s\n", file_track + 1, input);
        // print the lenght of the line
        printf("Length of line %d = %ld\n", file_track + 1, strlen(input));
        file_track++;
        if ((script_flag = true) && (file_track == file_line_count))
        {
            break;
        }
    }

    return EXIT_SUCCESS;
}

int script_mode(bool *script_flag, int argc, char *argv[], char file_lines[][BUFSIZ], int *file_line_count)
{
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return 1;
    }

    if (argc == 1)
    {
        printf("No file detected\n");
        *script_flag = false; // interactive mode
        return 1;             // don't need this for dedsec
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
        if (file_lines[*file_line_count][length - 1] == '\n')
        {
            file_lines[*file_line_count][length - 1] = '\0'; // Remove newline (:/)
        }
        (*file_line_count)++;
    }

    fclose(file);
}
