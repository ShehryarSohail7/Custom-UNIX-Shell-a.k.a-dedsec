#include <stdio.h>
#include <stdlib.h>

// gcc ./"modular c files"/img.c -o compiled/img && ./compiled/img

int main()
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
