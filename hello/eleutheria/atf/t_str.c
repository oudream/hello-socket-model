/*
 * Compile with:
 * gcc t_str.c -o t_str `pkg-config --cflags --libs atf-c`
 */

#include <stdio.h>
#include <string.h>

#include <atf-c.h>

/* Test case 1 -- strstr() */
ATF_TC(test_strstr);
ATF_TC_HEAD(test_strstr, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the strstr(3) function");
}
ATF_TC_BODY(test_strstr, tc)
{
    char s1[] = "This is a big string";
    char s2[] = "big";
    char s3[] = "none";

    ATF_CHECK(strstr(s1, s2) != NULL);
    ATF_CHECK(strstr(s1, s3) == NULL);
}

/* Test case 2 -- strcmp() */
ATF_TC(test_strcmp);
ATF_TC_HEAD(test_strcmp, tc)
{
    atf_tc_set_md_var(tc, "descr", "Tests the strcmp(3) function");
}
ATF_TC_BODY(test_strcmp, tc)
{
    char s1[] = "a";
    char s2[] = "b";
    char s3[] = "aaa";

    ATF_CHECK(strcmp(s1, s1) == 0);
    ATF_CHECK(strcmp(s1, s2) < 0);
    ATF_CHECK(strcmp(s1, s3) < 0);
}

/* Add test cases to test program */
ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, test_strstr);
    ATF_TP_ADD_TC(tp, test_strcmp);

    return atf_no_error();
}
