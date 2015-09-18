/*
 * Copyleft
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "str.h"
#include "cfg.h"

char *CONFIG_FILE           = NULL;
char *CONFIG_LOG_FILE       = NULL;

int CONFIG_LOG_FILE_SIZE    = 1;
int CONFIG_ALLOW_ROOT       = 0;
int CONFIG_TIMEOUT          = 3;

static int __parse_cfg_file(const char *cfg_file, struct cfg_line *cfg,
                            int level, int optional, int strict);

/**
 * See whether a file (e.g., "parameter.conf") matches a pattern (e.g.,
 * "p*.conf")
 *
 * @return
 *   SUCCEED - file matches a pattern
 *   FAIL - otherwise
 */
static int
match_glob(const char *file, const char *pattern)
{
    const char  *f, *g, *p, *q;

    f = file;
    p = pattern;

    while (1) {
        /* corner case */
        if ('\0' == *p)
            return '\0' == *f ? SUCCEED : FAIL;

        /* find a set of literal characters */
        while ('*' == *p)
            p++;

        for (q = p; '\0' != *q && '*' != *q; q++)
            ;

        /* if literal characters are at the beginning... */
        if (pattern == p) {
            if (0 != strncmp(f, p, q - p))
                return FAIL;

            f += q - p;
            p = q;

            continue;
        }

        /* if literal characters are at the end... */
        if ('\0' == *q) {
            for (g = f; '\0' != *g; g++)
                ;

            if (g - f < q - p)
                return FAIL;
            return 0 == strcmp(g - (q - p), p) ? SUCCEED : FAIL;
        }

        /* if literal characters are in the middle... */
        while (1) {
            if ('\0' == *f)
                return FAIL;
            if (0 == strncmp(f, p, q - p)) {
                f += q - p;
                p = q;

                break;
            }

            f++;
        }
    }
}

/**
 * Parse a glob like "/usr/local/etc/xxx.conf.d/p*.conf" into
 * "/usr/local/etc/xxx.conf.d" and "p*.conf" parts
 *
 * @param glob
 *   [IN] glob as specified in Include directive
 * @param path
 *   [OUT] parsed path, either directory or file
 * @param pattern
 *   [OUT] parsed pattern, if path is directory
 *
 * @return
 *   SUCCEED - glob is valid and was parsed successfully
 *   FAIL - otherwise
 */
static int
parse_glob(const char *glob, char **path, char **pattern)
{
    const char *p;

    if (NULL == (p = strchr(glob, '*'))) {
        *path = str_strdup(glob);
        if (NULL == *path) {
            return FAIL;
        }
        *pattern = NULL;

        goto trim;
    }

    if (NULL != strchr(p + 1, PATH_SEPARATOR)) {
        LOG_ERR("%s: glob pattern should be the last component of the path\n",
                glob);
        return FAIL;
    }

    do {
        if (glob == p) {
            LOG_ERR("%s: path should be absolute\n", glob);
            return FAIL;
        }

        p--;
    }
    while (PATH_SEPARATOR != *p)
        ;

    *path = str_strdup(glob);
    if (NULL == *path) {
        return FAIL;
    }
    (*path)[p - glob] = '\0';

    *pattern = str_strdup(p + 1);
    if (NULL == *pattern) {
        free(*path);
        return FAIL;
    }

trim:
    if (0 != str_rtrim(*path, "/") && NULL == *pattern) {
        *pattern = str_strdup("*");   /* make sure path is a directory */
        if (NULL == *pattern) {
            free(*path);
            return FAIL;
        }
    }

    if ('\0' == (*path)[0] && '/' == glob[0]) {
        /* retain forward slash for "/" */

        (*path)[0] = '/';
        (*path)[1] = '\0';
    }

    return SUCCEED;
}

/**
 * Parse directory with configuration files
 *
 * @param path
 *   full path to directory
 * @param pattern
 *   pattern that files in the directory should match
 * @param cfg
 *   pointer to configuration parameter structure
 * @param level
 *   a level of included file
 * @param strict
 *   treat unknown parameters as error
 *
 * @return
 *   SUCCEED - parsed successfully
 *   FAIL - error processing directory
 */
