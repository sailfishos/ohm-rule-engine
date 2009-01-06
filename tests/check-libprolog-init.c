#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <check.h>

#include <prolog/prolog.h>

#define PL_DUMMY_FILE "./.a-test-file.pl"


START_TEST(missing_init)
{
    fail_unless(!prolog_load_file("whatever.pl"),
                "prolog_load_file before prolog_init should return FALSE");
}
END_TEST


START_TEST(multiple_init)
{
    fail_unless(prolog_init("check-libprolog", 0, 0, 0, 0, NULL) == 0,
                "prolog_init failed");
    fail_unless(prolog_init("check-libprolog", 0, 0, 0, 0, NULL) != 0,
                "Calling prolog_init for the 2nd time should fail");
}
END_TEST



START_TEST(non_existing)
{

    fail_unless(prolog_init("check-libprolog", 0, 0, 0, 0, NULL) == 0,
                "prolog_init failed");
    
    unlink(PL_DUMMY_FILE);
    fail_unless(!prolog_load_file(PL_DUMMY_FILE),
                "prolog_load_file should fail for non-existing files");
}
END_TEST


START_TEST(non_readable)
{
    int fd;

    fail_unless(unlink(PL_DUMMY_FILE) == 0 || errno == ENOENT);
    fail_unless((fd = creat(PL_DUMMY_FILE, O_WRONLY)) >= 0);

    fail_unless(prolog_init("check-libprolog", 0, 0, 0, 0, NULL) == 0,
                "prolog_init failed");
    fail_unless(!prolog_load_file(PL_DUMMY_FILE),
                "prolog_load_file should fail for non-readable files");
    close(fd);
    unlink(PL_DUMMY_FILE);
}
END_TEST


TCase *
chkprolog_init_tests(void)
{
    TCase *tc;

    tc = tcase_create("initalization");
    tcase_add_test(tc, missing_init);
    tcase_add_test(tc, multiple_init);
    tcase_add_test(tc, non_existing);
    tcase_add_test(tc, non_readable);
#if 0 
    tcase_add_test(tc, syntax_error);
    tcase_add_test(tc, syntax_ok);
    tcase_add_test(tc, multiple_init);
#endif
    
    return tc;
}


/* 
 * Local Variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim:set expandtab shiftwidth=4:
 */


