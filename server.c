#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "structs.h"

Client clients[MAX_CLIENTS];
Message messages[MAX_MESSAGES];
int client_count = 0;
int message_count = 0;

void add_client(const char *login, const char *pipe_name)
{
    if (client_count < MAX_CLIENTS)
    {
        strcpy(clients[client_count].login, login);
        strcpy(clients[client_count].pipe_name, pipe_name);
        clients[client_count].pipe_fd = open(pipe_name, O_WRONLY);
        if (clients[client_count].pipe_fd == -1)
        {
            perror("Error opening pipe");
            return;
        }
        client_count++;
    }
    else
    {
        printf("Max clients reached. Can't add client\n");
    }
}

void remove_client(const char *login)
{
    for (int i = 0; i < client_count; i++)
    {
        if (strcmp(clients[i].login, login) == 0)
        {
            close(clients[i].pipe_fd);
            clients[i] = clients[client_count - 1];
            client_count--;
            return;
        }
    }
}

int find_client(const char *login)
{
    for (int i = 0; i < client_count; i++)
    {
        if (strcmp(clients[i].login, login) == 0)
        {
            return i;
        }
    }
    return -1;
}

void broadcast_message(const char *sender, const char *receiver, const char *message)
{
    int index = find_client(receiver);
    if (index != -1)
    {
        char buffer[MAX_MSG_LEN];
        snprintf(buffer, MAX_MSG_LEN, "From %s: %s\n", sender, message);
        write(clients[index].pipe_fd, buffer, strlen(buffer));
        printf("Message sent from '%s' to '%s'.\n", sender, receiver);
    }
    else
    {
        printf("Receiver '%s' not found.\n", receiver);
    }
}

void store_message(const char *sender, const char *receiver, const char *message)
{
    if (message_count < MAX_MESSAGES)
    {
        strcpy(messages[message_count].sender, sender);
        strcpy(messages[message_count].receiver, receiver);
        strncpy(messages[message_count].message, message, MAX_MSG_LEN);
        message_count++;
    }
    else
    {
        printf("Message history is full. Can't store message\n");
    }
}

void handle_search_request(const char *login, int client_fd)
{
    char result[MAX_MSG_LEN];
    write(client_fd, "-----HISTORY-----\n", strlen("-----HISTORY------\n"));
    for (int i = 0; i < message_count; i++)
    {
        if (strcmp(messages[i].receiver, login) == 0 || strcmp(messages[i].sender, login) == 0)
        {
            snprintf(result, MAX_HISTORY_LEN, "[%s -> %s]: %s\n", messages[i].sender, messages[i].receiver, messages[i].message);
            write(client_fd, result, strlen(result));
        }
    }
    write(client_fd, "-----HISTORY-----\n", strlen("-----HISTORY------\n"));
}

int main()
{
    if (mkfifo(SERVER_PIPE, 0666) == -1 && errno != EEXIST)
    {
        perror("Error creating server pipe");
        exit(EXIT_FAILURE);
    }

    int server_fd = open(SERVER_PIPE, O_RDONLY);
    if (server_fd == -1)
    {
        perror("Error opening server pipe");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for clients...\n");

    char buffer[MAX_MSG_LEN];
    while (1)
    {
        memset(buffer, 0, MAX_MSG_LEN);
        ssize_t bytes_read = read(server_fd, buffer, MAX_MSG_LEN);

        if (bytes_read > 0)
        {
            // Parse command: "LOGIN:login:pipe_name", "SEND:sender:receiver:message", or "SEARCH:login"
            char *command = strtok(buffer, ":");

            if (strcmp(command, "LOGIN") == 0)
            {
                char *login = strtok(NULL, ":");
                char *pipe_name = strtok(NULL, ":");
                add_client(login, pipe_name);
                printf("Client '%s' connected.\n", login);
            }
            else if (strcmp(command, "SEND") == 0)
            {
                char *sender = strtok(NULL, ":");
                char *receiver = strtok(NULL, ":");
                char *message = strtok(NULL, "\n");
                broadcast_message(sender, receiver, message);
                store_message(sender, receiver, message);
            }
            else if (strcmp(command, "SEARCH") == 0)
            {
                char *login = strtok(NULL, ":");
                int client_fd = find_client(login) != -1 ? clients[find_client(login)].pipe_fd : -1;
                if (client_fd != -1)
                {
                    handle_search_request(login, client_fd);
                }
            }
            else if (strcmp(command, "LOGOUT") == 0)
            {
                char *login = strtok(NULL, ":");
                remove_client(login);
                printf("Client '%s' disconnected.\n", login);
            }
        }
    }

    close(server_fd);
    unlink(SERVER_PIPE);
    return 0;
}