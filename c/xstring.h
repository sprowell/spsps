#ifndef XSTRING_H_
#define XSTRING_H_

/**
 * @file
 * Public interface to the simple immutable string.
 *
 * @verbatim
 * SPSPS
 * Stacy's Pathetically Simple Parsing System
 * https://github.com/sprowell/spsps
 *
 * Copyright (c) 2014, Stacy Prowell
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * @endverbatim
 */

#include <stddef.h>
#include <wchar.h>

/// Opaque type for a string.
typedef struct xstring_ * xstring;

/// Opaque type for a mutable string.
typedef struct mstring_ * mstring;


/**
 * A macro to print an xstring to the given stream.  The string is
 * printed, but no other action is taken.  Note that this is a
 * statement, not an expression.
 * @param m_stream			An IO stream, such as stdout.
 * @param m_xstring			The string to print.
 */
#define XPRINT(m_stream, m_xstring) { \
	char * cstring = xstr_cstr(m_xstring); \
	fprintf(m_stream, "%s", cstring); \
	fflush(m_stream); \
	free(cstring); \
}

/**
 * A macro to print an mstring to the given stream.  The string is
 * printed, but no other action is taken.  Note that this is a
 * statement, not an expression.
 * @param m_stream			An IO stream, such as stdout.
 * @param m_mstring			The string to print.
 */
#define MPRINT(m_stream, m_mstring) { \
	char * cstring = mstr_cstr(m_mstring); \
	fprintf(m_stream, "%s", cstring); \
	fflush(m_stream); \
	free(cstring); \
}

/// Opaque type for a character.
#ifdef SPSPS_CHAR
typedef SPSPS_CHAR xchar;
#else
typedef char xchar;
#endif

/// Incremental size to use for mutable string.
#ifndef MSTR_INC
#  define MSTR_INC 64
#endif

#ifdef MSTRING_DEBUG
/**
 * Inspect a mstring's internal data.  This is of no use other
 * than for debugging, and you must therefore set the MSTRING_DEBUG
 * flag to see the definition.
 * @param str			The mstring.
 */
void mstr_inspect(mstring str);
#endif

/**
 * Make a new empty and immutable string.
 * @return				The new empty string.
 */
xstring xstr_new();

/**
 * Make a new empty mutable string.  If the capacity is zero, then
 * a default capacity (MSTR_INC) is used.
 * @param capacity		The initial capacity of the string.
 * @return				The new mutable string.
 */
mstring mstr_new(size_t capacity);

/**
 * Free a string.  Be sure to call this method instead of simply
 * calling free on an xstring instance; the latter will cause a
 * memory leak.
 * @param value			The value to free.
 */
void xstr_free(xstring value);

/**
 * Free a mutable string.  Be sure to call this method instead of
 * calling free on a mstring instance; the latter will cause a
 * memory leak.
 * @param value			The value to free.
 */
void mstr_free(mstring value);

/**
 * Convert a C null-terminated string into an xstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call.  The caller should deallocate it.
 * @param value			The null-terminate C string.
 * @return				The new immutable string.
 */
xstring xstr_wrap(char * value);

/**
 * Convert a C null-terminated string into an xstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call, and is explicitly deallocated.
 * @param value			The null-terminate C string.
 * @return				The new immutable string.
 */
xstring xstr_wrap_f(char * value);

/**
 * Convert a C null-terminated string into an mstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call.  The caller should deallocate it.
 * @param value			The null-terminated C string.
 * @return				The new mutable string.
 */
mstring mstr_wrap(char * value);

/**
 * Convert a C null-terminated string into an mstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call, and is explicitly deallocated.
 * @param value			The null-terminated C string.
 * @return				The new mutable string.
 */
mstring mstr_wrap_f(char * value);

/**
 * Convert a C null-terminated string into an xstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call.  The caller should deallocate it.
 * @param value			The null-terminate C string.
 * @return				The new immutable string.
 */
xstring xstr_wwrap(wchar_t * value);

/**
 * Convert a C null-terminated string into an xstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call, and is explicitly deallocated.
 * @param value			The null-terminate C string.
 * @return				The new immutable string.
 */
xstring xstr_wwrap_f(wchar_t * value);

/**
 * Convert a C null-terminated string into an mstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call.  The caller should deallocate it.
 * @param value			The null-terminated C string.
 * @return				The new mutable string.
 */
mstring mstr_wwrap(wchar_t * value);

/**
 * Convert a C null-terminated string into an mstring.  The input
 * C string is converted to the proper characters and is not needed
 * subsequent to this call, and is explicitly deallocated.
 * @param value			The null-terminated C string.
 * @return				The new mutable string.
 */
