#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int random_number()
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

    if (child1_pid == 0)  // Child 1
    {
        close(pipe1[1]);  // Close unused write end
        dup2(pipe1[0], STDIN_FILENO);  // Redirect stdin to read end of pipe
        close(pipe1[0]);  // Close the original pipe read end

        execl("./child", "./child", "stdin", file1, NULL);
        perror("execl error");
        return 1;
    }

    pid_t child2_pid = fork();
    if (child2_pid == -1)
    {
        perror("fork error");
        return 1;
    }

    if (child2_pid == 0)  // Child 2
    {
        close(pipe2[1]);  // Close unused write end
        dup2(pipe2[0], STDIN_FILENO);  // Redirect stdin to read end of pipe
        close(pipe2[0]);  // Close the original pipe read end

        execl("./child", "./child", "stdin", file2, NULL);
        perror("execl error");
        return 1;
    }

    close(pipe1[0]);  // Parent closes read end of pipe1
    close(pipe2[0]);  // Parent closes read end of pipe2

    char input[1024];
    while (1)
    {
        printf("Enter string (use 'exit' to exit from program): ");
        scanf("%s", input);

        if (strcmp(input, "exit") == 0)
        {
            // Send the exit signal to both children
            write(pipe1[1], "exit", strlen("exit") + 1);  // +1 for null terminator
            write(pipe2[1], "exit", strlen("exit") + 1);  // +1 for null terminator
            break;
        }

        if (random_number() > 20)
        {
            write(pipe1[1], input, strlen(input) + 1);  // +1 to send null terminator
        }
        else
        {
            write(pipe2[1], input, strlen(input) + 1);  // +1 to send null terminator
        }
    }

    close(pipe1[1]);  // Close the write ends in the parent
    close(pipe2[1]);

    wait(NULL);
    wait(NULL);

    return 0;
}
