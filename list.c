// list.c
#include "list.h"
#include <stdlib.h>
#include <string.h>

typedef struct node_t {
    char* path;
    size_t size;
    struct node_t* next;
} node_t;

struct list_t {
    node_t* head;
};

struct iterator_t {
    node_t* current;
};

list_t* new_list() {
    list_t* list = (list_t*)malloc(sizeof(list_t));
    list->head = NULL;
    return list;
}

void add(list_t* list, const char* path, size_t size) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->path = strdup(path);
    new_node->size = size;
    new_node->next = NULL;

    node_t** current = &(list->head);
    while (*current != NULL && (*current)->size < size) {
        current = &((*current)->next);
    }
    new_node->next = *current;
    *current = new_node;
}

void free_list(list_t* list) {
    node_t* current = list->head;
    while (current != NULL) {
        node_t* temp = current;
        current = current->next;
        free(temp->path);
        free(temp);
    }
    free(list);
}

iterator_t* list_iterator(list_t* list) {
    iterator_t* iterator = (iterator_t*)malloc(sizeof(iterator_t));
    iterator->current = list->head;
    return iterator;
}

int iterator_has_next(iterator_t* iterator) {
    return iterator->current != NULL;
}

char * iterator_next(iterator_t* iterator) {
    if (!iterator_has_next(iterator))
        return NULL;
    char *to_return = iterator->current->path;
    iterator->current = iterator->current->next;
    return to_return;
}

void free_iterator(iterator_t* iterator) {
    free(iterator);
}
