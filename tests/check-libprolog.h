#ifndef __CHECK_LIBPROLOG_H__
#define __CHECK_LIBPROLOG_H__

#include <check.h>

void chkprolog_init_tests(Suite *suite);
void chkprolog_pred_tests(Suite *suite);
int  chkprolog_fork_status(void);

SRunner *chkprolog_srunner(void);

#endif /* __CHECK_LIBPROLOG_H__ */
