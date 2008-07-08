#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <SWI-Prolog.h>

#include <ohm/ohm-fact.h>

typedef struct {
    OhmFactStore  *store;                      /* fact store */
    GSList        *facts;                      /* matching facts */
    char         **fields;                     /* field names of interest */
    int            nfield;                     /* number of -||- */
} context_t;


/*************************
 * list_length
 *************************/
static int
list_length(term_t pl_list)
{
    predicate_t pr_length;
    term_t      pl_args, pl_length;
    int         length;

    pr_length = PL_predicate("length", 2, NULL);    
    pl_args   = PL_new_term_refs(2);
    pl_length = pl_args + 1;

    if (!PL_unify(pl_args, pl_list) || 
        !PL_call_predicate(NULL, PL_Q_NORMAL, pr_length, pl_args))
        length = -1;
    else
        PL_get_integer(pl_length, &length);
    
    return length;
}


/********************
 * get_field_names
 ********************/
static int
get_field_names(context_t *ctx, term_t pl_fields)
{
#define MAX_LENGTH 64                           /* max length of a field name */

    term_t  pl_list, pl_head;
    int     i, n, left;
    size_t  size;
    char   *p, *field;
    

    if ((n = list_length(pl_fields)) < 0)
        return EINVAL;

    size = n * sizeof(ctx->fields[0]) + n * MAX_LENGTH;
    if ((ctx->fields = malloc(size)) == NULL)
        return ENOMEM;
    memset(ctx->fields, 0, size);
    ctx->nfield = n;

    p    = ((char *)ctx->fields) + n * sizeof(ctx->fields[0]);
    left = size - n * sizeof(ctx->fields[0]);

    pl_list = PL_copy_term_ref(pl_fields);      /* XXX is this really needed? */
    pl_head = PL_new_term_ref();
    for (i = 0; i < n && PL_get_list(pl_list, pl_head, pl_list); i++) {
        if (!PL_get_atom_chars(pl_head, &field))
            goto fail;
        ctx->fields[i] = p;
        size = snprintf(p, left, "%s", field);
        p    += size + 1;
        left -= size + 1;
        if (left < 0)
            goto fail;
    }
    
    return 0;
    
 fail:
    if (ctx->fields != NULL) {
        free(ctx->fields);
        ctx->fields = NULL;
    }
    return EINVAL;
}


/********************
 * fact_field_value
 ********************/
static char *
fact_field_value(OhmFact *fact, char *field, char *buf, size_t size)
{
    GValue *value, gstr = {0};
    
    if ((value = ohm_fact_get(fact, field)) == NULL)
        return NULL;
    
    if (G_VALUE_HOLDS_STRING(value)) {
        snprintf(buf, size, (char *)g_value_get_string(value));
        return buf;
    }

    if (!g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_STRING))
        return NULL;
            
    g_value_init(&gstr, G_TYPE_STRING);
    if (!g_value_transform(value, &gstr))
        return NULL;
            
    snprintf(buf, size, "%s", g_value_get_string(&gstr));
    printf("*** value of %s: %s\n", field, buf);
    g_value_unset(&gstr);
    
    return buf;
}


/********************
 * fact_values
 ********************/
static int
fact_values(context_t *ctx, OhmFact *fact, term_t *pl_values)
{
    int     n    = ctx->nfield;
    term_t  list = PL_new_term_ref();
    term_t  item = PL_new_term_ref();
    char    value[64];

    PL_put_nil(list);
    while (n-- > 0) {
        if (!fact_field_value(fact, ctx->fields[n], value, sizeof(value)))
            return EINVAL;
        PL_put_atom_chars(item, value);
        PL_cons_list(list, item, list);
    }
    
    *pl_values = list;
    return 0;
}


/********************
 * pl_fact_exists
 ********************/
static foreign_t
pl_fact_exists(term_t pl_name,
               term_t pl_fields, term_t pl_list, control_t handle)
{
    context_t  *ctx;
    char       *name, factname[64];
    fid_t       frame;
    term_t      pl_values;
    OhmFact    *f;
    
    switch (PL_foreign_control(handle)) {
    case PL_FIRST_CALL:
        if (!PL_is_list(pl_fields) || /*!PL_is_list(pl_list) ||*/
            !PL_get_chars(pl_name, &name, CVT_ALL))
            PL_fail;
        strcpy(factname, name);

        if ((ctx = malloc(sizeof(*ctx))) == NULL)
            PL_fail;
        memset(ctx, 0, sizeof(*ctx));
        
        if (get_field_names(ctx, pl_fields) != 0) {
            free(ctx);
            PL_fail;
        }
        
        ctx->store = ohm_fact_store_get_fact_store();
        ctx->facts = ohm_fact_store_get_facts_by_name(ctx->store, factname);
        break;
        
    case PL_REDO:
        ctx = PL_foreign_context_address(handle);
        break;
        
    case PL_CUTTED:
        ctx = PL_foreign_context_address(handle);
        goto nomore;

    default:
        PL_fail;
    }


    frame = PL_open_foreign_frame();
    while (ctx->facts != NULL) {
        f = (OhmFact *)ctx->facts->data;
        ctx->facts = g_slist_next(ctx->facts);

        if (!fact_values(ctx, f, &pl_values) && PL_unify(pl_list, pl_values)) {
            PL_close_foreign_frame(frame);
            PL_retry_address(ctx);
        }
        
        PL_rewind_foreign_frame(frame);
    }
    PL_close_foreign_frame(frame);
    
 nomore:
    if (ctx->fields)
        free(ctx->fields);
    free(ctx);
    PL_fail;
}


/********************
 * install
 ********************/
install_t
install(void)
{
    PL_register_foreign("fact_exists",
                        3, pl_fact_exists, PL_FA_NONDETERMINISTIC);
}

/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */

