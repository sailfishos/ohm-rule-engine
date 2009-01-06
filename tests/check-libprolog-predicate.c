#include <check.h>


START_TEST(dummy)
{
    fail_unless(0 == 0);
}
END_TEST


TCase *
chkprolog_pred_tests(void)
{
    TCase *tc;

    tc = tcase_create("predicates");
    tcase_add_test(tc, dummy);

    return tc;
}


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */


