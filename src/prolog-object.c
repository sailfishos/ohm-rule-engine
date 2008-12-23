#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <SWI-Stream.h>
#include <SWI-Prolog.h>

#include <prolog/prolog.h>

#include "libprolog.h"

#define OBJECT_NAME "name"


/********************
 * collect_object
 ********************/
static int
collect_object(term_t item, int i, void *data)
{
    char   **object = (char **)data;
    term_t   pl_field, pl_value;
    char    *field, *value, *type;
    double  *d;
    size_t   dummy;

    if (i == 0) {
        if (!PL_get_chars(item, &field, CVT_ALL))
            return EINVAL;
        
        if ((object[0] = STRDUP(OBJECT_NAME)) == NULL ||
            (object[2] = STRDUP(field)) == NULL)
            return ENOMEM;
        object[1] = (char *)'s';
    }
    else {
        pl_field = PL_new_term_refs(2);
        pl_value = pl_field + 1;
        
        if (!PL_get_list(item, pl_field, pl_value))
            return EINVAL;
        if (!PL_get_head(pl_value, pl_value))
            return EINVAL;
        if (!PL_get_chars(pl_field, &field, CVT_ALL))
            return EINVAL;
        
        if ((field = STRDUP(field)) == NULL)
            return ENOMEM;

        switch (PL_term_type(pl_value)) {
        case PL_ATOM:
            if (!PL_get_atom_chars(pl_value, &value))
                return EINVAL;
            value = STRDUP(value);
            type  = (char *)'s';
            break;
        case PL_STRING:
            if (!PL_get_string_chars(pl_value, &value, &dummy))
                return EINVAL;
            value = STRDUP(value);
            type  = (char *)'s';
            break;
        case PL_INTEGER:
            /* XXX TODO: change object to void ** to avoid breaking
               gcc's strict aliasing */
            if (!PL_get_integer(pl_value, (int *)(void *)&value))
                return EINVAL;
            type = (char *)'i';
            break;
        case PL_FLOAT:
            if (ALLOC_OBJ(d) == NULL)
                return ENOMEM;
            if (!PL_get_float(pl_value, d))
                return EINVAL;
            value = (char *)d;
            type  = (char *)'d';
            break;
        default:
            PROLOG_ERROR("%s: invalid prolog type (%d) for object field",
                         __FUNCTION__, PL_term_type(pl_value));
            return EINVAL;
        }
        
        object[3*i  ] = field;
        object[3*i+1] = type;
        object[3*i+2] = value;
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

    if ((length = swi_list_length(item)) < 0)
        return EINVAL;
    
    if (length > 0) {
        if ((object = ALLOC_ARRAY(char *, 3 * length + 1)) == NULL)
            return ENOMEM;
        
        if ((err = swi_walk_list(item, collect_object, object)) != 0)
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
            FAIL(" (unknown details)");
        
        PL_get_arg(1, pl_formal, pl_kind);
        if (!PL_get_chars(pl_kind, &s, CVT_WRITE | BUF_DISCARDABLE))
            FAIL(" (details in unknown format)");
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
    char       ***objects;
    term_t        pl_error;
    atom_t        pl_name;
    int           arity;
    const char   *name;
    char         *error;

    if ((pl_error = PL_exception(qid)) == 0) {
        *(char **)retval = NULL;
        return 0;
    }

#if 0
    error = NULL;
    PL_get_chars(pl_error, &error, CVT_WRITE | BUF_DISCARDABLE);
    printf("*** write(exception): \"%s\" ***\n", error ? error : "<unknown>");
#endif

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

    *(char ****)retval = objects + 1;

    return (objects[1] != NULL ? 0 : ENOMEM);
}


/********************
 * libprolog_collect_result
 ********************/
int
libprolog_collect_result(qid_t qid, term_t pl_retval, void *retval)
{
    char ***objects;
    int      n;


    if (PL_exception(qid) != 0)
        return collect_exception(qid, retval);

    switch (PL_term_type(pl_retval)) {
        
    case PL_VARIABLE:                /* XXX hmm... isn't this an error ? */
        *(void **)retval = NULL;
        return 0;
        
    case PL_ATOM:                                    /* [] is an atom... */
    case PL_TERM:
        if (!PL_is_list(pl_retval))
            goto invalid;
        
        if ((n = swi_list_length(pl_retval)) < 0)
            return EIO;
            
        if ((objects = ALLOC_ARRAY(char **, 1 + n + 1)) == NULL)
            return ENOMEM;
            
        *objects++ = (char **)RESULT_OBJECTS;
        
        if (swi_walk_list(pl_retval, collect_objects, objects)) {
            prolog_free_objects(objects);
            return EIO;
        }
            
        *(char ****)retval = objects;
        return 0;
        
    invalid:
    default:
        PROLOG_WARNING("%s: cannot handle non-list term type %d", __FUNCTION__,
                       PL_term_type(pl_retval));
        return EINVAL;
    }
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
        PROLOG_WARNING("%s: called for invalid list (tag: 0x%x)",
                       __FUNCTION__, (int)objects[-1]);
        return;
    }
    
    for (i = 0; objects[i]; i++) {
        for (p = 0; objects[i][p]; p += 3) {
            FREE(objects[i][p]);
            switch ((int)objects[i][p+1]) {
            case 's':
            case 'd': FREE(objects[i][p+2]);
                break;
            default:
                break;
            }
        }
        FREE(objects[i]);
    }
    FREE(objects - 1);
}


/********************
 * prolog_dump_objects
 ********************/
void
prolog_dump_objects(char ***objects)
{
    int   i, p, type;
    char *field, *value, *t;

    if (objects == NULL)
        return;

    if (objects[-1] != (char **)RESULT_OBJECTS) {
        PROLOG_WARNING("%s: called for invalid list (tag: 0x%x)",
                       __FUNCTION__, (int)objects[-1]);
        return;
    }

    for (i = 0; objects[i]; i++) {
        if (objects[i][0] != NULL && !strcmp(objects[i][0], "name") &&
            objects[i][2] != NULL) {
            printf("%s: ", objects[i][2]);
            p = 3;
        }
        else
            p = 0;

        printf("{ ");
        t = "";
        while (objects[i][p] != NULL) {
            field = objects[i][p];
            type  = (int)objects[i][p+1];
            value = objects[i][p+2];
            printf("%s%s: ", t, field);
            switch (type) {
            case 's': printf("'%s'", value);          break;
            case 'i': printf("%d", (int)value);       break;
            case 'd': printf("%f", *(double *)value); break;
            default:  printf("<unknown>");            break;
            }
            p += 3;
            t = ", ";
        }
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
        PROLOG_WARNING("%s: called for invalid list (tag: 0x%x)",
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
        PROLOG_WARNING("%s: called for invalid list (tag: 0x%x)",
                       __FUNCTION__, (int)exception[-1]);
        return;
    }

    FREE((char *)exception[0]);
    FREE(exception - 1);
}


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
    case RESULT_OBJECTS:   prolog_free_objects(results);   break;
    case RESULT_EXCEPTION: prolog_free_exception(results); break;
    default:
        PROLOG_WARNING("%s: called with invalid result type %d",
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
    case RESULT_OBJECTS:   prolog_dump_objects(results);   break;
    case RESULT_EXCEPTION: prolog_dump_exception(results); break;
    default:
        PROLOG_WARNING("%s: called with invalid result type %d",
                       __FUNCTION__, tag);
    }
}






/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */

