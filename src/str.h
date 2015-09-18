/*
 * Copyleft
 */

#ifndef STR
#define STR

#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#define SUCCEED             0
#define FAIL               -1

#define MAX_STRING_LEN		2048

/**
 * Copy string, use malloc() to allocate memory, use free() to free it.
 */
char *
str_strdup(const char *str);

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
int
str_rtrim(char *str, const char *charlist);

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
str_ltrim(char *str, const char *charlist);


/**
 * Removes leading and trailing characters from the specified character string

 *
 * @param str
 *   [IN/OUT] string for processing
 * @param charlist
 *   [IN] null terminated list of characters
 */
void
str_lrtrim(char *str, const char *charlist);

/**
 * Dynamical formatted output conversion
 *
 * @return
 *   formatted string, a pointer to allocated memory
 */
char *
str_dvsprintf(char *dest, const char *f, va_list args);

/**
 * Dynamical formatted output conversion
 *
 * @return
 *   formatted string, a pointer to allocated memory
 */
char *
str_dsprintf(char *dest, const char *f, ...);

/**
 * Check UTF-8 sequences
 *
 * @param
 *   text - [IN] pointer to the string
 *
 * @return
 *   SUCCEED if string is valid or FAIL otherwise
 */
int str_is_utf8(const char *text);

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
                    uint64_t min, uint64_t max);

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
int str2uint64(const char *str, const char *suffixes, uint64_t *value);

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
double  str2double(const char *str);

/**
 * Remove whitespace surrounding a string list item delimiters
 *
 * @param
 *   list - the list (a string containing items separated by delimiter)
 * @param
 *   delimiter - the list delimiter
 */
void
str_trim_str_list(char *list, char delimiter);

/**
 * Add a string to dynamic string array
 *
 * @param
 *   arr - a pointer to array of strings
 * @param
 *   entry - string to add
 *
 * @return
 *   SUCCEED if succeed
 *   FAIL if fail
 */
int
str_strarr_add(char ***arr, const char *entry);

/**
 * Initialize dynamic string array
 *
 * @param
 *   arr - a pointer to array of strings
 *
 * @return
 *   SUCCEED if succeed
 *   FAIL if fail
 */
int
str_strarr_init(char ***arr);

#endif /* STR */
