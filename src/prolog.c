#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <prolog/prolog.h>
#include <SWI-Stream.h>
#include <SWI-Prolog.h>

#ifndef PATH_MAX
#  define PATH_MAX 256
#endif

#define ALLOC_ARRAY ALLOC_ARR
#define LIBPROLOG     "libprolog.so"
#define PRED_EXPORTED "exported"
#define OBJECT_NAME   "name"

#define QUERY_FLAGS (PL_Q_NORMAL | PL_Q_NODEBUG | PL_Q_CATCH_EXCEPTION)


enum {
    NATIVE  = 0,
    FOREIGN = 1,
};

enum {
    RESULT_UNKNOWN = 0,
    RESULT_ACTIONS,
    RESULT_OBJECTS,
    RESULT_EXCEPTION,
};


static int   load_file(char *path, int foreign);
static char *shlib_path(char *lib, char *buf, size_t size);
static int   collect_predicates(term_t pl_descriptor, int i, void *data);
static int   collect_result    (qid_t qid, term_t pl_retval, void *retval);
static int   collect_actions   (term_t item, int i, void *data);
static int   collect_objects   (term_t item, int i, void *data);
static int   collect_exception (qid_t qid, void *retval);


static char libpl[PATH_MAX];
static char lstack[16], gstack[16], tstack[16], astack[16];

static char *pl_argv[] = {
    NULL,                                      /* argv[0] */
    NULL,                                      /* -x */
    NULL,                                      /* bootfile or argv[0] */
    "-q",                                      /* no startup banners */
    "-nosignals",                              /* no prolog signal handling */
    "-tty",                                    /* no controlling TTY */
#define NUM_FIXED_ARGS 3
    "-L16k",                                   /* local stack size */
    "-G16k",                                   /* global stack size */
    "-T16k",                                   /* trail stack size */
    "-A16k",                                   /* argument stack size */
};


/*****************************************************************************
 *                      *** initialization & cleanup ***                     *
 *****************************************************************************/

/*************************
 * prolog_init
 *************************/
int
prolog_init(char *argv0,
            int lsize, int gsize, int tsize, int asize, char *bootfile)
{
    char **argv;
    int    argc, status;

    /*
     * Notes:
     *
     * PL_initialise wants to know the path to the binary that has libpl.a
     * linked into. It tries to load the binary as a prolog resource file.
     * If this fails it will practically skip parsing most of the command
     * line, including our options to silence output and limit the size of
     * the various stacks.
     *
     * In our case, libpl.a is linked to libprolog.so and not to the main
     * binary. Hence we first need to find the path to libprolog.so and then
     * pass this in as argv[0] to PL_initialise. Ugly, I know... but necessary
     * unless we link statically.
     *
     *
     * Notes #2:
     *
     * This is unarguably a linux-specific hack. An alternative would be
     * a GLIBC-specific hack using dl_iterate_phdr(3) to iterate directly
     * through the list of shared objects loaded into us. This would have
     * the benefit of not needing to open and parse /proc entries.
     *
     * Notes #3:
     *
     * Stand-alone precompiled prolog files include a copy of the interpreter
     * itself. Using one could be an alternative to avoid this problem
     * altogether.
     */
    
    putenv("SWI_HOME_DIR="PROLOG_HOME);    /* hmm... is this needed now ? */

    snprintf(lstack, sizeof(lstack), "-L%dk", lsize ?: 16);
    snprintf(gstack, sizeof(gstack), "-G%dk", gsize ?: 16);
    snprintf(tstack, sizeof(tstack), "-T%dk", tsize ?: 16);
    snprintf(astack, sizeof(astack), "-A%dk", asize ?: 16);

    if (bootfile != NULL)
        argv = pl_argv;
    else
        argv = pl_argv + 2;                   /* skip { "-x", bootfile } */
    
    argv[argc=0] = shlib_path(LIBPROLOG, libpl, sizeof(libpl));
    
    if (bootfile != NULL) {
        pl_argv[++argc] = "-x";               /* must be argv[1] */
        pl_argv[++argc] = bootfile;           /*     and argv[2] */
    }
    
    argc += NUM_FIXED_ARGS;
    pl_argv[++argc] = lstack;
    pl_argv[++argc] = gstack;
    pl_argv[++argc] = tstack;
    pl_argv[++argc] = astack;

#if 0
    {
        int i;
        for (i = 0; i <= argc; i++)
            printf("*** prolog argv[%d]: \"%s\"\n", i, argv[i]);
    }
#endif

    if ((status = !PL_initialise(argc + 1, argv)))
        PL_cleanup(0);
    
    return status;

    (void)argv0;
}


/*************************
 * prolog_exit
 *************************/
void
prolog_exit(void)
{
    if (PL_is_initialised(NULL, NULL))
        PL_cleanup(0);
}


/*****************************************************************************
 *                    *** ruleset & extension loading ***                    *
 *****************************************************************************/

/*************************
 * prolog_load_file
 *************************/
int
prolog_load_file(char *path)
{
    return load_file(path, NATIVE);
}


/*************************
 * prolog_load_extension
 *************************/
int
prolog_load_extension(char *path)
{
    return load_file(path, FOREIGN);
}


/*************************
 * load_file
 *************************/
static int
load_file(char *path, int foreign)
{
    char        *loader = foreign ? "load_foreign_library" : "consult";
    fid_t        frame;
    predicate_t  pr_load;
    term_t       pl_path;
    int          status;

    if (!(pr_load = PL_predicate(loader, 1, NULL)))
        return ENOSYS;

    frame = PL_open_foreign_frame();

    pl_path = PL_new_term_ref();
    PL_put_atom_chars(pl_path, path);
    status = PL_call_predicate(NULL, PL_Q_NORMAL, pr_load, pl_path) ? 0 : EIO;
    
    PL_discard_foreign_frame(frame);

    return status;
}


/*****************************************************************************
 *                        *** predicate handling ***                         *
 *****************************************************************************/


/********************
 * prolog_predicates
 ********************/
prolog_predicate_t *
prolog_predicates(char *query)
{
    char        *exported   = query ? query : PRED_EXPORTED;
    predicate_t pr_exported = PL_predicate(exported, 1, NULL);
    fid_t       frame;
    term_t      pl_predlist = PL_new_term_ref();

    prolog_predicate_t *predicates;
    int                 npredicate;

    frame = PL_open_foreign_frame();

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_exported, pl_predlist))
        goto fail;
    
    if ((npredicate = prolog_list_length(pl_predlist)) <= 0)
        goto fail;

    if ((predicates = ALLOC_ARRAY(prolog_predicate_t, npredicate + 1)) == NULL)
        goto fail;
    
    if (prolog_walk_list(pl_predlist, collect_predicates, predicates))
        goto fail;
    
    PL_discard_foreign_frame(frame);

    return predicates;

 fail:
    PL_discard_foreign_frame(frame);
    return NULL;
}


/********************
 * prolog_free_predicates
 ********************/
void
prolog_free_predicates(prolog_predicate_t *predicates)
{
    prolog_predicate_t *p;
    
    if (predicates == NULL)
        return;
    
    for (p = predicates; p->name != NULL; p++) {
        FREE(p->module);
        FREE(p->name);
    }

    FREE(predicates);
}


/********************
 * collect_predicates
 ********************/
static int
collect_predicates(term_t pl_descriptor, int i, void *data)
{
    prolog_predicate_t *predicate = ((prolog_predicate_t *)data) + i;
    const char         *name;
    int                 arity;
    predicate_t         pr_predicate;
    atom_t              slash_name;
    term_t              pl_module, pl_descr, pl_name, pl_arity;

    /*
     * Notes:
     *     Prolog represents the term bar/3 as /(bar, 3), ie. as
     *     the functor '/' with arity 2 and arguments 'bar' and '3'.
     *     Similarly the term foo:bar/3 is represented by the
     *     compound :(foo, /(bar, 3)).
     */
    
    if (!PL_get_name_arity(pl_descriptor, &slash_name, &arity))
        return EINVAL;
    
    name = PL_atom_chars(slash_name);
    
    if (name[1] != '\0')
        return EINVAL;

    if (name[0] == ':') {
        pl_module = PL_new_term_refs(2);
        pl_descr  = pl_module + 1;
        if (!PL_get_arg(1, pl_descriptor, pl_module) ||
            !PL_get_arg(2, pl_descriptor, pl_descr))
            return EINVAL;
    
        PL_get_chars(pl_module, (char **)&name, CVT_ALL);
        
        if ((predicate->module = STRDUP(name)) == NULL)
            return ENOMEM;
        
        pl_descriptor = pl_descr;
        if (!PL_get_name_arity(pl_descriptor, &slash_name, &arity))
            return EINVAL;
    }

    name = PL_atom_chars(slash_name);
    
    if ((name = PL_atom_chars(slash_name)) == NULL ||
        (name[0] != '/' || name[1] != '\0' || arity != 2))
        return EINVAL;

    pl_name  = PL_new_term_refs(2);
    pl_arity = pl_name + 1;
    
    if (!PL_get_arg(1, pl_descriptor, pl_name) ||
        !PL_get_arg(2, pl_descriptor, pl_arity))
        return EINVAL;
    
    PL_get_chars(pl_name, (char **)&name, CVT_ALL);
    PL_get_integer(pl_arity, &arity);
    
    pr_predicate = PL_predicate(name, arity, predicate->module);
    
    predicate->name      = strdup(name);
    predicate->arity     = arity;
    predicate->predicate = pr_predicate;

    if (predicate->name == NULL)
        return ENOMEM;
    
    return 0;
}


/*****************************************************************************
 *                      *** prolog predicate invocation ***                  *
 *****************************************************************************/

/********************
 * prolog_call
 ********************/
int
prolog_call(prolog_predicate_t *p, void *retval, ...)
{
    va_list  ap;
    fid_t    frame;
    qid_t    qid;
    term_t   pl_args, pl_retval;
    char    *arg;
    int      i, success;

    
    frame = PL_open_foreign_frame();
    
    pl_args   = PL_new_term_refs(p->arity);
    pl_retval = pl_args + p->arity - 1;

    va_start(ap, retval);
    for (i = 0; i < p->arity - 1; i++) {
        arg = va_arg(ap, char *);
        PL_put_atom_chars(pl_args + i, arg);
    }
    va_end(ap);

    qid     = PL_open_query(NULL, QUERY_FLAGS, p->predicate, pl_args);
    success = PL_next_solution(qid);
    if (collect_result(qid, pl_retval, retval) != 0)
        success = FALSE;
    PL_close_query(qid);

    PL_discard_foreign_frame(frame);
    
    return success;
} 


