#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "structs.h"

void connect_to_server(const char *login)
{
    int server_fd = open(SERVER_PIPE, O_WRONLY);
    if (server_fd == -1)
    {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_MSG_LEN];
    snprintf(buffer, MAX_MSG_LEN, "LOGIN:%s:/tmp/%s_pipe", login, login);
    write(server_fd, buffer, strlen(buffer));
    close(server_fd);
}

void disconnect_from_server(const char *login)
{
    int server_fd = open(SERVER_PIPE, O_WRONLY);
    if (server_fd == -1)
    {
        perror("Error disconnecting from server");
        return;
    }

    char buffer[MAX_MSG_LEN];
    snprintf(buffer, MAX_MSG_LEN, "LOGOUT:%s", login);
    write(server_fd, buffer, strlen(buffer));
    close(server_fd);
}

void send_message(const char *login, const char *receiver, const char *message)
{
    int server_fd = open(SERVER_PIPE, O_WRONLY);
    if (server_fd == -1)
    {
        perror("Error sending message");
        return;
    }

    char buffer[MAX_MSG_LEN];
    snprintf(buffer, MAX_MSG_LEN, "SEND:%s:%s:%s", login, receiver, message);
    write(server_fd, buffer, strlen(buffer));
    close(server_fd);
}

void search_history(const char *login)
{
    int server_fd = open(SERVER_PIPE, O_WRONLY);
    if (server_fd == -1)
    {
        perror("Error sending search request to server");
        return;
    }

    char buffer[MAX_MSG_LEN];
    snprintf(buffer, MAX_MSG_LEN, "SEARCH:%s", login);
    write(server_fd, buffer, strlen(buffer));
    close(server_fd);
}

void receive_messages(const char *pipe_name)
{
    int client_fd = open(pipe_name, O_RDONLY);
    if (client_fd == -1)
    {
        perror("Error opening client pipe");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_MSG_LEN];
    while (1)
    {
        memset(buffer, 0, MAX_MSG_LEN);
        ssize_t bytes_read = read(client_fd, buffer, MAX_MSG_LEN - 1);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            if (strcmp(buffer, "SERVER_CLOSED\n") == 0)
            {
                printf("Server has been closed. Disconnecting.\n");
                close(client_fd);
                exit(0);
            }
            printf("%s", buffer);
            printf("%s", "> ");
            fflush(stdout);
        }
        else if (bytes_read == 0)
        {
            printf("Disconnected from server. Exiting.\n");
            close(client_fd);
            exit(0);
        }
    }

    close(client_fd);
}

int main() {
    char login[50];
    char receiver[50];
    char message[MAX_MSG_LEN];

    printf("Enter your login: ");
    scanf("%49s", login);

    connect_to_server(login);

    char client_pipe[100];
    snprintf(client_pipe, sizeof(client_pipe), "/tmp/%s_pipe", login);
    if (mkfifo(client_pipe, 0666) == -1 && errno != EEXIST) {
        perror("Error creating client pipe");
        exit(EXIT_FAILURE);
    }

    printf("Connected as '%s'. Waiting for messages...\n", login);

    if (fork() == 0) {
        receive_messages(client_pipe);
        exit(0);
    }

    while (1) {
        printf("> ");
        char command[10];
        scanf("%9s", command);

        if (strcmp(command, "send") == 0) {
            printf("Enter receiver: ");
            scanf("%49s", receiver);

            printf("Enter your message: ");
            getchar();
            fgets(message, MAX_MSG_LEN, stdin);
            message[strcspn(message, "\n")] = 0;

            send_message(login, receiver, message);
        } else if (strcmp(command, "search") == 0) {
            search_history(login);
        } else if (strcmp(command, "logout") == 0) {
            disconnect_from_server(login);
            break;
        } else {
            printf("Unknown command. Use 'send', 'search', or 'logout'.\n");
        }
    }

    unlink(client_pipe);
    return 0;
}