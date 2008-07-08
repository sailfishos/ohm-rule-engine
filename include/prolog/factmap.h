#ifndef __PROLOG_FACTMAP_H__
#define __PROLOG_FACTMAP_H__

#include <prolog/relation.h>
#include <ohm/ohm-fact.h>


/*
 * structure to glue the factstore to the prologish relation
 */

typedef struct factmap_s factmap_t;

struct factmap_s {
    OhmFactStore      *store;                         /* fact store */
    OhmFactStoreView  *view;                          /* fact store view */
    char              *key;                           /* factstore key */
    char             **members;                       /* fields of interest */
    int                nmember;                       /* number of fields */
    relation_t       *relation;                       /* associated relation */
    int             (*filter)(int, char **, void *);  /* optional filter */
    void             *filter_data;                    /* optional filter data */
};


factmap_t *factmap_create (OhmFactStore *store,
                           char *name, char *factkey, char **members,
                           int (*filter)(int, char **, void *), void *data);
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