/********************
 * prolog_acall
 ********************/
int
prolog_acall(prolog_predicate_t *p, void *retval, void **args, int narg)
{
    fid_t  frame;
    qid_t  qid;
    term_t pl_args, pl_retval;
    int    i, success;

    if (narg < p->arity - 1)
        return FALSE;
    else if (narg > p->arity - 1) {
        fprintf(stderr, "WARNING: %s ignoring extra %d parameter%s to %s\n",
                __FUNCTION__,
                narg - p->arity - 1, narg - p->arity - 1 > 1 ? "s" : "",
                p->name);
    }
    
    frame = PL_open_foreign_frame();
    
    pl_args   = PL_new_term_refs(p->arity);
    pl_retval = pl_args + p->arity - 1;

    for (i = 0; i < p->arity - 1; i++)
        PL_put_atom_chars(pl_args + i, (char *)args[i]);
    
    qid     = PL_open_query(NULL, QUERY_FLAGS, p->predicate, pl_args);
    success = PL_next_solution(qid);
    if (collect_result(qid, pl_retval, retval) != 0)
        success = FALSE;
    PL_close_query(qid);
    
    PL_discard_foreign_frame(frame);
    
    return success;
} 


/********************
 * prolog_vcall
 ********************/
int
prolog_vcall(prolog_predicate_t *p, void *retval, va_list ap)
{
    fid_t    frame;
    qid_t    qid;
    term_t   pl_args, pl_retval;
    char    *arg;
    int      i, success;

    
    frame = PL_open_foreign_frame();
    
    pl_args   = PL_new_term_refs(p->arity);
    pl_retval = pl_args + p->arity - 1;

    for (i = 0; i < p->arity - 1; i++) {
        arg = va_arg(ap, char *);
        PL_put_atom_chars(pl_args + i, arg);
    }

    qid     = PL_open_query(NULL, QUERY_FLAGS, p->predicate, pl_args);
    success = PL_next_solution(qid);
    if (collect_result(qid, pl_retval, retval) != 0)
        success = FALSE;
    PL_close_query(qid);
    
    PL_discard_foreign_frame(frame);
    
    return success;
} 


/*****************************************************************************
 *                        *** action/object handling ***                     *
 *****************************************************************************/


/********************
 * prolog_free_results
 ********************/
void
prolog_free_results(char ***results)
{
    int tag;

    if (results == NULL)
        return;

    switch ((tag = (int)results[-1])) {
    case RESULT_ACTIONS:   prolog_free_actions(results);   break;
    case RESULT_OBJECTS:   prolog_free_objects(results);   break;
    case RESULT_EXCEPTION: prolog_free_exception(results); break;
    default:
        fprintf(stderr, "WARNING: %s called with invalid result type %d\n",
                __FUNCTION__, tag);
    }
}


/********************
 * prolog_dump_results
 ********************/
void
prolog_dump_results(char ***results)
{
    int tag;

    if (results == NULL)
        return;

    switch ((tag = (int)results[-1])) {
    case RESULT_ACTIONS:   prolog_dump_actions(results);   break;
    case RESULT_OBJECTS:   prolog_dump_objects(results);   break;
    case RESULT_EXCEPTION: prolog_dump_exception(results); break;
    default:
        fprintf(stderr, "WARNING: %s called with invalid result type %d\n",
                __FUNCTION__, tag);
    }
}


/********************
 * prolog_free_actions
 ********************/
void
prolog_free_actions(char ***actions)
{
    int a, p;

    if (actions == NULL)
        return;

    if (actions[-1] != (char **)RESULT_ACTIONS) {
        fprintf(stderr, "WARNING: %s called for invalid list (tag: 0x%x)",
                __FUNCTION__, (int)actions[-1]);
        return;
    }
    
    for (a = 0; actions[a]; a++) {
        for (p = 0; actions[a][p]; p++)
            free(actions[a][p]);
        free(actions[a]);
    }
    free(actions - 1);
}


/********************
 * prolog_dump_actions
 ********************/
void
prolog_dump_actions(char ***actions)
{
    int a, p;

    if (actions == NULL)
        return;

    if (actions[-1] != (char **)RESULT_ACTIONS) {
        fprintf(stderr, "WARNING: %s called for invalid list (tag: 0x%x)",
                __FUNCTION__, (int)actions[-1]);
        return;
    }
    
    for (a = 0; actions[a]; a++) {
        printf("(%s", actions[a][0]);
        for (p = 1; actions[a][p]; p++)
            printf(", %s", actions[a][p]);
        printf(")\n");
    }
}


/********************
 * prolog_flatten_actions
 ********************/
char *
prolog_flatten_actions(char ***actions)
{
#define CHUNK 128
#define NEED_SPACE(n) do {                                      \
        if (left < n) {                                         \
            p -= (int)flattened;                                \
            if (!REALLOC_ARR(flattened, size, size + CHUNK)) {  \
                errno = ENOMEM;                                 \
                free(flattened);                                \
                return NULL;                                    \
            }                                                   \
            p    += (int)flattened;                             \
            size += CHUNK;                                      \
            left += CHUNK;                                      \
        }                                                       \
    } while (0)

    char **action, *a, *flattened, *p, *t;
    int    i, j, n;
    unsigned int size, left;
    
    if (actions == NULL)
        return NULL;
    
    if (actions[-1] != (char **)RESULT_ACTIONS) {
        fprintf(stderr, "WARNING: %s called for invalid list (tag: 0x%x)",
                __FUNCTION__, (int)actions[-1]);
        return NULL;
    }
    
    p = flattened = NULL;
    size = left = 0;

    NEED_SPACE(1);
    
    *p++ = '[';
    left--;

    for (i = 0; (action = actions[i]) != NULL; i++) {
        for (j = 0, t = "["; (a = action[j]) != NULL; j++, t = " ") {
            NEED_SPACE(strlen(a) + strlen(t));
            n     = snprintf(p, left, "%s%s", t, a);
            p    += n;
            left -= n;
        }
        NEED_SPACE(1);
        *p++ = ']';
        left--;
    }
    
    NEED_SPACE(2);
    *p++ = ']';
    *p   = '\0';
    
    return flattened;
}


/********************
 * prolog_free_objects
 ********************/
void
prolog_free_objects(char ***objects)
{
    int i, p;

    if (objects == NULL)
        return;

    if (objects[-1] != (char **)RESULT_OBJECTS) {
        fprintf(stderr, "WARNING: %s called for invalid list (tag: 0x%x)",
                __FUNCTION__, (int)objects[-1]);
        return;
    }
    
    for (i = 0; objects[i]; i++) {
        for (p = 0; objects[i][p]; p++)
            free(objects[i][p]);
        free(objects[i]);
    }
    free(objects - 1);
}


/********************
 * prolog_dump_objects
 ********************/
void
prolog_dump_objects(char ***objects)
{
    int   i, p;
    char *t;

    if (objects == NULL)
        return;

    if (objects[-1] != (char **)RESULT_OBJECTS) {
        fprintf(stderr, "WARNING: %s called for invalid list (tag: 0x%x)",
                __FUNCTION__, (int)objects[-1]);
        return;
    }

    for (i = 0; objects[i]; i++) {
        printf("{ ");
        for (p = 0, t = ""; objects[i][p]; p += 2, t = ", ")
            printf("%s%s:%s", t, objects[i][p], objects[i][p+1]);
        printf(" }\n");
    }
}


/********************
 * prolog_dump_exception
 ********************/
void
prolog_dump_exception(char ***exception)
{
    if (exception == NULL)
        return;
    
    if (exception[-1] != (char **)RESULT_EXCEPTION) {
        fprintf(stderr, "WARNING: %s called for invalid list (tag: 0x%x)",
                __FUNCTION__, (int)exception[-1]);
        return;
    }

    printf("prolog exception '%s'\n", (char *)exception[0]);
}


/********************
 * prolog_free_exception
 ********************/
void
prolog_free_exception(char ***exception)
{
    if (exception == NULL)
        return;

    if (exception[-1] != (char **)RESULT_EXCEPTION) {
        fprintf(stderr, "WARNING: %s called for invalid list (tag: 0x%x)",
                __FUNCTION__, (int)exception[-1]);
        return;
    }

    FREE((char *)exception[1]);
    FREE(exception);
}




/*****************************************************************************
 *                        *** prolog list handling ***                       *
 *****************************************************************************/

/*************************
 * prolog_list_length
 *************************/
int
prolog_list_length(term_t pl_list)
{
    fid_t       frame;
    predicate_t pr_length;
    term_t      pl_args, pl_length;
    int         length;


    frame = PL_open_foreign_frame();

    pr_length = PL_predicate("length", 2, NULL);    
    pl_args   = PL_new_term_refs(2);
    pl_length = pl_args + 1;
    length    = -1;

    if (!PL_unify(pl_args, pl_list) || 
        !PL_call_predicate(NULL, PL_Q_NORMAL, pr_length, pl_args))
        goto out;

    PL_get_integer(pl_length, &length);
    
 out:
    PL_discard_foreign_frame(frame);

    return length;
}


/********************
 * prolog_list_new
 ********************/
term_t
prolog_list_new(char **items, int n, term_t result)
{
    term_t list = PL_new_term_ref();
    term_t item = PL_new_term_ref();

    if (n < 0) {                  /* NULL-terminated list, calculate items */
        n = 0;
        if (items)
            while (items[n])
                n++;
    }

    PL_put_nil(list);
    while (n-- > 0) {
        PL_put_atom_chars(item, items[n]);
        PL_cons_list(list, item, list);
    }
    
    if (result && PL_is_variable(result))
        PL_unify(list, result);
    
    return list;
}


/*************************
 * prolog_walk_list
 *************************/
int
prolog_walk_list(term_t list,
                 int (*callback)(term_t item, int i, void *data), void *data)
{
    term_t pl_list, pl_head;
    int    i, err;
    
    pl_list = PL_copy_term_ref(list);
    pl_head = PL_new_term_ref();

    for (i = err = 0; !err && PL_get_list(pl_list, pl_head, pl_list); i++)
        err = callback(pl_head, i, data);
    
    return err;
}



/*****************************************************************************
 *                       *** action/object collectors ***                    *
 *****************************************************************************/


/********************
 * is_action_list
 ********************/
