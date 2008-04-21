#ifndef POLICY_SET_H
#define POLICY_SET_H

#include "list.h"

typedef struct set_s {
    char         *name;                             /* name of this set */
    list_hook_t   hook;                             /* hook to more sets */
    char        **items;                            /* items in the set */
    int           nitem;                            /* number of items */
    int           nslot;                            /* number of slots */
} set_t;

set_t *set_create (char *name, char **initial_items);
void   set_destroy(set_t *set);

set_t *set_lookup(char *name);
int    set_insert(set_t *set, char *item);
int    set_delete(set_t *set, char *item);
void   set_reset (set_t *set);
int    set_member(set_t *set, char *item);



#endif /* POLICY_SET_H */




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/

