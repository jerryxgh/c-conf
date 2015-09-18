/*
 * Copyleft
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cfg.h"

/* test config file */
#define CONFIG_FILE "../conf/example.conf"

/* Test variables */
char *CONFIG_STR;
char *CONFIG_STR_LIST;
char **CONFIG_MUL_STR;
long CONFIG_INT;
uint64_t CONFIG_UINT64;

static int
load_config(int requirement)
{
    struct cfg_line cfg[] =
    {
        {"test_int", &CONFIG_INT, TYPE_INT, PARM_MAND, 0, 100},
        {"test_str", &CONFIG_STR, TYPE_STRING, PARM_OPT, 0, 0},
        {"test_str_list", &CONFIG_STR_LIST, TYPE_STRING_LIST, PARM_OPT, 0, 0},
        {"test_mul_str", &CONFIG_MUL_STR, TYPE_MULTISTRING, PARM_OPT, 0, 0},
        {"test_uint64", &CONFIG_UINT64, TYPE_UINT64, PARM_OPT, 0, 12121212121},
        {NULL, NULL, 0, 0, 0, 0}
    };

    /* initialize multistrings */
    str_strarr_init(&CONFIG_MUL_STR);

    return parse_cfg_file(CONFIG_FILE, cfg, requirement, 1);
}
static void
print_str(const char *key, const char *value)
{
    if (NULL == value)
        value = "NULL";
    fprintf(stderr, "%s: %s\n", key, value);
}

static void
print_multi_str(const char *key, char **value)
{
    if (NULL == value) {
        fprintf(stderr, "%s: %s\n", key, "NULL");
        return;
    }
    for (;NULL != *value; value++)
        fprintf(stderr, "%s: %s\n", key, *value);
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    if (FAIL == load_config(0)) {
        fprintf(stderr, "load config failed\n");
        exit(1);
    }

    print_str(      "test_str      ", CONFIG_STR);
    print_str(      "test_str_list ", CONFIG_STR_LIST);
    fprintf(stderr, "test_int      : %ld\n", CONFIG_INT);
    fprintf(stderr, "test_uint64   : %lu\n", CONFIG_UINT64);
    print_multi_str("test_mul_str  ", CONFIG_MUL_STR);
    return 0;
}
