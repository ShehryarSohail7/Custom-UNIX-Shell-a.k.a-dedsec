#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for sleep()

// gcc ./"modular c files"/hello_world.c -o compiled/hello_world && ./compiled/hello_world

void cycle(char *str);

int main()
{
    cycle("Hello World!");
    return 1;
}

void cycle(char *str)
{
    printf("The string is: %s\n", str);
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
