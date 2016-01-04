/*
 * Copyleft
 */

#ifndef CFG_H
#define CFG_H

#include <stdint.h>

#include "str.h"

#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR     '/'
#endif

#define TYPE_INT            0
#define TYPE_STRING         1
#define TYPE_MULTISTRING    2
#define TYPE_UINT64         3
#define TYPE_STRING_LIST    4

#define PARM_OPT            0
#define PARM_MAND           1

/* config file parsing options */
#define CFG_FILE_REQUIRED   0
#define CFG_FILE_OPTIONAL   1

#define CFG_NOT_STRICT      0
#define CFG_STRICT          1

#ifndef S_ISREG
#	define S_ISREG(x) (((x) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISDIR
#	define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#ifndef LOG_ERR
#define LOG_ERR(f, arg...)                                              \
    do {                                                                \
        fprintf(stderr, "["__FILE__"][%s:%d]: "f"\n",                   \
                __func__, __LINE__, ##arg);                             \
    } while (0)
#endif

struct cfg_line {
    const char *parameter;
    void       *variable;
    int         type;
    int         mandatory;
    uint64_t    min;
    uint64_t    max;
};

int parse_cfg_file(const char *cfg_file, struct cfg_line *cfg, int optional,
                   int strict);

#endif /* CFG_H */
