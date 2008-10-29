#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>                             /* SYS_gettid */

#include <glib.h>
#include <gmodule.h>

#include <prolog/prolog.h>

#include <ohm/ohm-plugin.h>
#include <ohm/ohm-plugin-log.h>

#define PLUGIN_NAME    "rule_engine"
#define PLUGIN_VERSION "0.0.1"

#define DEFAULT_STACK  16                            /* 16k stacks */

#define STACK_LIMIT    16,16,16,16                   /* 16k stacks */
#define NO_PREDICATE   (-1)                          /* unknown predicate */

static prolog_predicate_t *discover_predicates(void);
static void                free_predicates(void);

static char **get_extensions(const char *param);
static char **get_rules     (const char *param);
static int    get_stack     (const char *param);
static int    setup         (char **extensions, char **files, int stack);

static prolog_predicate_t *predicates;
static int                 npredicate; 
static int                 busy;



/*****************************************************************************
 *                       *** initialization & cleanup ***                    *
 *****************************************************************************/

/**
 * plugin_init:
 **/
static void
plugin_init(OhmPlugin *plugin)
{
    const char *param_extensions = ohm_plugin_get_param(plugin, "extensions");
    const char *param_rules      = ohm_plugin_get_param(plugin, "rules");
    const char *param_stack      = ohm_plugin_get_param(plugin, "stacksize");
    
    char **extensions;
    char **rules;
    int    stack;
    
    extensions = get_extensions(param_extensions);
    rules      = get_rules(param_rules);
    stack      = get_stack(param_stack);

    if (rules != NULL)
        if (setup(extensions, rules, stack) != 0)
            exit(1);
    
    (void)plugin;
}


/**
 * plugin_exit:
 **/
static void
plugin_exit(OhmPlugin *plugin)
{
    free_predicates();
    prolog_exit();

    (void)plugin;
}


/*****************************************************************************
 *                           *** exported methods ***                        *
 *****************************************************************************/

/********************
 * setup_rules
 ********************/
OHM_EXPORTABLE(int, setup_rules, (char **extensions, char **files))
{
    return setup(extensions, files, DEFAULT_STACK);
}


/********************
 * find_rule
 ********************/
OHM_EXPORTABLE(int, find_rule, (char *name, int arity))
{
    prolog_predicate_t *p;
    int                 i;

    if (discover_predicates() == NULL)
        return NO_PREDICATE;
    
    for (i = 0, p = predicates; p->name != NULL; i++, p++)
        if ((arity < 0 || p->arity == arity) && !strcmp(p->name, name))
            return i;

    return NO_PREDICATE;
}


/********************
 * eval_rule
 ********************/
OHM_EXPORTABLE(int, eval_rule, (int rule, void *retval, void **args, int narg))
{
    if (rule < 0 || rule >= npredicate)
        return ENOENT;
    
    return prolog_acall(predicates + rule, retval, args, narg);
}


/********************
 * free_result
 ********************/
OHM_EXPORTABLE(void, free_result, (void *retval))
{
    char ***results = (char ***)retval;

    prolog_free_results(results);
}


/********************
 * dump_result
 ********************/
OHM_EXPORTABLE(void, dump_result, (void *retval))
{
    char ***results = (char ***)retval;

    prolog_dump_results(results);
}


/********************
 * shell
 ********************/
OHM_EXPORTABLE(void, prompt, (void))
{
    prolog_shell(fileno(stdin));
}


/********************
 * setup
 ********************/
static int
setup(char **extensions, char **files, int stack)
{
    int   i;
    char *p, *boot;

    if (busy)
        return EBUSY;
    
    boot = NULL;
    if (files != NULL) {
        if ((p = strrchr(files[0], '.')) != NULL && !strcmp(p + 1, "plc")) {
            boot = files[0];
            files++;
        }
    }

    if (prolog_init(PLUGIN_NAME, stack,stack,stack,stack, boot) != 0) {
        OHM_WARNING("%s: failed to initialize prolog library", __FUNCTION__);
        exit(1);
    }

    busy = TRUE;
    
    for (p = extensions[i=0]; p != NULL; p = extensions[++i])
        if (prolog_load_extension(p)) {
            OHM_WARNING("%s: failed to load extension \"%s\"", __FUNCTION__, p);
            return EINVAL;
        }

    for (p = files[i=0]; p != NULL; p = files[++i])
        if (prolog_load_file(p)) {
            OHM_WARNING("%s: failed to load file \"%s\"", __FUNCTION__, p);
            return EINVAL;
        }

    discover_predicates();
    
    return 0;
}


/*****************************************************************************
 *                        *** miscallaneous routines ***                     *
 *****************************************************************************/

/********************
 * discover_predicates
 ********************/
static prolog_predicate_t *
discover_predicates(void)
{
    prolog_predicate_t *p;
    
    if (predicates != NULL)
        return predicates;
    
    if ((predicates = prolog_predicates(NULL)) == NULL)
        return NULL;
    
    npredicate = 0;
    for (p = predicates; p->name != NULL; p++)
        npredicate++;
    
    return predicates;
}


/********************
 * free_predicates
 ********************/
static void
free_predicates(void)
{
    if (predicates) {
        prolog_free_predicates(predicates);
        predicates = NULL;
        npredicate = 0;
    }
}


/********************
 * get_extensions
 ********************/
static char **
get_extensions(const char *param)
{
    static char *extensions[2];

    if (param == NULL)
        return NULL;

    /* XXX TODO
     * implement support for parsing param to potentially multiple extensions
     */

    extensions[0] = (char *)param;
    extensions[1] = NULL;
    return &extensions[0];
}


/********************
 * get_rules
 ********************/
static char **
get_rules(const char *param)
{
    static char *rules[2];

    if (param == NULL)
        return NULL;

    /* XXX TODO
     * implement support for parsing param to potentially multiple rule files
     */

    rules[0] = (char *)param;
    rules[1] = NULL;
    return &rules[0];
}


/********************
 * get_stack
 ********************/
static int
get_stack(const char *param)
{
    int   size;
    char *end;

    if (param == NULL || *param == '\0')
        return DEFAULT_STACK;

    size = (int)strtol(param, &end, 10);
    
    if (end && *end != '\0')
        g_error("%s: Invalid stack size '%s' requested.", PLUGIN_NAME, param);
    
    return size ? size : DEFAULT_STACK;
}


OHM_PLUGIN_DESCRIPTION(PLUGIN_NAME, PLUGIN_VERSION,
                       "krisztian.litkey@nokia.com",
                       OHM_LICENSE_NON_FREE,
                       plugin_init,
                       plugin_exit,
                       NULL);

OHM_PLUGIN_PROVIDES_METHODS(rule_engine, 6,
    OHM_EXPORT(setup_rules, "setup"),

    OHM_EXPORT(find_rule,   "find"),
    OHM_EXPORT(eval_rule  , "eval"),
    OHM_EXPORT(free_result, "free"),
    OHM_EXPORT(dump_result, "dump"),

    OHM_EXPORT(prompt     , "prompt")
);

                            

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

