#ifndef PROLOG_H_INCLUDED
#define PROLOG_H_INCLUDED

#include <SWI-Prolog.h>

#define ALLOC_ARRAY(type, n) ({                 \
        type   *__ptr;                          \
        size_t  __size = sizeof(type) * (n);    \
        if ((__ptr = malloc(__size)) != NULL)   \
            memset(__ptr, 0, __size);           \
        __ptr; })

typedef struct {
    char  *argv0;
    char  *ruleset;
    char **extensions;
    int    local_stack;
    int    global_stack;
    int    tail_stack;
    int    argument_stack;
} prolog_config_t;


typedef struct {
    char        *module;                     /* module name */
    char        *name;                       /* exported predicate name */
    int          arity;                      /* exported predicate arity */
    predicate_t  predicate;                  /* prolog predicate handle */
} prolog_export_t;



int  prolog_init(char *argv0, int, int, int, int);
void prolog_exit(void);

int  prolog_load_extension(char *path);
int  prolog_load_file     (char *path);

prolog_export_t *prolog_get_exported(char *query);


int  prolog_query_once(char *predicate, int arity, term_t pl_arguments,
                       char *module);
int  prolog_list_length(term_t pl_list);
int  prolog_walk_list(term_t list,
                      int (*callback)(term_t item, int i, void *data),
                      void *data);

term_t prolog_list_new(char **items, int n, term_t result);


void   prolog_prompt(void);


#if 0
void    policy_free_actions(char ***actions);
void    policy_dump_actions(char ***actions);
#endif



#endif /* PROLOG_H_INCLUDED */


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/

