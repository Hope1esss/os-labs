#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#define SHARED_MEMORY_SIZE 1024
#define SHM_NAME1 "/shm1"
#define SHM_NAME2 "/shm2"

int random_number()
{
    return rand() % 100;
}

int main()
{
    srand(time(NULL));

    int shm_fd1 = shm_open(SHM_NAME1, O_CREAT | O_RDWR, 0666);
    int shm_fd2 = shm_open(SHM_NAME2, O_CREAT | O_RDWR, 0666);
    
    if (shm_fd1 == -1 || shm_fd2 == -1)
    {
        perror("shm_open error");
        close(shm_fd1);
        close(shm_fd2);
        shm_unlink(SHM_NAME1);
        shm_unlink(SHM_NAME2);
        return 1;
    }

    if (ftruncate(shm_fd1, SHARED_MEMORY_SIZE) == -1 || ftruncate(shm_fd2, SHARED_MEMORY_SIZE) == -1)
    {
        perror("ftruncate error");
        close(shm_fd1);
        close(shm_fd2);
        shm_unlink(SHM_NAME1);
        shm_unlink(SHM_NAME2);
        return 1;
    }

    char *shared_memory1 = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd1, 0);
    char *shared_memory2 = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);

    if (shared_memory1 == MAP_FAILED || shared_memory2 == MAP_FAILED)
    {
        perror("mmap error");
        close(shm_fd1);
        close(shm_fd2);
        shm_unlink(SHM_NAME1);
        shm_unlink(SHM_NAME2);
        return 1;
    }

    char file1[SHARED_MEMORY_SIZE], file2[SHARED_MEMORY_SIZE];
    printf("Enter file1 name: ");
    scanf("%s", file1);
    printf("Enter file2 name: ");
    scanf("%s", file2);

    pid_t child1_pid = fork();
    if (child1_pid == -1)
    {
        perror("fork error");
        munmap(shared_memory1, SHARED_MEMORY_SIZE);
        munmap(shared_memory2, SHARED_MEMORY_SIZE);
        close(shm_fd1);
        close(shm_fd2);
        shm_unlink(SHM_NAME1);
        shm_unlink(SHM_NAME2);
        return 1;
    }

    if (child1_pid == 0)
    {
        execl("./child", "./child", SHM_NAME1, file1, NULL);
        perror("execl error");
        return 1;
    }

    pid_t child2_pid = fork();
    if (child2_pid == -1)
    {
        perror("fork error");
        munmap(shared_memory1, SHARED_MEMORY_SIZE);
        munmap(shared_memory2, SHARED_MEMORY_SIZE);
        close(shm_fd1);
        close(shm_fd2);
        shm_unlink(SHM_NAME1);
        shm_unlink(SHM_NAME2);
        return 1;
    }

    if (child2_pid == 0)
    {
        execl("./child", "./child", SHM_NAME2, file2, NULL);
        perror("execl error");
        return 1;
    }

    char input[1024];
    while (1)
    {
        printf("Enter string (use 'exit' to exit from the program): ");
        scanf("%s", input);

        if (strcmp(input, "exit") == 0)
        {
            strncpy(shared_memory1, "exit", SHARED_MEMORY_SIZE);
            strncpy(shared_memory2, "exit", SHARED_MEMORY_SIZE);
            break;
        }

        if (random_number() > 20)
        {
            strncpy(shared_memory1, input, SHARED_MEMORY_SIZE);
        }
        else
        {
            strncpy(shared_memory2, input, SHARED_MEMORY_SIZE);
        }
    }

    wait(NULL);
    wait(NULL);

    munmap(shared_memory1, SHARED_MEMORY_SIZE);
    munmap(shared_memory2, SHARED_MEMORY_SIZE);
    close(shm_fd1);
    close(shm_fd2);
    shm_unlink(SHM_NAME1);
    shm_unlink(SHM_NAME2);
    return 0;
}