mstring mstr_wwrap_f(wchar_t * value);

/**
 * Create a copy of the string.  The resulting copy is independent
 * of the original.  The main purpose for this is to allow copies
 * to be passed around, with the recipient assuming responsibility
 * for deallocating its instance.
 * @param other 		The string to copy.
 * @return				The new copy.
 */
xstring xstr_copy(xstring other);

/**
 * Create a copy of the string.  The resulting copy is independent
 * of the original.  The main purpose for this is to allow modification
 * of the original and the copy independently.  Note that the new copy
 * may be more efficient than the original.
 * @param other 		The string to copy.
 * @return				The new copy.
 */
mstring mstr_copy(mstring other);

/**
 * Convert a mutable string to an immutable string.
 * @param other			The mutable string.
 * @return				The immutable string.
 */
xstring mstr_to_xstr(mstring other);

/**
 * Convert an immutable string to a mutable string.
 * @param other			The immutable string.
 * @return				The mutable string.
 */
mstring xstr_to_mstr(xstring other);

/**
 * Obtain the length of the string, in characters.
 * @param value			The string.
 * @return				The number of characters in the string.
 */
size_t xstr_length(xstring value);

/**
 * Obtain the length of the string, in characters.
 * @param value			The string.
 * @return				The number of charactesr in the string.
 */
size_t mstr_length(mstring value);

/**
 * Append a character to the end of the string.  A new string is
 * created (since xstrings are immutable) and returned.  As such,
 * this is not a particularly efficient way to build up a string.
 * @param value			The string.
 * @param ch			The character to add.
 * @return				The new string.
 */
xstring xstr_append(xstring value, xchar ch);

/**
 * Append a character to the end of the string.  This modifies the
 * string in place and - if the string has excess capacity - does
 * not perform any allocation.  The input string is returned.  This
 * is the primary use for the mstring.
 * @param value			The string.
 * @param ch			The character to add.
 * @return				The string.
 */
mstring mstr_append(mstring value, xchar ch);

/**
 * Append a character to the end of the string.  A new string is
 * created (since xstrings are immutable) and returned.  As such,
 * this is not a particularly efficient way to build up a string.
 * The input string is explicitly deallocated.
 * @param value			The string.
 * @param ch			The character to add.
 * @return				The new string.
 */
xstring xstr_append_f(xstring value, xchar ch);

/**
 * Append a C string to the end of the given xstring.  The
 * C string is not stored by this action, and can be deallocated
 * by the caller.  The input string not modified; a new string is
 * returned.
 * @param value			The string.
 * @param cstr			The string to append.
 * @return				The input string, modified.
 */
xstring xstr_append_cstr(xstring value, char * cstr);

/**
 * Append a C string to the end of the given xstring.  The
 * C string is automatically deallocated by this function, as is
 * the input xstring.  A new xstring is allocated and returned.
 * @param value			The string.
 * @param cstr			The string to append.
 * @return				The input string, modified.
 */
xstring xstr_append_cstr_f(xstring value, char * cstr);

/**
 * Append a C string to the end of the given mstring.  The
 * C string is not stored by this action, and can be deallocated
 * by the caller.  The input string is modified and returned.
 * @param value			The string.
 * @param cstr			The string to append.
 * @return				The input string, modified.
 */
mstring mstr_append_cstr(mstring value, char * cstr);

/**
 * Append a C string to the end of the given mstring.  The
 * C string is automatically deallocated by this function.
 * The input string is modified and returned.
 * @param value			The string.
 * @param cstr			The string to append.
 * @return				The input string, modified.
 */
mstring mstr_append_cstr_f(mstring value, char * cstr);

/**
 * Concatenate two strings.  The second string is appended to the
 * end of the first string, creating a new string.  The input
 * strings are not modified, since the are immutable.
 * @param first			The first string.
 * @param second		The second string.
 * @return				The new string.
 */
xstring xstr_concat(xstring first, xstring second);

/**
 * Concatenate two strings.  The second string is appended to the
 * end of the first string, which is modified in-place.  The two
 * strings are used as-is by this operation, and should not be
 * deallocated!  No memory allocation is performed by this method.
 * @param first			The first string.
 * @param second		The second string.
 * @return				The first string, with the second appended.
 */
mstring mstr_concat(mstring first, mstring second);

/**
 * Concatenate two strings.  The second string is appended to the
 * end of the first string, creating a new string.  The input strings
 * are explicitly deallocated.
 * @param first			The first string.
 * @param second		The second string.
 * @return				The new string.
 */
