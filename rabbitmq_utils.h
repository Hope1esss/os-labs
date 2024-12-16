#ifndef RABBITMQ_UTILS_H
#define RABBITMQ_UTILS_H

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <stdlib.h>
#include <stdio.h>

amqp_connection_state_t connect_rabbitmq(const char *hostname, int port)
{
    amqp_connection_state_t connection = amqp_new_connection();
    amqp_socket_t *socket = amqp_tcp_socket_new(connection);

    if (!socket)
    {
        fprintf(stderr, "Error: Unable to create TCP socket\n");
        exit(1);
    }

    int status = amqp_socket_open(socket, hostname, port);
    if (status != 0)
    {
        fprintf(stderr, "Error: Unable to open TCP socket: %s\n", amqp_error_string2(status));
        exit(1);
    }

    amqp_login(connection, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    amqp_channel_open(connection, 1);
    amqp_get_rpc_reply(connection);
    return connection;
}

void close_rabbitmq(amqp_connection_state_t connection)
{
    amqp_channel_close(connection, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(connection, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(connection);
}

#endif