static int
is_action_list(term_t list)
{
    term_t pl_list, pl_head, pl_action;
    
    pl_list   = PL_copy_term_ref(list);
    pl_head   = PL_new_term_refs(2);
    pl_action = pl_head + 1;


    /*
     * Notes:
     *   Action lists are of the form:
     *     [[action1, arg1, arg2, ...], [action2, arg1, arg2, ...], ...]
     *
     *   Object lists are of the form:
     *     [[name1, [field1, value1], [field2, value2]], ...]
     *
     *   Thus we conclude that list is an action list if the second element
     *   of the first element is not a list.
     */

    if (!PL_get_head(pl_list, pl_action))            /* get first action */
        return FALSE;
    
    if (!PL_get_list(pl_action, pl_head, pl_list))   /* get tail of action */
        return FALSE;
    
    if (!PL_get_head(pl_list, pl_head))              /* get second element */
        return FALSE;
    
    return !PL_is_list(pl_head);
}



/********************
 * collect_result
 ********************/
static int
collect_result(qid_t qid, term_t pl_retval, void *retval)
{
    char     *s;
    int       n;

    if (PL_exception(qid) != 0)
        return collect_exception(qid, retval);

    switch (PL_term_type(pl_retval)) {
        
    case PL_INTEGER:
        return !PL_get_integer(pl_retval, (int *)retval);

    case PL_FLOAT:
        return !PL_get_float(pl_retval, (double *)retval);

    case PL_ATOM:
        if (PL_is_list(pl_retval))              /* [] is an atom... */
            goto list;
        if (!PL_get_atom_chars(pl_retval, &s))
            return EIO;
        return (*(char **)retval = strdup(s)) == NULL ? EIO : 0;

    case PL_STRING:
        if (!PL_get_string_chars(pl_retval, &s, (size_t *)&n))
            return EIO;
        return (*(char **)retval = strdup(s)) == NULL ? EIO : 0;
        
    case PL_VARIABLE:
        *(void **)retval = NULL;
        return 0;

    case PL_TERM:
        if (!PL_is_list(pl_retval))
            goto invalid;
    list:
        if ((n = prolog_list_length(pl_retval)) < 0)
            return EIO;

        if (is_action_list(pl_retval)) {
            char ***actions;

            if ((actions = ALLOC_ARRAY(char **, 1 + n + 1)) == NULL)
                return ENOMEM;
            
            *actions++ = (char **)RESULT_ACTIONS;

            if (prolog_walk_list(pl_retval, collect_actions, actions)) {
                prolog_free_actions(actions);
                return EIO;
            }
            
            *(char ****)retval = actions;
            return 0;
        }
        else {
            char ***objects;
            
            if ((objects = ALLOC_ARRAY(char **, 1 + n + 1)) == NULL)
                return ENOMEM;
            
            *objects++ = (char **)RESULT_OBJECTS;

            if (prolog_walk_list(pl_retval, collect_objects, objects)) {
                prolog_free_objects(objects);
                return EIO;
            }
            
            *(char ****)retval = objects;
            return 0;
        }
        
    invalid:
    default:
        printf("cannot handle term of type %d\n", PL_term_type(pl_retval));
        return EINVAL;
    }
}


/********************
 * collect_action
 ********************/
static int
collect_action(term_t pl_param, int i, void *data)
{
    char **action = (char **)data;
    char  *param;

    if (!PL_get_chars(pl_param, &param, CVT_ALL))
        return EINVAL;

    if ((action[i] = strdup(param)) == NULL)
        return ENOMEM;
    
    return 0;
}


/********************
 * collect_actions
 ********************/
static int
collect_actions(term_t item, int i, void *data)
{
    char ***actions = (char ***)data;
    char  **action  = NULL;
    int     length, err;

    if ((length = prolog_list_length(item)) < 0)
        return EINVAL;
    
    if (length > 0) {
        if ((action = ALLOC_ARRAY(char *, length + 1)) == NULL)
            return ENOMEM;
        
        if ((err = prolog_walk_list(item, collect_action, action)) != 0)
            return err;
    }
    
    actions[i] = action;
    return 0;
}


/********************
 * collect_object
 ********************/
static int
collect_object(term_t item, int i, void *data)
{
    char   **object = (char **)data;
    term_t   pl_field, pl_value;
    char    *field, *value;

    if (i == 0) {
        if (!PL_get_chars(item, &field, CVT_ALL))
            return EINVAL;
        
        if ((object[0] = STRDUP(OBJECT_NAME)) == NULL ||
            (object[1] = STRDUP(field))  == NULL)
            return ENOMEM;
    }
    else {
        pl_field = PL_new_term_refs(2);
        pl_value = pl_field + 1;
        
        if (!PL_get_list(item, pl_field, pl_value))
            return EINVAL;
        if (!PL_get_head(pl_value, pl_value))
            return EINVAL;
        if (!PL_get_chars(pl_field, &field, CVT_ALL) ||
            !PL_get_chars(pl_value, &value, CVT_ALL))
            return EINVAL;
            
        if ((object[2*i  ] = STRDUP(field)) == NULL ||
            (object[2*i+1] = STRDUP(value)) == NULL)
            return ENOMEM;
    }

    return 0;
}
    

/********************
 * collect_objects
 ********************/
