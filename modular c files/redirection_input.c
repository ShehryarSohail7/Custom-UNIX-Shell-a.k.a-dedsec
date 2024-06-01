#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// gcc ./"modular c files"/redirection_input.c -o compiled/redirection_input && ./compiled/redirection_input

int redirect_input(const char *filename);

int main()
{
    printf("Before redirection:\n");

    // Read from the original stdin (usually the terminal)
    char input[100];
    fgets(input, sizeof(input), stdin);
    printf("Original stdin: %s", input);

    // Redirect stdin from a file
    int t = redirect_input("txt_outputs/input.txt");

    printf("\nAfter redirection:\n");

    // Read from the redirected stdin (file)
    fgets(input, sizeof(input), stdin);
    printf("Redirected stdin: %s", input);

    // Restore stdin to the original state
    dup2(t, STDIN_FILENO);
    close(t);

    // Continue reading from the original stdin
    printf("\nBack to original stdin:\n");
    fgets(input, sizeof(input), stdin);
    printf("Original stdin: %s", input);

    return 0;
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
