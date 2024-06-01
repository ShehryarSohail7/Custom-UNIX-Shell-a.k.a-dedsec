#include <stdio.h>
#include <string.h>

// gcc ./"modular c files"/alias.c -o compiled/alias && ./compiled/alias

#define MAX_ALIASES 100

// User-Defined Datatypes
struct AliasPair
{
    char name[50];
    char value[50];
};

// Function to add or update an alias in the array
void addOrUpdateAlias(struct AliasPair aliasArray[], int *aliasCount, const char *name, const char *value)
{
    for (int i = 0; i < *aliasCount; i++)
    {
        if (strcmp(aliasArray[i].name, name) == 0)
        {
            // Override existing alias
            strcpy(aliasArray[i].value, value);
            printf("Alias overridden successfully.\n");
            return;
        }
    }

    // If alias does not exist, add a new one
    if (*aliasCount < MAX_ALIASES)
    {
        strcpy(aliasArray[*aliasCount].name, name);
        strcpy(aliasArray[*aliasCount].value, value);
        (*aliasCount)++;
        printf("Alias added successfully.\n");
    }
    else
    {
        printf("Maximum number of aliases reached. Cannot add more aliases.\n");
    }
}

// Function to search and print an alias
void searchAlias(const struct AliasPair aliasArray[], int aliasCount, const char *name)
{
    for (int i = 0; i < aliasCount; i++)
    {
        if (strcmp(aliasArray[i].name, name) == 0)
        {
            printf("Alias found: %s -> %s\n", aliasArray[i].name, aliasArray[i].value);
            return;
        }
    }
    printf("Alias not found.\n");
}

// Function to remove an alias from the array
void unalias(struct AliasPair aliasArray[], int *aliasCount, const char *name)
{
    for (int i = 0; i < *aliasCount; i++)
    {
        if (strcmp(aliasArray[i].name, name) == 0)
        {
            // Shift elements to remove alias
            for (int j = i; j < *aliasCount - 1; j++)
            {
                aliasArray[j] = aliasArray[j + 1];
            }
            (*aliasCount)--;
            printf("Alias removed successfully.\n");
            return;
        }
    }
    printf("Alias not found. Cannot remove.\n");
}

// Function to print all aliases in the array
void printAllAliases(const struct AliasPair aliasArray[], int aliasCount)
{
    if (aliasCount == 0)
    {
        printf("No aliases found.\n");
        return;
    }

    printf("List of aliases:\n");
    for (int i = 0; i < aliasCount; i++)
    {
        printf("%s -> %s\n", aliasArray[i].name, aliasArray[i].value);
    }
}

int main()
{
    // Local array to store aliases
    struct AliasPair aliasArray[MAX_ALIASES];
    int aliasCount = 0;

    // Example usage
    addOrUpdateAlias(aliasArray, &aliasCount, "cmd", "ls");
    addOrUpdateAlias(aliasArray, &aliasCount, "ff", "echo 2000");
    searchAlias(aliasArray, aliasCount, "cmd");

    addOrUpdateAlias(aliasArray, &aliasCount, "cmd", "dir");
    searchAlias(aliasArray, aliasCount, "cmd");

    printAllAliases(aliasArray, aliasCount); // Print all aliases

    unalias(aliasArray, &aliasCount, "cmd");
    searchAlias(aliasArray, aliasCount, "cmd");

    printAllAliases(aliasArray, aliasCount); // Print all aliases after removal

    return 0;
}