static int
collect_objects(term_t item, int i, void *data)
{
    char ***objects = (char ***)data;
    char  **object  = NULL;
    int      length, err;

    if ((length = prolog_list_length(item)) < 0)
        return EINVAL;
    
    if (length > 0) {
        if ((object = ALLOC_ARRAY(char *, 2 * length + 1)) == NULL)
            return ENOMEM;
        
        if ((err = prolog_walk_list(item, collect_object, object)) != 0)
            return err;
    }

    objects[i] = object;
    return 0;
}


/********************
 * parse_exception
 ********************/
static char *
parse_exception(term_t pl_exception)
{

    /*
     * The documentation says:
     *
     * Built-in predicates generate exceptions using a term
     * error(Formal, Context). The first argument is the `formal'
     * description of the error, specifying the class and generic
     * defined context information. When applicable, the ISO error-term
     * definition is used. The second part describes some additional
     * context to help the programmer while debugging. In its most generic
     * form this is a term of the form context(Name/Arity, Message), where
     * Name/Arity describes the built-in predicate that raised the error,
     * and Message provides an additional description of the error. Any
     * part of this structure may be a variable if no information was present.
     */

#define FAIL(fmt, args...) do { APPEND(fmt, ## args); goto out; } while (0)

#define APPEND(fmt, args...) do {                       \
        n     = snprintf(ep, left, fmt, ## args);       \
        ep   += n;                                      \
        left -= n;                                      \
    } while (0)

    fid_t  frame;
    term_t pl_terms, pl_formal, pl_context, pl_type, pl_kind, pl_what;
    
    char exception[1024], *ep, *s;
    int  size, left, n, arity, i;
    
    
    frame      = PL_open_foreign_frame();
    pl_terms   = PL_new_term_refs(5);

    i = 0;
    pl_formal  = pl_terms + i++;
    pl_context = pl_terms + i++;
    pl_type    = pl_terms + i++;
    pl_kind    = pl_terms + i++;
    pl_what    = pl_terms + i++;
    
    PL_get_arg(1, pl_exception, pl_formal);
    PL_get_arg(2, pl_exception, pl_context);
    
    ep   = exception;
    size = 0;
    left = sizeof(exception) - 1;

    if (PL_is_compound(pl_formal)) {
        /* eg: error(existence_error(procedure, foo/3), context(...)) */
        /* eg: error(type(kind, what), context(...)) */
        PL_get_name_arity(pl_formal, &pl_type, &arity);
        APPEND("%s", PL_atom_chars(pl_type));

        if (arity != 2)
            FAIL(" (unknown details");
        
        PL_get_arg(1, pl_formal, pl_kind);
        PL_get_chars(pl_kind, &s, CVT_WRITE | BUF_DISCARDABLE);
        APPEND(": %s", s);
        
        PL_get_arg(2, pl_formal, pl_what);
        if (PL_is_atomic(pl_what)) {
            PL_get_chars(pl_what, &s, CVT_ALL | BUF_DISCARDABLE);
            APPEND(", %s", s);
        }
        else if (PL_is_compound(pl_what)) {
            PL_get_chars(pl_what, &s, CVT_WRITE | BUF_DISCARDABLE);
            APPEND(", %s", s);
        }
        else
            FAIL(" (details in unknown format)");
    }
    else
        FAIL("unknown prolog exception");
    
 out:
    PL_discard_foreign_frame(frame);

    
    return STRDUP(exception);
}


/********************
 * collect_exception
 ********************/
static int
collect_exception(qid_t qid, void *retval)
{
    char       ***objects = (char ***)retval;
    term_t        pl_error;
    atom_t        pl_name;
    int           arity;
    const char   *name;
    char         *error;

    *objects = NULL;
        
    if ((pl_error = PL_exception(qid)) == 0)
        return 0;

    if (!PL_is_compound(pl_error) ||
        !PL_get_name_arity(pl_error, &pl_name, &arity))
        return EINVAL;
    
    if ((name = PL_atom_chars(pl_name)) == NULL)
        return EINVAL;

    if (arity == 2 && !strcmp(name, "error"))
        error = parse_exception(pl_error);
    else
        error = STRDUP("unknown prolog exception");
    
    printf("*** prolog exception '%s'\n", error);

    if ((objects = ALLOC_ARRAY(char **, 1 + 1 + 1)) == NULL)
        return ENOMEM;

    objects[0] = (char **)RESULT_EXCEPTION;
    objects[1] = (char **)error;
    objects[2] = NULL;

    return (objects[1] != NULL ? 0 : ENOMEM);
}


/*****************************************************************************
 *                            *** prolog shell ***                           *
 *****************************************************************************/

/********************
 * Sread_shell
 ********************/
static ssize_t
Sread_shell(void *handle, char *buf, size_t bufsize)
{
#define QUIT_COMMAND "quit"
    int fd = (int)handle;
    int n;

    if ((n = read(fd, buf, bufsize)) > 0) {
        if (n < (int)bufsize)
            buf[n] = '\0';

        /* convert CRLF to LF */
        if (n >= 2 && buf[n-1] == '\n' && buf[n-2] == '\r') {
            buf[n-2] = '\n';
            buf[--n] = '\0';
        }
        
        /* emulate EOF for our quit command */
        if (!strcmp(buf, QUIT_COMMAND"\n")) {
            *buf = '\0';
            n    = 0;
        }
    }
    
    /* disgusting kludge to get looping through multiple solutions working */
    if (n == 2 && buf[0] == ';' && buf[1] == '\n') {
        n = 1;
        buf[n] = '\0';
    }

    return (ssize_t)n;
}


