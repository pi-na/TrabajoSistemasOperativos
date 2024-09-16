#include "list.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct node * list_type;

typedef struct node{
    char * path;
    size_t size_mb;
    struct node * tail;
} node_type;

typedef struct list_cdt{
    list_type first;
    list_type iterator;
    int (*compare)(size_t, size_t);
} list_cdt;

list_adt new_list(int (*compare)(size_t, size_t)) {
    list_adt aux = calloc(1, sizeof(list_cdt));
    if (aux == NULL) {
        fprintf(stderr, "Error, no se pudo asignar memoria para el list.\n");
        return NULL;
    }
    aux->compare = compare;
    return aux;
}

static list_type recursive_add(list_type list, char * path, size_t size_mb, int (*compare)(size_t, size_t)){
    if (list == NULL || compare(list->size_mb, size_mb) >= 0){
        list_type aux = malloc(sizeof(node_type));
        if (aux == NULL) {
            fprintf(stderr, "Error, no se pudo asignar memoria para el nuevo list.\n");
            return NULL;
        }
        aux->path = path;
        aux->size_mb = size_mb;
        aux->tail = list;
        return aux;
    }

    if (compare(list->size_mb, size_mb) < 0){
        list->tail = recursive_add(list->tail, path, size_mb, compare);
    }

    return list;
}

void add(list_adt list, char * path, size_t size_mb){
    list_type aux = calloc(1, sizeof(node_type));
    if (aux == NULL) {
        fprintf(stderr, "Errorrrr, no se pudo asignar memoriaaa\n");
        return;
    }
    aux->path = path;
    
    list->first = recursive_add(list->first, path, size_mb, list->compare);

}

void to_begin(list_adt list){
    list->iterator = list->first;
}


int has_next(list_adt list){
   return list->iterator != NULL;
}

char * next(list_adt list){
    
    if (has_next(list)){
        char * path = list->iterator->path;
        list->iterator = list->iterator->tail;
        return path;
    }else {
        fprintf(stderr, "No such path!\nAborting...");
        exit(1);
    }
}

void free_list(list_adt list){
    list_type actual = list->first;
    list_type next;

    while(actual != NULL){
        next = actual->tail;
        free(actual);
        actual = next;
    }

    free(list);
}