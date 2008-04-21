#include <stdio.h>
#include <stdlib.h>

#include <prolog/prolog.h>

#define fatal(ec, fmt, args...) do {                    \
        printf("fatal error: "fmt"\n", ## args);        \
        exit(ec);                                       \
} while (0)

#define TESTPL "./libtest.pl"


int
main(int argc, char *argv[])
{
    prolog_export_t *exported, *e;

    /* initialize our prolog library */
    if (prolog_init("test", 0, 0, 0, 0) != 0)
        fatal(1, "failed to initialize prolog library");

    /* load our test file */
    if (prolog_load_file(argc < 2 ? TESTPL : argv[1]) != 0)
        fatal(2, "failed to load %s", TESTPL);
    
    if ((exported = prolog_get_exported(NULL)) == NULL)
        fatal(3, "failed to get exports from prolog");

    for (e = exported; e->name; e++)
        printf("found exported predicate: %s%s%s/%d (0x%x)\n",
               e->module ?: "", e->module ? ":" : "",
               e->name, e->arity,
               (unsigned int)e->predicate);
    
    /* clean up our prolog library */
    prolog_exit();
    
    return 0;
}




/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
*/



