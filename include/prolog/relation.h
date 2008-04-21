#ifndef POLICY_RELATION_H
#define POLICY_RELATION_H

#include "list.h"


/*
 * Notes:
 *   Sets and relations operate on items. For generality and language
 *   independence we want items to be strings. For speed, however, we'd
 *   like to define the actual operations on integers. We might want to
 *   use something similar to X atoms, where we intern items by name and
 *   later refer to them by the unique integer ID we get back.
 *
 *   We could have it in the APIs without actually needing to implement it
 *   at the beginning by defining dummy typedef and macros like this...
 */

typedef char *item_t;

/*
item_t item_intern(char *item);
char * item_resolv(item_t item);
*/

#define item_intern(name) ((item_t)name)
#define item_lookup(item) ((char *)item)


typedef struct relation_s {
    char           *name;
    list_hook_t     hook;
    int             arity;
    char          **items;
    int           **relations;
    int             nrelation;
    int             nslot;
} relation_t;


relation_t *relation_create(char *name, int arity, char ***initial_items);
void        relation_destroy(relation_t *r);
relation_t *relation_lookup(char *name);
int         relation_insert(relation_t *r, char **items);
int         relation_delete(relation_t *r, char **items);
void        relation_reset(relation_t *r);
int         relation_member(relation_t *r, char **items);
void        relation_dump(relation_t *r);








#endif /* POLICY_RELATION_H */


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/

