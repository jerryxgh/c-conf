/*
 * Copyleft
 */

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"

char *
str_strdup(const char *str)
{
    size_t ssize = strlen(str) + 1;
    char *tmp = (char *)malloc(ssize);

    if(NULL == tmp) {
        return NULL;
    } else {
        memcpy(tmp, str, ssize);
        return tmp;
    }
}

/**
 * Strip characters from the end of a string
 *
 * @param str
 *   string for processing
 * @param charlist
 *   null terminated list of characters
 *
 * @return
 *   number of trimmed characters
 */
int str_rtrim(char *str, const char *charlist)
{
    char    *p;
    int count = 0;

    if (NULL == str || '\0' == *str)
        return count;

    for (p = str + strlen(str) - 1; p >= str && NULL != strchr(charlist, *p);
         p--) {

        *p = '\0';
        count++;
    }

    return count;
}

/**
 * Strip characters from the beginning of a string
 *
 * @param str
 *   string for processing
 * @param charlist
 *   null terminated list of characters
 *
 */
void
str_ltrim(char *str, const char *charlist)
{
    char    *p;

    if (NULL == str || '\0' == *str)
        return;

    for (p = str; '\0' != *p && NULL != strchr(charlist, *p); p++)
        ;

    if (p == str)
        return;

    while ('\0' != *p)
        *str++ = *p++;

    *str = '\0';
}

/**
 * Removes leading and trailing characters from the specified character string

 *
 * @param str
 *   [IN/OUT] string for processing
 * @param charlist
 *   [IN] null terminated list of characters
 */
void
str_lrtrim(char *str, const char *charlist)
{
    str_rtrim(str, charlist);
    str_ltrim(str, charlist);
}

/**
 * Dynamical formatted output conversion
 *
 * @return
 *   formatted string, a pointer to allocated memory
 */
char *
str_dvsprintf(char *dest, const char *f, va_list args)
{
    char *string = NULL;
    int n, size = MAX_STRING_LEN >> 1;

    va_list curr;

    while (1) {
        string = malloc(size);

        va_copy(curr, args);
        n = vsnprintf(string, size, f, curr);
        va_end(curr);

        if (0 <= n && n < size)
            break;

        /* result was truncated */
        if (-1 == n)
            size = size * 3 / 2 + 1;    /* the length is unknown */
        else
            size = n + 1;   /* n bytes + trailing '\0' */

        free(string);
    }

    free(dest);

    return string;
}

/**
 * Dynamical formatted output conversion
 *
 * @return
 *   formatted string, a pointer to allocated memory
 */
char *
str_dsprintf(char *dest, const char *f, ...)
{
    char    *string;
    va_list args;

    va_start(args, f);

    string = str_dvsprintf(dest, f, args);

    va_end(args);

    return string;
}

/**
 * Check UTF-8 sequences
 *
 * @param
 *   text - [IN] pointer to the string
 *
 * @return
 *   SUCCEED if string is valid or FAIL otherwise
 */
