#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct node * stack_type;

typedef struct node{
    char * path;
    size_t size_mb;
    struct node * tail;
}node_type;

typedef struct stack_cdt{
    stack_type first;
    stack_type iterator;
    int (*compare)(size_t, size_t)
}stack_cdt;

void to_begin(stack_adt stack){
    stack->iterator = s
}


stack_adt new_stack(int (*compare)(size_t, size_t)) {
    stack_adt aux = calloc(1, sizeof(stack_cdt));
    if (aux == NULL) {
        fprintf(stderr, "Error, no se pudo asignar memoria para el stack.\n");
        return NULL;
    }
    aux->compare = compare;
    return aux;
}

static stack_type recursive_add(stack_type stack, char * path, size_t size_mb, int (*compare)(size_t, size_t)){
    if (stack == NULL || compare(stack->size_mb, size_mb) >= 0){
        stack_type aux = malloc(sizeof(node_type));
        if (aux == NULL) {
            fprintf(stderr, "Error, no se pudo asignar memoria para el nuevo stack.\n");
            return NULL;
        }
        aux->path = path;
        aux->size_mb = size_mb;
        aux->tail = stack;
        return aux;
    }

    if (compare(stack->size_mb, size_mb) < 0){
        stack->tail = recursive_add(stack->tail, path, size_mb, compare);
    }

    return stack;
}

stack_adt add(stack_adt stack, char * path, size_t size_mb){
    stack_type aux = calloc(1, sizeof(node_type));
    if (aux == NULL) {
        fprintf(stderr, "Errorrrr, no se pudo asignar memoriaaa\n");
        return NULL;
    }
    aux->path = path;
    
    stack->first = recursive_add(stack->first, path, size_mb, stack->compare);
}

stack_adt pop(stack_adt stack, char * to_ret) {
    if (stack == NULL || stack->first == NULL) {
        return NULL;
    }
    stack_type aux = stack->first;
    *to_ret = aux->path;
    stack->first = stack->first->tail;
    free(aux);
    return stack;
}

void free_stack(stack_adt stack){
    stack_type actual = stack->first;
    stack_type next;

    while(actual != NULL){
        next = actual->tail;
        free(actual);
        actual = next;
    }

    free(stack);
}