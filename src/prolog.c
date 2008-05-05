#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <prolog/prolog.h>
#include <SWI-Prolog.h>

#ifndef PATH_MAX
#  define PATH_MAX 256
#endif

#define ALLOC_ARRAY ALLOC_ARR
#define LIBPROLOG     "libprolog.so"
#define PRED_EXPORTED "exported"

#define NATIVE  0
#define FOREIGN 1

static int   load_file(char *path, int foreign);
static char *shlib_path(char *lib, char *buf, size_t size);
static int   collect_predicates(term_t pl_descriptor, int i, void *data);
static int   collect_result    (term_t pl_retval, void *retval);


static char libpl[PATH_MAX];
static char lstack[16], gstack[16], tstack[16], astack[16];

static char *pl_argv[] = {
    NULL,                                      /* argv[0] */
    "-L16k",                                   /* local stack size */
    "-G16k",                                   /* global stack size */
    "-T16k",                                   /* trail stack size */
    "-A16k",                                   /* argument stack size */
    "-q",                                      /* seems to get ignored... */
    "-nosignals",                              /* no prolog signal handling */
    "-tty"                                     /* no controlling TTY */
};
static int pl_argc = sizeof(pl_argv) / sizeof(pl_argv[0]);



/*****************************************************************************
 *                      *** initialization & cleanup ***                     *
 *****************************************************************************/

/*************************
 * prolog_init
 *************************/
int
prolog_init(char *argv0, int lsize, int gsize, int tsize, int asize)
{
    int i, status;

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
     */
    

    putenv("SWI_HOME_DIR="PROLOG_HOME);    /* hmm... is this needed now ? */

    snprintf(lstack, sizeof(lstack), "-L%dk", lsize ?: 16);
    snprintf(gstack, sizeof(gstack), "-G%dk", gsize ?: 16);
    snprintf(tstack, sizeof(tstack), "-T%dk", tsize ?: 16);
    snprintf(astack, sizeof(astack), "-A%dk", asize ?: 16);
    
    pl_argv[i=0] = shlib_path(LIBPROLOG, libpl, sizeof(libpl));
    pl_argv[++i] = lstack;
    pl_argv[++i] = gstack;
    pl_argv[++i] = tstack;
    pl_argv[++i] = astack;

    if ((status = !PL_initialise(pl_argc, pl_argv)))
        PL_cleanup(0);

    return status;
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
    predicate_t  pr_load;
    term_t       pl_path;
    
    if (!(pr_load = PL_predicate(loader, 1, NULL)))
        return 1;
    pl_path = PL_new_term_ref(); PL_put_atom_chars(pl_path, path);

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_load, pl_path))
        return 1;
    else
        return 0;
}


/*****************************************************************************
 *                      *** misc. prolog functionality ***                   *
 *****************************************************************************/


/********************
 * prolog_predicates
 ********************/
prolog_predicate_t *
prolog_predicates(char *query)
{
    char        *exported   = query ? query : PRED_EXPORTED;
    predicate_t pr_exported = PL_predicate(exported, 1, NULL);
    term_t      pl_predlist = PL_new_term_ref();

    prolog_predicate_t *predicates;
    int                 npredicate;

    if (!PL_call_predicate(NULL, PL_Q_NORMAL, pr_exported, pl_predlist))
        return NULL;
    
    if ((npredicate = prolog_list_length(pl_predlist)) <= 0)
        return NULL;

    if ((predicates = ALLOC_ARRAY(prolog_predicate_t, npredicate + 1)) == NULL)
        return NULL;
    
    if (prolog_walk_list(pl_predlist, collect_predicates, predicates))
        return NULL;
    
    return predicates;
}


/********************
 * prolog_call
 ********************/
int
prolog_call(prolog_predicate_t *p, void *retval, ...)
{
    va_list  ap;
    fid_t    frame;
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

    success = PL_call_predicate(NULL, PL_Q_NORMAL, p->predicate, pl_args);
    if (success)
        if (collect_result(pl_retval, retval))
            success = FALSE;
    
    PL_close_foreign_frame(frame);
    
    return success;
} 


/********************
 * prolog_free_actions
 ********************/
void
prolog_free_actions(char ***actions)
{
    int a, p;
    
    if (actions) {
        for (a = 0; actions[a]; a++) {
            for (p = 0; actions[a][p]; p++)
                free(actions[a][p]);
            free(actions[a]);
        }
        free(actions);
    }
}


/********************
 * prolog_dump_actions
 ********************/
void
prolog_dump_actions(char ***actions)
{
    int a, p;
    
    if (actions) {
        for (a = 0; actions[a]; a++) {
            printf("(%s", actions[a][0]);
            for (p = 1; actions[a][p]; p++)
                printf(", %s", actions[a][p]);
            printf(")\n");
        }
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
    int    i, j, size, left, n;

    p = flattened = NULL;
    size = left = 0;

    NEED_SPACE(1);
    
    *p++ = '[';
    left--;

    if (actions != NULL) {
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
    }
    
    NEED_SPACE(2);
    *p++ = ']';
    *p   = '\0';

    return flattened;
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
        
        if ((predicate->module = strdup(name)) == NULL)
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
 * collect_result
 ********************/
static int
collect_result(term_t pl_retval, void *retval)
{
    char     *s;
    size_t    n;
    char   ***actions;

    switch (PL_term_type(pl_retval)) {
        
    case PL_INTEGER:
        return !PL_get_integer(pl_retval, (int *)retval);

    case PL_FLOAT:
        return !PL_get_float(pl_retval, (double *)retval);

    case PL_ATOM:
        if (!PL_get_atom_chars(pl_retval, &s))
            return EIO;
        return (*(char **)retval = strdup(s)) == NULL ? EIO : 0;

    case PL_STRING:
        if (!PL_get_string_chars(pl_retval, &s, &n))
            return EIO;
        return (*(char **)retval = strdup(s)) == NULL ? EIO : 0;
        
    case PL_VARIABLE:
        *(void **)retval = NULL;
        return 0;

    case PL_TERM:
        if (!PL_is_list(pl_retval))
            goto invalid;
        
        if ((n = prolog_list_length(pl_retval)) < 0)
            return EIO;

        if ((actions = ALLOC_ARRAY(char **, n + 1)) == NULL)
            return ENOMEM;
        
        if (prolog_walk_list(pl_retval, collect_actions, actions)) {
            prolog_free_actions(actions);
            return EIO;
        }
        
        *(char ****)retval = actions;
        return 0;
        
    invalid:
    default:
        printf("cannot handle term of type %d\n", PL_term_type(pl_retval));
        printf("is_list: %d\n", PL_is_list(pl_retval));
        return EINVAL;
    }
}


/*************************
 * prolog_query_once
 *************************/
int
prolog_query_once(char *predicate, int arity, term_t pl_arguments, char *module)
{
    predicate_t pr_predicate;

    pr_predicate = PL_predicate(predicate, arity, module);
    return PL_call_predicate(NULL, PL_Q_NORMAL, pr_predicate, pl_arguments);
}


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
    PL_close_foreign_frame(frame);

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
 * prolog_prompt
 *************************/
void
prolog_prompt(void)
{
    PL_toplevel();
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



/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/

