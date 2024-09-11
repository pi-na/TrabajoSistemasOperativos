#include "list.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct node * TList;

typedef struct node{
    char * path;
    size_t size_mb;
    struct node * tail;
}TNode;

typedef struct listCDT{
    TList first;
    TList iterator;
}listCDT;


listADT toBegin(listADT list ){
    
    list->iterator = list->first;
    
}

listADT newList() {
    listADT aux = calloc(1, sizeof(listCDT));
    if (aux == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para la nueva lista.\n");
        return NULL;
    }
    return aux;
}

listADT add(listADT list, char * path, size_t size_mb){

}