xstring xstr_concat_f(xstring first, xstring second);

/**
 * Obtain a character from the given string.  If the index is out
 * of range of the string, then the null character is returned (0).
 * @param value			The string.
 * @param index			The zero-based index of the character.
 * @return				The requested character.
 */
xchar xstr_char(xstring value, size_t index);

/**
 * Obtain a character from the given string.  If the index is out
 * of range of the string, then the null character is returned (0).
 * @param value			The string.
 * @param index			The zero-based index of the character.
 * @return				The requested character.
 */
xchar mstr_char(mstring value, size_t index);

/**
 * Extract a substring from the given string.  The substring can
 * be empty.  If the start position is out of the string's range,
 * or the number of characters is too large, or both, then the
 * returned string is padded with null characters (0).
 * @param value			The string.
 * @param start			The zero-based index of the first character.
 * @param num			The number of characters to extract.
 * @return				The requested substring.
 */
xstring xstr_substr(xstring value, size_t start, size_t num);

/**
 * Extract a substring from the given string.  The substring can
 * be empty.  If the start position is out of the string's range,
 * or the number of characters is too large, or both, then the
 * returned string is padded with null characters (0).
 * @param value			The string.
 * @param start			The zero-based index of the first character.
 * @param num			The number of characters to extract.
 * @return				The requested substring.
 */
mstring mstr_substr(mstring value, size_t start, size_t num);

/**
 * Extract a substring from the given string.  The substring can
 * be empty.  If the start position is out of the string's range,
 * or the number of characters is too large, or both, then the
 * returned string is padded with null characters (0).  The input
 * string is explicitly deallocated.
 * @param value			The string.
 * @param start			The zero-based index of the first character.
 * @param num			The number of characters to extract.
 * @return				The requested substring.
 */
xstring xstr_substr_f(xstring value, size_t start, size_t num);

/**
 * Extract a substring from the given string.  The substring can
 * be empty.  If the start position is out of the string's range,
 * or the number of characters is too large, or both, then the
 * returned string is padded with null characters (0).  The input
 * string is explicitly deallocated.
 * @param value			The string.
 * @param start			The zero-based index of the first character.
 * @param num			The number of characters to extract.
 * @return				The requested substring.
 */
mstring mstr_substr_f(mstring value, size_t start, size_t num);

/**
 * Compare two strings.  The return value is the standard C
 * comparison.  The value is negative iff lhs is less than
 * rhs.  The value is zero iff they are equal.  The value is
 * positive iff lhs is greater than rhs.  The comparison is
 * done lexicographically.
 * @param lhs			The first string.
 * @param rhs			The second string.
 * @return				The comparison result.
 */
int xstr_strcmp(xstring lhs, xstring rhs);

/**
 * Compare two strings.  The return value is the standard C
 * comparison.  The value is negative iff lhs is less than
 * rhs.  The value is zero iff they are equal.  The value is
 * positive iff lhs is greater than rhs.  The comparison is
 * done lexicographically.
 * @param lhs			The first string.
 * @param rhs			The second string.
 * @return				The comparison result.
 */
int mstr_strcmp(mstring lhs, mstring rhs);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
char * xstr_cstr(xstring value);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.  The input string is explicitly deallocated.
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
char * xstr_cstr_f(xstring value);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
char * mstr_cstr(mstring value);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.  The input string is explicitly deallocated.
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
char * mstr_cstr_f(mstring value);

/**
 * Convert a string into a C null-terminated wide character array.
 * @param value			The string.
 * @return				The null-terminated array of wide characters.
 */
wchar_t * xstr_wcstr(xstring value);

/**
 * Convert a string into a C null-terminated wide character array.
 * The input string is explicitly deallocated.
 * @param value			The string.
 * @return				The null-terminated array of wide characters.
 */
wchar_t * xstr_wcstr_f(xstring value);

/**
 * Convert a string into a C null-terminated wide character array and
 * return it.  The caller is responsible for freeing the returned
 * string.
 * @param value			The string.
 * @return				The null-terminated array of wide chars.
 */
wchar_t * mstr_wcstr(mstring value);

/**
 * Convert a string into a C null-terminated wide character array and
 * return it.  The caller is responsible for freeing the returned
 * string.  The input string is explicitly deallocated.
 * @param value			The string.
 * @return				The null-terminated array of wide chars.
 */
wchar_t * mstr_wcstr_f(mstring value);

#endif /* XSTRING_H_ */