/********************
 * Sclose_shell
 ********************/
static int
Sclose_shell(void *handle)
{
    return 0;
    (void)handle;
}


/********************
 * Scontrol_shell
 ********************/
static int
Scontrol_shell(void *handle, int action, void *arg)
{
    int fd = (int)handle;

    switch (action) {
    case SIO_GETFILENO:   *(int *)arg = fd; return 0;
    case SIO_SETENCODING:                   return 0;
    default:                                return -1;
    }
}


/********************
 * Sopen_shell
 ********************/
static IOSTREAM *
Sopen_shell(int fdin)
{
    static IOFUNCTIONS Sshellfunctions = {
        .read    = Sread_shell,
        .write   = NULL,
        .seek    = NULL,
        .close   = Sclose_shell,
        .control = Scontrol_shell,
        .seek64  = NULL
    };
    
    return Snew((void *)fdin, SIO_INPUT | SIO_ISATTY, &Sshellfunctions);
}


/********************
 * set_shell_IO
 ********************/
static IOSTREAM *
set_shell_IO(IOSTREAM *in)
{
    IOSTREAM    *old_in;
    predicate_t  pr_set_IO;
    fid_t        frame;
    term_t       pl_args;

    pr_set_IO = PL_predicate("set_prolog_IO", 3, NULL);
    
    old_in = Suser_input;
    frame  = PL_open_foreign_frame();

    pl_args = PL_new_term_refs(3);
    if (!PL_unify_stream(pl_args + 0, in) ||
        !PL_unify_stream(pl_args + 1, Suser_output) ||
        !PL_unify_stream(pl_args + 2, Suser_error))
        goto fail;

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_set_IO, pl_args))
        goto fail;

    PL_discard_foreign_frame(frame);
    return old_in;

 fail:
    PL_discard_foreign_frame(frame);
    return NULL;
}


/********************
 * prolog_shell
 ********************/
int
prolog_shell(int in)
{
    IOSTREAM *sin, *old;

    if ((sin = Sopen_shell(in)) == NULL)
        goto out;
    
    if ((old = set_shell_IO(sin)) == NULL)
        goto out;
    
    PL_toplevel();
    set_shell_IO(old);

 out:
    if (sin)
        Sclose(sin);
    
    return 0;
}


/*****************************************************************************
 *                          *** misc, helper routines ***                    *
 *****************************************************************************/



/********************
 * shlib_path
 ********************/
static char *
shlib_path(char *lib, char *buf, size_t bufsize)
{
    FILE  *maps = NULL;
    char  *p;
    int    plen, llen;

    snprintf(buf, bufsize - 1, "/proc/%u/maps", (int)getpid());
    if ((maps = fopen(buf, "r")) == NULL)
        return lib;

    llen = strlen(lib);
    while (fgets(buf, bufsize, maps) != NULL) {
        if ((plen = strlen(buf)) < llen)
            continue;

        if (*(p = buf + plen - 1) == '\n')
            *p-- = '\0', plen--;
        if (plen < llen)
            continue;

        p -= llen - 1;
        
        if (!strcmp(p, lib)) {
            while (p > buf) {
                if (*--p == ' ' || *p == '\t') {
                    fclose(maps);
                    return p + 1;
                }
            }
        }
    }
    
    fclose(maps);
    return lib;
}








#if 0

/*****************************************************************************
 *                         *** prolog_exec & friends ***                     *
 *****************************************************************************/


/********************
 * prolog_run
 ********************/
#if 0
int
prolog_run(char *code)
{
    fid_t       frame;
    predicate_t pr_atom2term, pr_goal;
    functor_t   f_goal;
    term_t      pl_lgb, pl_goal, pl_args;
    qid_t       qid;
    int         i, arity, status;

    /*
     * parse code using atom_to_term
     */
    
    if ((pr_atom2term = PL_predicate("atom_to_term", 3, NULL)) == NULL)
        return ENOENT;
    
    frame = PL_open_foreign_frame();

    /* atom_to_term(Line, Goal, Bindings) */
    pl_lgb = PL_new_term_refs(3);
    PL_put_atom_chars(pl_lgb, code);
    
    status = EINVAL;

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_atom2term, pl_lgb)) {
        printf("*** failed to run atom_to_term\n");
        goto out;
    }

    pl_goal = pl_lgb + 1;
    if (!PL_get_functor(pl_goal, &f_goal)) {
        printf("*** failed to get functor for goal\n");
        goto out;
    }

    /* create argument list */
    if ((arity = PL_functor_arity(f_goal)) > 0) {
        pl_args = PL_new_term_refs(arity);
        for (i = 0; i < arity; i++)
            _PL_get_arg(i + 1, pl_goal, pl_args + i);
    }
    else
        pl_args = 0;
    
    pr_goal = PL_pred(f_goal, NULL);

#if 0
    qid = PL_open_query(NULL, PL_Q_NORMAL, pr_goal, pl_args);
    while (PL_next_solution(qid))
        ;
    /* PL_exception(qid); */
    PL_close_query(qid);
#else
    PL_call_predicate(NULL, PL_Q_NORMAL, pr_goal, pl_args);

    {
        pl_result = PL_new_term_ref();
        PL_put_atom_chars(pl_result, "Result");
        
    }

#endif
    
    status = 0;

 out:
    PL_discard_foreign_frame(frame);
    return status;
}

