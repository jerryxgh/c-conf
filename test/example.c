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
long CONFIG_INT;

static int
load_config(int requirement)
{
    struct cfg_line cfg[] =
    {
        {"test_int", &CONFIG_INT, TYPE_INT, PARM_MAND, 0, 100},
        {"test_str", &CONFIG_STR, TYPE_STRING, PARM_OPT, 0, 0},
        {NULL, NULL, 0, 0, 0, 0}
    };

    /* initialize multistrings */
    /* str_strarr_init(&CONFIG_MUL_STR); */

    return parse_cfg_file(CONFIG_FILE, cfg, requirement, 1);
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
    if (FAIL == load_config(0)) {
        fprintf(stderr, "load config failed\n");
        exit(1);
    }

    printf("load config succeed\n");

    /* printf("test_str: %s\n", CONFIG_STR); */
    fprintf(stderr, "test_int: %ld\n", CONFIG_INT);
    return 0;
}