int str_is_utf8(const char *text)
{
    size_t i, mb_len, expecting_bytes = 0;
    const unsigned char *utf8;
    unsigned int utf32;

    while ('\0' != *text) {
        /* single ASCII character */
        if (0 == (*text & 0x80)) {
            text++;
            continue;
        }

        /* unexpected continuation byte or invalid UTF-8 bytes '\xfe' & '\xff' */
        if (0x80 == (*text & 0xc0) || 0xfe == (*text & 0xfe))
            return FAIL;

        /* multibyte sequence */

        utf8 = (const unsigned char *)text;

        if (0xc0 == (*text & 0xe0))     /* 2-bytes multibyte sequence */
            expecting_bytes = 1;
        else if (0xe0 == (*text & 0xf0))    /* 3-bytes multibyte sequence */
            expecting_bytes = 2;
        else if (0xf0 == (*text & 0xf8))    /* 4-bytes multibyte sequence */
            expecting_bytes = 3;
        else if (0xf8 == (*text & 0xfc))    /* 5-bytes multibyte sequence */
            expecting_bytes = 4;
        else if (0xfc == (*text & 0xfe))    /* 6-bytes multibyte sequence */
            expecting_bytes = 5;

        mb_len = expecting_bytes + 1;
        text++;

        for (; 0 != expecting_bytes; expecting_bytes--) {
            /* not a continuation byte */
            if (0x80 != (*text++ & 0xc0))
                return FAIL;
        }

        /* overlong sequence */
        if (0xc0 == (utf8[0] & 0xfe) ||
            (0xe0 == utf8[0] && 0x00 == (utf8[1] & 0x20)) ||
            (0xf0 == utf8[0] && 0x00 == (utf8[1] & 0x30)) ||
            (0xf8 == utf8[0] && 0x00 == (utf8[1] & 0x38)) ||
            (0xfc == utf8[0] && 0x00 == (utf8[1] & 0x3c))) {
            return FAIL;
        }

        utf32 = 0;

        if (0xc0 == (utf8[0] & 0xe0))
            utf32 = utf8[0] & 0x1f;
        else if (0xe0 == (utf8[0] & 0xf0))
            utf32 = utf8[0] & 0x0f;
        else if (0xf0 == (utf8[0] & 0xf8))
            utf32 = utf8[0] & 0x07;
        else if (0xf8 == (utf8[0] & 0xfc))
            utf32 = utf8[0] & 0x03;
        else if (0xfc == (utf8[0] & 0xfe))
            utf32 = utf8[0] & 0x01;

        for (i = 1; i < mb_len; i++) {
            utf32 <<= 6;
            utf32 += utf8[i] & 0x3f;
        }

        /* according to the Unicode standard the high and low
         * surrogate halves used by UTF-16 (U+D800 through U+DFFF)
         * and values above U+10FFFF are not legal
         */
        if (utf32 > 0x10ffff || 0xd800 == (utf32 & 0xf800))
            return FAIL;
    }

    return SUCCEED;
}

/**
 * Check if the string is unsigned integer within the specified range and
 *          optionally store it into value parameter
 *
 * @param
 *   str   - [IN] string to check
 * @param n
 *   [IN] string length or MAX_UINT64_LEN
 * @param value
 * - [OUT] a pointer to output buffer where the converted value is to be written
 *          (optional, can be NULL)
 * @param size
 *   [IN] size of the output buffer (optional)
 * @param min
 *   [IN] the minimum acceptable value
 * @param max
 *   [IN] the maximum acceptable value
 *
 * @return
 *   SUCCEED - the string is unsigned integer
 *   FAIL - the string is not a number or its value is outside the specified
 *           range
 *
 */
int is_uint_n_range(const char *str, size_t n, void *value, size_t size,
                    uint64_t min, uint64_t max)
{
    const uint64_t  max_uint64 = ~(uint64_t)__UINT64_C(0);
    uint64_t value_uint64 = 0, c;
    unsigned short value_offset;
    int len = 0;

    if ('\0' == *str || 0 == n || sizeof(uint64_t) < size
        || (0 == size && NULL != value)) {

        return FAIL;
    }

    while ('\0' != *str && 0 < n--) {
        if (0 == isdigit(*str))
            return FAIL;    /* not a digit */

        c = (uint64_t)(unsigned char)(*str - '0');

        if (20 <= ++len && (max_uint64 - c) / 10 < value_uint64)
            return FAIL;    /* maximum value exceeded */

        value_uint64 = value_uint64 * 10 + c;

        str++;
    }

    if (min > value_uint64 || value_uint64 > max)
        return FAIL;

    if (NULL != value) {
        /* On little endian architecture the output value will be stored
           starting from the first bytes of 'value' buffer while on big endian
           architecture it will be stored starting from the last bytes. We handle
           it by storing the offset in the most significant byte of short value
           and then use the first byte as source offset.  */

        value_offset = (unsigned short)((sizeof(uint64_t) - size) << 8);

        memcpy(value,
               (unsigned char *)&value_uint64 + *((unsigned char *)&value_offset),
               size);
    }

    return SUCCEED;
}