#else

/* THIS_WORKS_SOMEWHAT */

int
prolog_run(char *code)
{
    fid_t       frame;
    predicate_t pr_a2t, pr_expq, pr_xeq, pr_write;
    term_t      pl_lgb, pl_gegbeb, pl_egeb, pl_goal, pl_args;
    int         status;

    
    /* find predicates atom_to_term and expand_query */
    if ((pr_a2t = PL_predicate("atom_to_term", 3, NULL)) == NULL)
        return ENOENT;
    
    if ((pr_expq = PL_predicate("expand_query", 4, "user")) == NULL &&
        (pr_expq = PL_predicate("expand_query", 4, NULL))   == NULL)
        return ENOENT;
    
    pr_xeq   = PL_predicate("$execute", 2, "$toplevel");
    pr_write = PL_predicate("write", 1, NULL);

    frame  = PL_open_foreign_frame();

    /* atom_to_term(Line, Goal, Bindings) */
    pl_lgb = PL_new_term_refs(3);
    PL_put_atom_chars(pl_lgb, code);
    
    status = EINVAL;

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_a2t, pl_lgb)) {
        printf("*** failed to run atom_to_term\n");
        goto out;
    }

    /* expand_query(Goal, ExpandedGoal, Bindings, ExpandedBindings) */
    pl_gegbeb = PL_new_term_refs(4);
    if (!PL_unify(pl_gegbeb + 0, pl_lgb + 1) ||
        !PL_unify(pl_gegbeb + 2, pl_lgb + 2)) {
        printf("*** failed to unify gegbeb\n");
        goto out;
    }

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_expq, pl_gegbeb)) {
        printf("*** expand_query failed\n");
        goto out;
    }

    pl_egeb = PL_new_term_refs(2);
    if (!PL_unify(pl_egeb + 0, pl_gegbeb + 1) ||
        !PL_unify(pl_egeb + 1, pl_gegbeb + 3)) {
        printf("*** failed to unify egeb\n");
        goto out;
    }

    PL_call_predicate(NULL, PL_Q_NORMAL, pr_xeq, pl_egeb);
    PL_call_predicate(NULL, PL_Q_NORMAL, pr_write, pl_egeb + 1);
    
    status = 0;
    
 out:
    PL_discard_foreign_frame(frame);

    return status;
}

#endif

#if 0

int
prolog_run(char *code)
{
    fid_t       frame;
    predicate_t pr_a2t, pr_expq;
    functor_t   xeq;
    term_t      pl_lgb, pl_gegbeb, pl_egeb, pl_goal;
    int         status;

    
    /* find predicates atom_to_term and expand_query */
    if ((pr_a2t = PL_predicate("atom_to_term", 3, NULL)) == NULL)
        return ENOENT;
    
    if ((pr_expq = PL_predicate("expand_query", 4, "user")) == NULL &&
        (pr_expq = PL_predicate("expand_query", 4, NULL))   == NULL)
        return ENOENT;
       
    xeq = PL_new_functor(PL_new_atom("$execute"), 2);
    
    frame = PL_open_foreign_frame();

    /* atom_to_term(Line, Goal, Bindings) */
    pl_lgb = PL_new_term_refs(3);
    PL_put_atom_chars(pl_lgb, code);
    
    status = EINVAL;

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_a2t, pl_lgb)) {
        printf("*** failed to run atom_to_term\n");
        goto out;
    }

    /* expand_query(Goal, ExpandedGoal, Bindings, ExpandedBindings) */
    pl_gegbeb = PL_new_term_refs(4);
    if (!PL_unify(pl_gegbeb + 0, pl_lgb + 1) ||
        !PL_unify(pl_gegbeb + 2, pl_lgb + 2)) {
        printf("*** failed to unify gegbeb\n");
        goto out;
    }

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_expq, pl_gegbeb)) {
        printf("*** expand_query failed\n");
        goto out;
    }

    pl_egeb = PL_new_term_refs(2);
    if (!PL_unify(pl_egeb + 0, pl_gegbeb + 1) ||
        !PL_unify(pl_egeb + 1, pl_gegbeb + 3)) {
        printf("*** failed to unify egeb\n");
        goto out;
    }

    PL_cons_functor(pl_goal, xeq, pl_egeb + 0, pl_egeb + 1);
    if (!PL_call(pl_goal, NULL)) {
        printf("*** $execute failed\n");
        goto out;
    }
        
    status = 0;
    

 out:
    PL_discard_foreign_frame(frame);

    return status;
}
#endif



/********************
 * prolog_exec
 ********************/
int
prolog_exec(char *code)
{
    IOSTREAM    *in, *old;
    predicate_t  pr_set_input, pr_toplevel;
    fid_t        frame;
    int          status;


    if ((pr_toplevel = PL_predicate("$toplevel", 0, NULL)) == NULL)
        return EIO;
    
    if ((in = Sopen_string(NULL, code, strlen(code), "r")) == NULL)
        return errno;

    old         = Suser_input;
    Suser_input = in;

    frame = PL_open_foreign_frame();
    PL_toplevel();
    PL_discard_foreign_frame(frame);
    status = 0;

    Suser_input = old;

#if 0
    if (in)                      /* XXX TODO: is this garbage-collected ? */
        Sclose(in);
#endif
    
    return status;
}

#endif




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/