static int
parse_cfg_dir(const char *path, const char *pattern, struct cfg_line *cfg,
              int level, int strict)
{
    DIR             *dir;
    struct dirent   *d;
    struct stat      sb;
    char            *file = NULL;
    int              ret = FAIL;

    if (NULL == (dir = opendir(path))) {
        goto out;
    }

    while (NULL != (d = readdir(dir))) {
        file = str_dsprintf(file, "%s/%s", path, d->d_name);

        if (0 != stat(file, &sb) || 0 == S_ISREG(sb.st_mode))
            continue;

        if (NULL != pattern && SUCCEED != match_glob(d->d_name, pattern))
            continue;

        if (SUCCEED != __parse_cfg_file(file, cfg, level, CFG_FILE_REQUIRED,
                                        strict))
            goto close;
    }

    ret = SUCCEED;
close:
    if (0 != closedir(dir)) {
        ret = FAIL;
    }

    if (NULL != file) {
        free(file);
    }

out:
    return ret;
}

/**
 *  Parse "Include=..." line in configuration file
 *
 * @param cfg_file
 *   full name of config file
 * @param cfg
 *   pointer to configuration parameter structure
 * @param level
 *   a level of included file
 * @param strict
 *   treat unknown parameters as error
 *
 * @return
 *   SUCCEED - parsed successfully
 *   FAIL - error processing object
 */
static int  parse_cfg_object(const char *cfg_file, struct cfg_line *cfg,
                             int level, int strict)
{
    int ret = FAIL;
    char *path = NULL, *pattern = NULL;
    struct stat  sb;

    if (SUCCEED != parse_glob(cfg_file, &path, &pattern))
        goto clean;

    if (0 != stat(path, &sb)) {
        goto clean;
    }

    if (0 == S_ISDIR(sb.st_mode)) {
        if (NULL == pattern) {
            ret = __parse_cfg_file(path, cfg, level, CFG_FILE_REQUIRED, strict);
            goto clean;
        }

        LOG_ERR("%s: base path is not a directory\n", cfg_file);
        goto clean;
    }

    ret = parse_cfg_dir(path, pattern, cfg, level, strict);
clean:
    free(pattern);
    free(path);

    return ret;
}

/**
 * Parse configuration file
 *
 * @param cfg_file
 *   full name of config file
 * @param cfg
 *   pointer to configuration parameter structure
 * @param level
 *   a level of included file
 * @param optional
 *   do not treat missing configuration file as error
 * @param strict
 *   treat unknown parameters as error
 *
 * @return
 *  SUCCEED - parsed successfully
 *  FAIL - error processing config file
 */
