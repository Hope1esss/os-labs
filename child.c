#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>

#define SHARED_MEMORY_SIZE 1024

void reverse_string(char *string)
{
    int len = strlen(string);
    for (int i = 0; i < len / 2; i++)
    {
        char temp = string[i];
        string[i] = string[len - i - 1];
        string[len - i - 1] = temp;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("Invalid number of arguments in child");
        return 1;
    }

    char *shared_memory_name = argv[1];
    char *output_file = argv[2];

    int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);

    if (shm_fd == -1)
    {
        perror("shm_open error");
        return 1;
    }

    char *shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shared_memory == MAP_FAILED)
    {
        perror("mmap error");
        close(shm_fd);
        shm_unlink(shared_memory_name);
        return 1;
    }

    while (1)
    {
        if (strlen(shared_memory) > 0)
        {
            if (strcmp(shared_memory, "exit") == 0)
            {
                break;
            }

            reverse_string(shared_memory);

            FILE *file = fopen(output_file, "a");

            if (!file)
            {
                perror("error while opening output file");
                munmap(shared_memory, SHARED_MEMORY_SIZE);
                close(shm_fd);
                shm_unlink(shared_memory_name);
                return 1;
            }

            fprintf(file, "%s\n", shared_memory);
            fclose(file);

            memset(shared_memory, 0, SHARED_MEMORY_SIZE);
        }
        usleep(500);
    }

    munmap(shared_memory, SHARED_MEMORY_SIZE);
    close(shm_fd);
    shm_unlink(shared_memory_name);
    return 0;
}