#define STR_KIBIBYTE        1024
#define STR_MEBIBYTE        1048576
#define STR_GIBIBYTE        1073741824
#define STR_TEBIBYTE        __UINT64_C(1099511627776)

#define SEC_PER_MIN         60
#define SEC_PER_HOUR        3600
#define SEC_PER_DAY         86400
#define SEC_PER_WEEK       (7 * SEC_PER_DAY)
#define SEC_PER_MONTH      (30 * SEC_PER_DAY)
#define SEC_PER_YEAR       (365 * SEC_PER_DAY)

#define is_uint64_n(str, n, value)                                      \
    is_uint_n_range(str, n, value, 8, 0x0, __UINT64_C(0xFFFFFFFFFFFFFFFF))

static uint64_t
suffix2factor(char c)
{
    switch (c)
    {
    case 'K':
        return STR_KIBIBYTE;
    case 'M':
        return STR_MEBIBYTE;
    case 'G':
        return STR_GIBIBYTE;
    case 'T':
        return STR_TEBIBYTE;
    case 's':
        return 1;
    case 'm':
        return SEC_PER_MIN;
    case 'h':
        return SEC_PER_HOUR;
    case 'd':
        return SEC_PER_DAY;
    case 'w':
        return SEC_PER_WEEK;
    default:
        return 1;
    }
}

/**
 * Convert string to 64bit unsigned integer
 *
 * @param
 *   str   - string to convert
 *   value - a pointer to converted value
 *
 * @return
 *   SUCCEED - the string is unsigned integer
 *   FAIL - otherwise
 *
 * @comments
 *   the function automatically processes suffixes K, M, G, T
 */
int str2uint64(const char *str, const char *suffixes, uint64_t *value)
{
    uint64_t factor = 1;
    const char *p;
    size_t sz;
    int ret;

    sz = strlen(str);
    p = str + sz - 1;

    if (NULL != strchr(suffixes, *p)) {
        factor = suffix2factor(*p);
        sz--;
    }

    if (SUCCEED == (ret = is_uint64_n(str, sz, value)))
        *value *= factor;

    return ret;
}

/**
 * Convert string to double
 *
 * @param
 *   str - string to convert
 *
 * @return
 *   converted double value
 *
 * @comments
 *  the function automatically processes suffixes K, M, G, T and s, m, h, d, w
 */
double  str2double(const char *str)
{
    size_t  sz;

    sz = strlen(str) - 1;

    return atof(str) * suffix2factor(str[sz]);
}

/**
 * Remove whitespace surrounding a string list item delimiters
 *
 * @param
 *   list - the list (a string containing items separated by delimiter)
 * @param
 *   delimiter - the list delimiter
 */
void
str_trim_str_list(char *list, char delimiter)
{
    /* NB! strchr(3): "terminating null byte is considered part of the string" */
    char whitespace[] = " \t";
    char *out, *in;

    if (NULL == list || '\0' == *list)
        return;

    out = in = list;

    while ('\0' != *in) {
        /* trim leading spaces from list item */
        while ('\0' != *in && NULL != strchr(whitespace, *in))
            in++;

        /* copy list item */
        while (delimiter != *in && '\0' != *in)
            *out++ = *in++;

        /* trim trailing spaces from list item */
        if (out > list)
        {
            while (NULL != strchr(whitespace, *(--out)))
                ;
            out++;
        }
        if (delimiter == *in)
            *out++ = *in++;
    }
    *out = '\0';
}

int
str_strarr_add(char ***arr, const char *entry)
{
    int i;

    for (i = 0; NULL != (*arr)[i]; i++)
        ;

    *arr = realloc(*arr, sizeof(char **) * (i + 2));

    if (NULL == *arr) {
        return FAIL;
    }

    (*arr)[i] = str_strdup(entry);
    if (NULL == (*arr)[i]) {
        return FAIL;
    }

    (*arr)[++i] = NULL;

    return SUCCEED;
}

int
str_strarr_init(char ***arr)
{
    *arr = malloc(sizeof(char **));
    if (NULL == *arr) {
        return FAIL;
    }

    **arr = NULL;

    return SUCCEED;
}
