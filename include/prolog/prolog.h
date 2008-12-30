#ifndef __PROLOG_H__
#define __PROLOG_H__

#include <sys/time.h>

#include <SWI-Stream.h>
#include <SWI-Prolog.h>

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
    char           *module;                  /* module name */
    char           *name;                    /* exported predicate name */
    int             arity;                   /* exported predicate arity */
    predicate_t     predicate;               /* prolog predicate handle */
    /* runtime statistics */
    struct timeval  usr;                     /* time spent in user space */
    struct timeval  sys;                     /* time spent in kernel space */
    int             calls;                   /* number of invocations */
} prolog_predicate_t;



int  prolog_init(char *argv0, int, int, int, int, char *bootfile);
void prolog_exit(void);

int  prolog_load_extension(char *path);
int  prolog_load_file     (char *path);

prolog_predicate_t *prolog_predicates(char *query);
prolog_predicate_t *prolog_undefined (void);

int prolog_predicate (prolog_predicate_t *p, char *name, int arity);
int prolog_rules     (prolog_predicate_t **rules, prolog_predicate_t **undef);
int prolog_statistics(prolog_predicate_t *pred,
                      int *invocations, double *sys, double *usr, double *avg);


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

