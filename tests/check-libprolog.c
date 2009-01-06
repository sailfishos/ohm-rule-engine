#include <check.h>
#include "check-libprolog.h"


static Suite *
chkprolog_suite(void)
{
    Suite *suite;
    TCase *tc_init, *tc_pred;

    suite   = suite_create("libprolog");
    tc_init = chkprolog_init_tests();
    tc_pred = chkprolog_pred_tests();

    suite_add_tcase(suite, tc_init);
    suite_add_tcase(suite, tc_pred);

    return suite;
}



int
main(int argc, char *argv[])
{
    Suite   *s;
    SRunner *sr;
    int    nfailed;
    
    s  = chkprolog_suite();
    sr = srunner_create(s);
    
    srunner_run_all(sr, CK_ENV);
    nfailed = srunner_ntests_failed(sr);

    srunner_free(sr);
    
    return nfailed == 0 ? 0 : 1;
}



/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */


