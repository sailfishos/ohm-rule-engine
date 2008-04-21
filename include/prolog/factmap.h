#ifndef __PROLOG_FACTMAP_H__
#define __PROLOG_FACTMAP_H__

#include <prolog/relation.h>
#include <prolog/ohm-fact.h>


/*
 * structure to glue the factstore to the prologish relation
 */

typedef struct {
    OhmFactStore      *store;                    /* fact store */
    OhmFactStoreView  *view;                     /* fact changesets */
    char              *key;                      /* key name in factstore */
    char             **members;                  /* fact fields of interest */
    int                nmember;                  /* number of fields */
    relation_t       *relation;                  /* associated relation */
} factmap_t;


factmap_t *factmap_create (OhmFactStore *store,
                           char *name, char *factkey, char **members);
void       factmap_destroy(factmap_t *map);
int        factmap_update (factmap_t *map);
void       factmap_dump   (factmap_t *map);




#endif /* __PROLOG_FACTMAP_H__ */




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/


