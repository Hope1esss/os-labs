#include "rabbitmq_utils.h"
#include "tree.h"
#include <string.h>

#define MAX_DICT_ENTRIES 100

typedef struct {
    char key[50];
    int value;
} DictEntry;

static DictEntry dictionary[MAX_DICT_ENTRIES];
static int dict_size = 0;

void process_command(int node_id, const char *command, char *response) {
    char name[50];
    int value;

    if (sscanf(command, "%s %d", name, &value) == 2) {
        bool found = false;
        for (int i = 0; i < dict_size; i++) {
            if (strcmp(dictionary[i].key, name) == 0) {
                dictionary[i].value = value;
                found = true;
                break;
            }
        }
        if (!found && dict_size < MAX_DICT_ENTRIES) {
            strcpy(dictionary[dict_size].key, name);
            dictionary[dict_size].value = value;
            dict_size++;
        }
        sprintf(response, "Ok:%d", node_id);
    } else if (sscanf(command, "%s", name) == 1) {
        for (int i = 0; i < dict_size; i++) {
            if (strcmp(dictionary[i].key, name) == 0) {
                sprintf(response, "Ok:%d: %d", node_id, dictionary[i].value);
                return;
            }
        }
        sprintf(response, "Ok:%d: '%s' not found", node_id, name);
    } else {
        sprintf(response, "Error:%d: Invalid command", node_id);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: Node ID is required\n");
        return 1;
    }

    int node_id = atoi(argv[1]);
    char queue_name[50];
    snprintf(queue_name, sizeof(queue_name), "node_%d", node_id);

    amqp_connection_state_t conn = connect_rabbitmq("localhost", 5672);
    amqp_queue_declare(conn, 1, amqp_cstring_bytes(queue_name), 0, 0, 0, 1, amqp_empty_table);
    amqp_basic_consume(conn, 1, amqp_cstring_bytes(queue_name), amqp_empty_bytes, 0, 1, 0, amqp_empty_table);


    while (1) {
        amqp_rpc_reply_t res;
        amqp_envelope_t envelope;

        amqp_maybe_release_buffers(conn);
        res = amqp_consume_message(conn, &envelope, NULL, 0);

        if (res.reply_type == AMQP_RESPONSE_NORMAL) {
            char response[256];
            process_command(node_id, (char *)envelope.message.body.bytes, response);
            printf("%s\n", response);
        }
        amqp_destroy_envelope(&envelope);
    }

    close_rabbitmq(conn);
    return 0;
}
