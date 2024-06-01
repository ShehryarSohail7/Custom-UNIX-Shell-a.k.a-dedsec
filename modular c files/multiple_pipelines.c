#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// gcc ./"modular c files"/multiple_pipelines.c -o compiled/multiple_pipelines && ./compiled/multiple_pipelines

// test : cat file.txt | sort | grep 2

int main()
{
    // char *example[][1000] = {
    //     {"cat", "file.txt", NULL},
    //     {"sort", NULL},
    //     {"grep", "2", NULL},
    //     {NULL}};

    // printf("example[0][0] = %s\n", example[0][0]);

    // pipeline basic declarations
    int total_pipes = 134;
    int total_pids[total_pipes - 1];

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

    // // step 4: fork 3rd child
    int pid3 = fork();
    if (pid3 < 0)
    {
        fprintf(stderr, "3rd Fork Failed");
        return 1;
    }

    if (pid3 == 0) // child of 3rd fork
    {
        // all we need are fd[2][0] and fd[0][1]
        for (int i = 0; i < total_pipes; i++)
        {
            if (i != total_pipes - 1)
            {
                close(fd[i][0]); // close read end of pipe
            }
            if (i != 0)
            {
                close(fd[i][1]); // close read end of pipe
            }
        }

        // in this one, we will first write and then read cuz this is the start and end point
        // of the pipeline cycle
        int x;
        x = 0;

        if (write(fd[0][1], &x, sizeof(int)) == -1)
        {
            fprintf(stderr, "Write Failed");
            return 1;
        }

        if (read(fd[total_pipes - 1][0], &x, sizeof(int)) == -1)
        {
            fprintf(stderr, "Read Failed");
            return 1;
        }

        printf("Final value: %d\n", x);

        // close these ends now
        close(fd[total_pipes - 1][0]);
        close(fd[0][1]);

        // finish this process
        return 0;
    }

    for (int p = 0; p < total_pipes - 1; p++)
    {
        total_pids[p] = fork();
        if (total_pids[p] < 0)
        {
            fprintf(stderr, "1st Fork Failed");
            return 1;
        }

        if (total_pids[p] == 0) // child of 1st fork
        {

            for (int i = 0; i < total_pipes; i++)
            {
                if (i != p)
                {
                    close(fd[i][0]); // close read end of pipe
                }
                if (i != p + 1)
                {
                    close(fd[i][1]); // close read end of pipe
                }
            }

            int x;

            if (read(fd[p][0], &x, sizeof(int)) == -1)
            {
                fprintf(stderr, "Read Failed");
                return 1;
            }

            x = x + 5;

            if (write(fd[p + 1][1], &x, sizeof(int)) == -1)
            {
                fprintf(stderr, "Write Failed");
                return 1;
            }

            // close these ends now
            close(fd[p][0]);
            close(fd[p + 1][1]);

            // finish this process
            return 0;
        }
    }

    // step 5: close all ends of the pipe in the parent process as none are needed here
    for (int i = 0; i < total_pipes; i++)
    {
        close(fd[i][0]);
        close(fd[i][1]);
    }

    // step 6: wait for all children to finish
    for (int i = 0; i < total_pipes - 1; i++)
    {
        wait(NULL);
    }
    printf("All children finished and we are back in the main()\n");

    return 0;
}

// ================== Loop version ==================

// int main()
// {
//     // pipeline basic declarations
//     int total_pipes = 3;
//     int total_pids[total_pipes - 1];

//     // step 1 : create pipe
//     int fd[total_pipes][2]; // fd[0] is read end, fd[1] is write end
//     for (int i = 0; i < total_pipes; i++)
//     {
//         if (pipe(fd[i]) == -1)
//         {
//             fprintf(stderr, "Pipe Failed");
//             return 1;
//         }
//     }

//     // // step 4: fork 3rd child
//     int pid3 = fork();
//     if (pid3 < 0)
//     {
//         fprintf(stderr, "3rd Fork Failed");
//         return 1;
//     }

//     if (pid3 == 0) // child of 3rd fork
//     {
//         // all we need are fd[2][0] and fd[0][1]
//         for (int i = 0; i < total_pipes; i++)
//         {
//             if (i != total_pipes - 1)
//             {
//                 close(fd[i][0]); // close read end of pipe
//             }
//             if (i != 0)
//             {
//                 close(fd[i][1]); // close read end of pipe
//             }
//         }

//         // in this one, we will first write and then read cuz this is the start and end point
//         // of the pipeline cycle
//         int x;
//         x = 0;

//         if (write(fd[0][1], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Write Failed");
//             return 1;
//         }

//         if (read(fd[total_pipes - 1][0], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Read Failed");
//             return 1;
//         }

//         printf("Final value: %d\n", x);

//         // close these ends now
//         close(fd[total_pipes - 1][0]);
//         close(fd[0][1]);

//         // finish this process
//         return 0;
//     }

//     for (int p = 0; p < total_pipes - 1; p++)
//     {
//         total_pids[p] = fork();
//         if (total_pids[p] < 0)
//         {
//             fprintf(stderr, "1st Fork Failed");
//             return 1;
//         }

