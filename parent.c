#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

int random()
{
    return rand() % 100;
}

int main()
{
    srand(time(NULL));

    int pipe1[2], pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("pipe error");
        return 1;
    }

    char file1[256], file2[256];

    printf("Enter file1 name: ");
    scanf("%s", file1);
    printf("Enter file2 name: ");
    scanf("%s", file2);

    pid_t child1_pid = fork();
    if (child1_pid == -1)
    {
        perror("fork error");
        return 1;
    }

    if (child1_pid == 0)
    {
        close(pipe1[1]);
        execl("./child", "./child", "0", file1, NULL);
        perror("execl error");
        return 1;
    }

    pid_t child2_pid = fork();
    if (child2_pid == -1)
    {
        perror("fork error");
        return 1;
    }

    if (child2_pid == 0)
    {
        close(pipe2[1]);
        execl("./child", "./child", "1", file2, NULL);
        perror("execl error");
        return 1;
    }

    close(pipe1[0]);
    close(pipe2[0]);

    char input[1024];
    while (1)
    {
        printf("Enter string (use 'exit' to exit from program): ");
        scanf("%s", input);

        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        if (random() > 20)
        {
            write(pipe1[1], input, strlen(input));
        }
        else
        {
            write(pipe2[1], input, strlen(input));
        }
    }

    close(pipe1[1]);
    close(pipe2[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
