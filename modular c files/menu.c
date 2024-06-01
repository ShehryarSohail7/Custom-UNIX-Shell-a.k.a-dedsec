#include <stdio.h>
#include <ncurses.h>

// gcc ./"modular c files"/menu.c -o compiled/menu -lncurses && ./compiled/menu

int main()
{
    int choice = 0;
    int ch;

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    do
    {
        clear(); // Clear screen
        printw("Select an option:\n");
        printw("%c Option 1\n", (choice == 0) ? '*' : ' ');
        printw("%c Option 2\n", (choice == 1) ? '*' : ' ');
        printw("%c Option 3\n", (choice == 2) ? '*' : ' ');
        printw("%c Option 4\n", (choice == 3) ? '*' : ' ');
        refresh();

        ch = getch(); // Get character

        switch (ch)
        {
        case KEY_UP:
            if (choice > 0)
                choice--;
            break;
        case KEY_DOWN:
            if (choice < 3)
                choice++;
            break;
        }
    } while (ch != '\n'); // Repeat until Enter is pressed

    clear();
    printw("Option %d is under development.\n", choice + 1);
    refresh();

    // Clean up ncurses
    getch();
    endwin();

    return 0;
}