//         if (total_pids[p] == 0) // child of 1st fork
//         {

//             for (int i = 0; i < total_pipes; i++)
//             {
//                 if (i != p)
//                 {
//                     close(fd[i][0]); // close read end of pipe
//                 }
//                 if (i != p + 1)
//                 {
//                     close(fd[i][1]); // close read end of pipe
//                 }
//             }

//             int x;

//             if (read(fd[p][0], &x, sizeof(int)) == -1)
//             {
//                 fprintf(stderr, "Read Failed");
//                 return 1;
//             }

//             x = x + 5;

//             if (write(fd[p + 1][1], &x, sizeof(int)) == -1)
//             {
//                 fprintf(stderr, "Write Failed");
//                 return 1;
//             }

//             // close these ends now
//             close(fd[p][0]);
//             close(fd[p + 1][1]);

//             // finish this process
//             return 0;
//         }
//     }

//     // step 5: close all ends of the pipe in the parent process as none are needed here
//     for (int i = 0; i < total_pipes; i++)
//     {
//         close(fd[i][0]);
//         close(fd[i][1]);
//     }

//     // step 6: wait for all children to finish
//     for (int i = 0; i < total_pipes - 1; i++)
//     {
//         wait(NULL);
//     }
//     printf("All children finished and we are back in the main()\n");

//     return 0;
// }

// ==================Basics==================

// int main()
// {
//     // step 1 : create pipe
//     int fd[3][2]; // fd[0] is read end, fd[1] is write end
//     for (int i = 0; i < 3; i++)
//     {
//         if (pipe(fd[i]) == -1)
//         {
//             fprintf(stderr, "Pipe Failed");
//             return 1;
//         }
//     }

//     // step 2: fork 1st child
//     int pid1 = fork();
//     if (pid1 < 0)
//     {
//         fprintf(stderr, "1st Fork Failed");
//         return 1;
//     }

//     if (pid1 == 0) // child of 1st fork
//     {
//         // all we need are fd[0][0] and fd[1][1]
//         close(fd[0][1]); // close write end of pipe
//         close(fd[1][0]); // close read end of pipe
//         close(fd[2][0]); // close read end of pipe
//         close(fd[2][1]); // close write end of pipe

//         int x;

//         if (read(fd[0][0], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Read Failed");
//             return 1;
//         }

//         x = x + 5;

//         if (write(fd[1][1], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Write Failed");
//             return 1;
//         }

//         // close these ends now
//         close(fd[0][0]);
//         close(fd[1][1]);

//         // finish this process
//         return 0;
//     }

//     // step 3: fork 2nd child
//     int pid2 = fork();
//     if (pid2 < 0)
//     {
//         fprintf(stderr, "2nd Fork Failed");
//         return 1;
//     }

//     if (pid2 == 0) // child of 2nd fork
//     {
//         // all we need are fd[1][0] and fd[2][1]
//         close(fd[1][1]); // close write end of pipe
//         close(fd[2][0]); // close read end of pipe
//         close(fd[0][0]); // close read end of pipe
//         close(fd[0][1]); // close write end of pipe

//         int x;

//         if (read(fd[1][0], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Read Failed");
//             return 1;
//         }

//         x = x + 5;

//         if (write(fd[2][1], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Write Failed");
//             return 1;
//         }

//         // close these ends now
//         close(fd[1][0]);
//         close(fd[2][1]);

//         // finish this process
//         return 0;
//     }

//     // // step 4: fork 3rd child
//     int pid3 = fork();
//     if (pid3 < 0)
//     {
//         fprintf(stderr, "3rd Fork Failed");
//         return 1;
//     }

//     if (pid3 == 0) // child of 3rd fork
//     {
//         // all we need are fd[2][0] and fd[0][1]
//         close(fd[2][1]); // close write end of pipe
//         close(fd[0][0]); // close read end of pipe
//         close(fd[1][0]); // close read end of pipe
//         close(fd[1][1]); // close write end of pipe

//         // in this one, we will first write and then read cuz this is the start and end point
//         // of the pipeline cycle
//         int x;
//         x = 0;

//         if (write(fd[0][1], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Write Failed");
//             return 1;
//         }

//         if (read(fd[2][0], &x, sizeof(int)) == -1)
//         {
//             fprintf(stderr, "Read Failed");
//             return 1;
//         }

//         printf("Final value: %d\n", x);

//         // close these ends now
//         close(fd[2][0]);
//         close(fd[0][1]);

//         // finish this process
//         return 0;
//     }

//     // step 5: close all ends of the pipe in the parent process as none are needed here
//     close(fd[0][0]);
//     close(fd[0][1]);
//     close(fd[1][0]);
//     close(fd[1][1]);
//     close(fd[2][0]);
//     close(fd[2][1]);

//     // step 6: wait for all children to finish
//     waitpid(pid3, NULL, 0); // wait for 3rd child
//     waitpid(pid1, NULL, 0); // wait for 1st child
//     waitpid(pid2, NULL, 0); // wait for 2nd child

//     printf("All children finished and we are back in the main()\n");

//     return 0;
// }
