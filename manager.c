#include "rabbitmq_utils.h"
#include "tree.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define QUEUE_NAME "task_queue"

void launch_worker(int id)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        char id_str[10];
        snprintf(id_str, sizeof(id_str), "%d", id);
        execl("./worker", "worker", id_str, NULL);
        perror("Error: failed to launch worker");
        exit(1);
    }
    else if (pid > 0)
    {
        Node *node = find_node(id);
        if (node)
        {
            node->pid = pid;
            printf("Ok: Worker %d launched with pid %d \n", id, pid);
        }
    }
    else
    {
        perror("Error: failed to fork");
    }
}

void handle_create_command(const char *command)
{
    int id, pid;
    if (sscanf(command, "create %d %d", &id, &pid) < 2)
    {
        printf("Error: Invalid create command\n");
        return;
    }

    Node *parent_node = find_node(pid);
    if (pid != -1 && parent_node == NULL)
    {
        printf("Error: Parent not found\n");
        return;
    }

    if (pid != -1 && !parent_node->is_available)
    {
        printf("Error: Parent is unavailable\n");
        return;
    }

    Node *new_node = create_node(id, -1, parent_node);
    if (new_node == NULL)
    {
        printf("Error: Already exists\n");
        return;
    }

    launch_worker(id);
}

void handle_exec_command(amqp_connection_state_t connection, const char *command)
{
    int id;
    char params[256];
    if (sscanf(command, "exec %d %[^\n]", &id, params) < 2)
    {
        printf("Error:%d: Invalid exec command\n", id);
        return;
    }

    Node *node = find_node(id);
    if (!node)
    {
        printf("Error:%d: Node not found\n", id);
        return;
    }

    if (!node->is_available)
    {
        printf("Error:%d: Node is unavailable\n", id);
        return;
    }

    char queue_name[256];
    snprintf(queue_name, sizeof(queue_name), "node_%d", id);

    amqp_bytes_t queue = amqp_cstring_bytes(queue_name);
    amqp_bytes_t body = amqp_cstring_bytes(params);

    amqp_basic_publish(connection, 1, amqp_empty_bytes, queue, 0, 0, NULL, body);
}

void handle_pingall_command()
{
    printf("Checking availability of all nodes...\n");
    bool all_available = true;

    for (int i = 0; i < MAX_NODES; i++)
    {
        if (node_table[i] != NULL && !node_table[i]->is_available)
        {
            printf("Node %d is unavailable\n", i);
            all_available = false;
        }
    }

    if (all_available)
    {
        printf("Ok: -1\n");
    }
}

void handle_list_command()
{
    printf("Listing all nodes:\n");
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (node_table[i] != NULL)
        {
            printf("Node %d, Parent: %d, PID: %d, Available: %s\n",
                   node_table[i]->id,
                   node_table[i]->parent ? node_table[i]->parent->id : -1,
                   node_table[i]->pid,
                   node_table[i]->is_available ? "Yes" : "No");
        }
    }
}

int main()
{
    amqp_connection_state_t conn = connect_rabbitmq("localhost", 5672);

    // Инициализация дерева
    init_tree();

    char command[256];
    while (1)
    {
        printf("> ");
        fgets(command, 256, stdin);
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0)
        {
            break;
        }
        else if (strncmp(command, "create", 6) == 0)
        {
            handle_create_command(command);
        }
        else if (strncmp(command, "exec", 4) == 0)
        {
            handle_exec_command(conn, command);
        }
        else if (strcmp(command, "pingall") == 0)
        {
            handle_pingall_command();
        }
        else if (strcmp(command, "list") == 0)
        {
            handle_list_command();
        }
        else
        {
            printf("Error: Unknown command\n");
        }
    }

    close_rabbitmq(conn);
    return 0;
}