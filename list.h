#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct list_t list_t;
typedef struct iterator_t iterator_t;

list_t* new_list();
void add(list_t* list, const char* path, size_t size);
void free_list(list_t* list);

iterator_t* list_iterator(list_t* list);
int iterator_has_next(iterator_t* iterator);
char* iterator_next(iterator_t* iterator);
void free_iterator(iterator_t* iterator);

#endif /* LIST_H */
