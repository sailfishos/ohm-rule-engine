#ifndef __PROLOG_H__
#define __PROLOG_H__

#include <SWI-Stream.h>
#include <SWI-Prolog.h>

#undef ALLOC
#undef ALLOC_OBJ
#undef ALLOC_ARR
#undef REALLOC_ARR
#undef STRDUP
#undef FREE

#define ALLOC(type) ({                            \
            type   *__ptr;                        \
            size_t  __size = sizeof(type);        \
                                                  \
            if ((__ptr = malloc(__size)) != NULL) \
                memset(__ptr, 0, __size);         \
            __ptr; })

#define ALLOC_OBJ(ptr) ((ptr) = ALLOC(typeof(*ptr)))

#define ALLOC_ARR(type, n) ({                     \
            type   *__ptr;                        \
            size_t   __size = (n) * sizeof(type); \
                                                  \
            if ((__ptr = malloc(__size)) != NULL) \
                memset(__ptr, 0, __size);         \
            __ptr; })

#define REALLOC_ARR(ptr, o, n) ({                                       \
            typeof(ptr) __ptr;                                          \
            size_t      __size = sizeof(*ptr) * (n);                    \
                                                                        \
            if ((ptr) == NULL) {                                        \
                (__ptr) = ALLOC_ARR(typeof(*ptr), n);                   \
                ptr = __ptr;                                            \
            }                                                           \
            else if ((__ptr = realloc(ptr, __size)) != NULL) {          \
                if ((n) > (o))                                          \
                    memset(__ptr + (o), 0, ((n)-(o)) * sizeof(*ptr));   \
                ptr = __ptr;                                            \
            }                                                           \
            __ptr; })
                
#define FREE(obj) do { if (obj) free(obj); } while (0)

#define STRDUP(s) ({                                    \
            char *__s = (char*) s;                      \
            __s = ((s) ? strdup(s) : strdup(""));       \
            __s; })

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
} prolog_predicate_t;



int  prolog_init(char *argv0, int, int, int, int, char *bootfile);
void prolog_exit(void);

int  prolog_load_extension(char *path);
int  prolog_load_file     (char *path);

prolog_predicate_t *prolog_predicates(char *query);
prolog_predicate_t *prolog_undefined (void);

int prolog_predicate(prolog_predicate_t *p, char *name, int arity);
int prolog_rules    (prolog_predicate_t **rules, prolog_predicate_t **undef);

int                 prolog_call      (prolog_predicate_t *p, void *ret, ...);
int                 prolog_acall     (prolog_predicate_t *p, void *retval,
                                      void **args, int narg);
int                 prolog_vcall     (prolog_predicate_t *p, void *ret,
                                      va_list ap);
#define             prolog_callarr prolog_acall

int                 prolog_trace_set(char *commands);
void                prolog_trace_show(char *predicate);


void                prolog_free_predicates(prolog_predicate_t *predicates);

void prolog_free_results(char ***results);
void prolog_dump_results(char ***results);

void prolog_free_exception(char ***exception);
void prolog_dump_exception(char ***exception);

void prolog_free_actions(char ***actions);
void prolog_dump_actions(char ***actions);

char *prolog_flatten_actions(char ***actions);

void prolog_free_objects(char ***objects);
void prolog_dump_objects(char ***objects);



int  prolog_list_length(term_t pl_list);
int  prolog_walk_list(term_t list,
                      int (*callback)(term_t item, int i, void *data),
                      void *data);

term_t prolog_list_new(char **items, int n, term_t result);


int prolog_shell(int in);


#endif /* __PROLOG_H__ */


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/

