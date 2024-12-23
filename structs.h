#ifndef STRUCTS_H
#define STRUCTS_H

#define MAX_CLIENTS 256
#define MAX_MESSAGES 2048
#define MAX_HISTORY_LEN 4096
#define MAX_MSG_LEN 2048
#define SERVER_PIPE "/tmp/server_pipe"
typedef struct
{
    char login[256];
    char pipe_name[256];
    int pipe_fd;
} Client;

typedef struct
{
    char sender[256];
    char receiver[256];
    char message[MAX_MSG_LEN];
} Message;

#endif