static int
__parse_cfg_file(const char *cfg_file, struct cfg_line *cfg, int level,
                 int optional, int strict)
{
#define MAX_INCLUDE_LEVEL   10

#define CFG_LTRIM_CHARS "\t "
#define CFG_RTRIM_CHARS CFG_LTRIM_CHARS "\r\n"

    FILE *file;
    int i, lineno, param_valid;
    char line[MAX_STRING_LEN], *parameter, *value;
    uint64_t    var;
    if (++level > MAX_INCLUDE_LEVEL) {
        LOG_ERR("Recursion detected! Skipped processing of '%s'.", cfg_file);
        return FAIL;
    }

    if (NULL != cfg_file) {
        if (NULL == (file = fopen(cfg_file, "r")))
            goto cannot_open;

        for (lineno = 1; NULL != fgets(line, sizeof(line), file); lineno++) {
            str_ltrim(line, CFG_LTRIM_CHARS);
            str_rtrim(line, CFG_RTRIM_CHARS);

            if ('#' == *line || '\0' == *line)
                continue;

            /* we only support UTF-8 characters in the config file */
            if (SUCCEED != str_is_utf8(line))
                goto non_utf8;

            parameter = line;
            if (NULL == (value = strchr(line, '=')))
                goto non_key_value;

            *value++ = '\0';

            str_rtrim(parameter, CFG_RTRIM_CHARS);
            str_ltrim(value, CFG_LTRIM_CHARS);

            if (0 == strcmp(parameter, "Include")) {
                if (FAIL == parse_cfg_object(value, cfg, level, strict)) {
                    fclose(file);
                    goto error;
                }

                continue;
            }

            param_valid = 0;

            for (i = 0; NULL != cfg[i].parameter; i++) {
                if (0 != strcmp(cfg[i].parameter, parameter))
                    continue;

                param_valid = 1;

                switch (cfg[i].type) {
                case TYPE_INT:
                    if (FAIL == str2uint64(value, "KMGT", &var))
                        goto incorrect_config;

                    if (cfg[i].min > var ||
                        (0 != cfg[i].max && var > cfg[i].max))
                        goto incorrect_config;

                    *((int *)cfg[i].variable) = (int)var;
                    break;
                case TYPE_STRING_LIST:
                    str_trim_str_list(value, ',');
                    /* break; is not missing here */
                case TYPE_STRING:
                    *((char **)cfg[i].variable) = str_strdup(value);
                    if (NULL == *((char **)cfg[i].variable)) {
                        goto copy_str_error;
                    }
                    break;
                case TYPE_MULTISTRING:
                    if (SUCCEED != str_strarr_add(cfg[i].variable, value)) {
                        goto copy_str_error;
                    }
                    break;
                case TYPE_UINT64:
                    if (FAIL == str2uint64(value, "KMGT", &var))
                        goto incorrect_config;

                    if (cfg[i].min > var || (0 != cfg[i].max && var > cfg[i].max))
                        goto incorrect_config;

                    *((uint64_t *)cfg[i].variable) = var;
                    break;
                default:
                    break;
                }
            }

            if (0 == param_valid && CFG_STRICT == strict)
                goto unknown_parameter;
        }
        fclose(file);
    }

    if (1 != level) /* skip mandatory parameters check for included files */
        return SUCCEED;

    for (i = 0; NULL != cfg[i].parameter; i++) {
        /* check for mandatory parameters */

        if (PARM_MAND != cfg[i].mandatory)
            continue;

        switch (cfg[i].type) {
        case TYPE_INT:
            if (0 == *((int *)cfg[i].variable))
                goto missing_mandatory;
            break;
        case TYPE_STRING:
        case TYPE_STRING_LIST:
            if (NULL == (*(char **)cfg[i].variable))
                goto missing_mandatory;
            break;
        default:
            break;
        }
    }

    return SUCCEED;
cannot_open:
    if (0 != optional)
        return SUCCEED;
    goto error;
non_utf8:
    fclose(file);
    LOG_ERR("non-UTF-8 character at line %d (%s) in config file [%s]", lineno,
            line, cfg_file);
    goto error;
copy_str_error:
    fclose(file);
    LOG_ERR("copying string failed at line [%s] in config file [%s], line %d",
            line, cfg_file, lineno);
    goto error;
non_key_value:
    fclose(file);
    LOG_ERR("invalid entry [%s] (not following \"parameter=value\" notation) "
            "in config file [%s], line %d", line, cfg_file, lineno);
    goto error;
incorrect_config:
    fclose(file);
    LOG_ERR("wrong value of [%s] in config file [%s], line %d",
            cfg[i].parameter, cfg_file, lineno);
    goto error;
unknown_parameter:
    fclose(file);
    LOG_ERR("unknown parameter [%s] in config file [%s], line %d",
            parameter, cfg_file, lineno);
    goto error;

missing_mandatory:
    LOG_ERR("missing mandatory parameter [%s] in config file [%s]",
            cfg[i].parameter, cfg_file);
error:
    return FAIL;
}

/**
 * Parse configuration file
 *
 * @param cfg_file
 *   full name of config file
 * @param cfg
 *   pointer to configuration parameter structure
 * @param optional
 *   do not treat missing configuration file as error
 * @param strict
 *   treat unknown parameters as error
 *
 * @return
 *  SUCCEED - parsed successfully
 *  FAIL - error processing config file
 */
int parse_cfg_file(const char *cfg_file, struct cfg_line *cfg, int optional, int strict)
{
    return __parse_cfg_file(cfg_file, cfg, 0, optional, strict);
}
