#ifndef TREE_H
#define TREE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_NODES 1000

typedef struct Node
{
    int id;
    int pid;
    struct Node *parent;
    struct Node *children[100];
    int child_count;
    bool is_available;
} Node;

Node *node_table[MAX_NODES];

void init_tree()
{
    for (int i = 0; i < MAX_NODES; i++)
    {
        node_table[i] = NULL;
    }
}

Node *create_node(int id, int pid, Node *parent)
{
    if (node_table[id] != NULL)
    {
        return NULL;
    }

    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->id = id;
    new_node->pid = pid;
    new_node->parent = parent;
    new_node->child_count = 0;
    new_node->is_available = true;

    if (parent != NULL)
    {
        parent->children[parent->child_count++] = new_node;
    }

    node_table[id] = new_node;
    return new_node;
}

Node *find_node(int id)
{
    return (id >= 0 && id < MAX_NODES) ? node_table[id] : NULL;
}

void update_availability(int id, bool available)
{
    Node *node = find_node(id);
    if (node)
    {
        node->is_available = available;
    }
}

#endif