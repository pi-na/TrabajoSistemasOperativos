#ifndef __list
#define __list
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct list_cdt * list_adt;

void free_list(list_adt list);
void add(list_adt list, char * path, size_t size_mb);
list_adt new_list(int (*compare)(size_t, size_t));
void to_begin(list_adt list);
int has_next(list_adt list);
char * next(list_adt list);


#endif // __list