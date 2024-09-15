#ifndef __stack
#define __stack


typedef struct stack_cdt * stack_adt;

void free_stack(stack_adt stack);
stack_adt pop(stack_adt stack, stack_type * to_ret);
stack_adt add(stack_adt stack, char * path, size_t size_mb);
static stack_type recursive_add(stack_type stack, char * path, size_t size_mb, int (*compare)(size_t, size_t));
stack_adt new_stack(int (*compare)(size_t, size_t));


#endif // __stack