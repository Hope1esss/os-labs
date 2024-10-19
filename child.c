#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void reverse_string(char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("wrong number of arguments");
        return 1;
    }

    // Using "stdin" to read from the pipe
    char *output_file = argv[2];

    char buffer[1024];

    // Continuously read from the pipe until "exit" is received
    while (1)
    {
        ssize_t read_bytes = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (read_bytes < 0)
        {
            perror("error while reading from pipe");
            return 1;
        }

        buffer[read_bytes] = '\0';

        if (strcmp(buffer, "exit") == 0)
        {
            break;  // Exit the loop if "exit" is received
        }

        reverse_string(buffer);

        FILE *file = fopen(output_file, "a");  // Append to the file
        if (!file)
        {
            perror("error while opening output file");
            return 1;
        }

        fprintf(file, "%s\n", buffer);  // Write the reversed string
        fclose(file);
    }

    return 0;
}
