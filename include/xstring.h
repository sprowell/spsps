#ifndef SPSPS_XSTRING_H_
#define SPSPS_XSTRING_H_

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
#include <stdbool.h>
#include "utf8.h"

/// Opaque type for a string.
typedef struct xstring_ * xstring;

/// Opaque type for a mutable string.
typedef struct mstring_ * mstring;

/// Opaque type for an xstring character iterator.
typedef struct xstr_iter_ * xstr_iter;

/// Opaque type for an mstring character iterator.
typedef struct mstr_iter_ * mstr_iter;


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

/// Incremental size to use for mutable string.  To override this \#define
/// it prior to inclusion.
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
 * Make a new empty and immutable string.  You should never need to
 * use this, since NULL is a perfectly valid string, and the system
 * will defer allocation until needed.
 * @return				The new empty string.
 */
xstring xstr_new();

/**
 * Make a new empty mutable string.  If the capacity is zero, then
 * a default capacity (MSTR_INC) is used.  O(1).  Note that the
 * capacity is specified in bytes, not characters, due to the UTF-8
 * encoding.
 * @param capacity		The initial capacity of the string.
 * @return				The new mutable string.
 */
mstring mstr_new(size_t capacity);

/**
 * Free a string.  Be sure to call this method instead of simply
 * calling free on an xstring instance; the latter will cause a
 * memory leak.  O(1).
 * @param value			The value to free.
 */
void xstr_free(xstring value);

/**
 * Free a mutable string.  Be sure to call this method instead of
 * calling free on a mstring instance; the latter will cause a
 * memory leak.  O(C*len(value)) because all blocks of the string
 * must be deallocated.
 * @param value			The value to free.
 */
void mstr_free(mstring value);

/**
 * Convert a C null-terminated string into an xstring.  The input
 * C string is not needed subsequent to this call.  The caller should
 * deallocate it.  Note that the string is assumed to be UTF-8.
 * O(len(value)) because each character must be copied.
 * @param value			The null-terminated UTF-8 C string.
 * @return				The new immutable string.
 */
xstring xstr_wrap(utf8_string value);

/**
 * Convert a C null-terminated string into an xstring.  The input
 * C string is not needed subsequent to this call, and is explicitly
 * deallocated.  Note that the string is assumed to be UTF-8.
 * O(len(value)) because each character must be copied.
 * @param value			The null-terminated UTF-8 C string.
 * @return				The new immutable string.
 */
xstring xstr_wrap_f(utf8_string value);

/**
 * Convert a C null-terminated string into an mstring.  The input
 * C string is not needed subsequent to this call.  The caller should
 * deallocate it.  Note that the string is assumed to be UTF-8.
 * O(len(value)) because each character must be copied.
 * @param value			The null-terminated C string.
 * @return				The new mutable string.
 */
mstring mstr_wrap(utf8_string value);

/**
 * Convert a C null-terminated string into an mstring.  The input
 * C string is not needed subsequent to this call, and is explicitly
 * deallocated.  Note that the string is assumed to be UTF-8.
 * O(len(value)) because each character must be copied.
 * @param value			The null-terminated C string.
 * @return				The new mutable string.
 */
mstring mstr_wrap_f(utf8_string value);

/**
 * Create a copy of the string.  The resulting copy is independent
 * of the original.  The main purpose for this is to allow copies
 * to be passed around, with the recipient assuming responsibility
 * for deallocating its instance.  O(len(other)) using memcopy.
 * @param other 		The string to copy.
 * @return				The new copy.
 */
xstring xstr_copy(xstring other);

/**
 * Create a copy of the string.  The resulting copy is independent
 * of the original.  The main purpose for this is to allow modification
 * of the original and the copy independently.  Note that the new copy
 * may be more efficient than the original.  O(len(other)) because each
 * block must be memcopy'd.
 * @param other 		The string to copy.
 * @return				The new copy.
 */
mstring mstr_copy(mstring other);

/**
 * Convert a mutable string to an immutable string.  O(len(other)).
 * @param other			The mutable string.
 * @return				The immutable string.
 */
xstring mstr_to_xstr(mstring other);

/**
 * Convert an immutable string to a mutable string.  O(len(other)).
 * @param other			The immutable string.
 * @return				The mutable string.
 */
mstring xstr_to_mstr(xstring other);

/**
 * Obtain the length of the string, in bytes.  O(1).
 * @param value			The string.
 * @return				The number of bytes in the string.
 */
size_t xstr_length(xstring value);

/**
 * Obtain the length of the string, in bytes.  O(1).
 * @param value			The string.
 * @return				The number of bytes in the string.
 */
size_t mstr_length(mstring value);

/**
 * Append a character to the end of the string.  A new string is
 * created (since xstrings are immutable) and returned.  As such,
 * this is not a particularly efficient way to build up a string.
 * O(len(value)) because of memcopy.
 * @param value			The string.
 * @param ch			The character to add.
 * @return				The new string.
 */
xstring xstr_append(xstring value, utf32_char ch);

/**
 * Append a character to the end of the string.  This modifies the
 * string in place and - if the string has excess capacity - does
 * not perform any allocation.  The input string is returned.  This
 * is the primary use for the mstring.  O(C*len(value)) because the
 * last block must be located.
 * @param value			The string.
 * @param ch			The character to add.
 * @return				The string.
 */
mstring mstr_append(mstring value, utf32_char ch);

/**
 * Append a character to the end of the string.  A new string is
 * created (since xstrings are immutable) and returned.  As such,
 * this is not a particularly efficient way to build up a string.
 * The input string is explicitly deallocated.  O(len(value)) because
 * of memcopy.
 * @param value			The string.
 * @param ch			The character to add.
 * @return				The new string.
 */
xstring xstr_append_f(xstring value, utf32_char ch);

/**
 * Append a C string to the end of the given xstring.  The
 * C string is not stored by this action, and can be deallocated
 * by the caller.  The input string not modified; a new string is
 * returned.  O(len(value)+len(cstr)).
 * @param value			The string.
 * @param cstr			The string to append, assumed to be UTF-8.
 * @return				The input string, modified.
 */
xstring xstr_append_cstr(xstring value, utf8_string cstr);

/**
 * Append a C string to the end of the given xstring.  The
 * C string is automatically deallocated by this function, as is
 * the input xstring.  A new xstring is allocated and returned.
 * Because the input C string is deallocated, you should not use
 * string literals.  O(len(value)+len(cstr)).
 * @param value			The string.
 * @param cstr			The string to append, assumed to be UTF-8.
 * @return				The input string, modified.
 */
xstring xstr_append_cstr_f(xstring value, utf8_string cstr);

/**
 * Append a C string to the end of the given mstring.  The
 * C string is not stored by this action, and can be deallocated
 * by the caller.  The input string is modified and returned.
 * O(C*len(value)+len(cstr)) because the last block must be found
 * and the C string converted and copied.
 * @param value			The string.
 * @param cstr			The string to append, assumed to be UTF-8.
 * @return				The input string, modified.
 */
mstring mstr_append_cstr(mstring value, utf8_string cstr);

/**
 * Append a C string to the end of the given mstring.  The
 * C string is automatically deallocated by this function.
 * The input string is modified and returned.  Because the C string
 * is deallocated, you should not use string literals.
 * O(C*len(value)+len(cstr)) because the last block must be found and
 * the C string converted and copied.
 * @param value			The string.
 * @param cstr			The string to append, assumed to be UTF-8.
 * @return				The input string, modified.
 */
mstring mstr_append_cstr_f(mstring value, utf8_string cstr);

/**
 * Concatenate two strings.  The second string is appended to the
 * end of the first string, creating a new string.  The input
 * strings are not modified, since the are immutable.
 * O(len(first)+len(second)).
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
 * O(C*len(first)) because only the last block of the first string
 * need be located.
 * @param first			The first string.
 * @param second		The second string.
 * @return				The first string, with the second appended.
 */
mstring mstr_concat(mstring first, mstring second);

/**
 * Concatenate two strings.  The second string is appended to the
 * end of the first string, creating a new string.  The input strings
 * are explicitly deallocated.  O(len(first)+len(second)).
 * @param first			The first string.
 * @param second		The second string.
 * @return				The new string.
 */
xstring xstr_concat_f(xstring first, xstring second);

/**
 * Compare two strings.  The return value is the standard C
 * comparison.  The value is negative iff lhs is less than
 * rhs.  The value is zero iff they are equal.  The value is
 * positive iff lhs is greater than rhs.  The comparison is
 * done lexicographically.  O(max(len(lhs),len(rhs))).
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
 * done lexicographically.  O(C*max(len(lhs),len(rhs))).
 * @param lhs			The first string.
 * @param rhs			The second string.
 * @return				The comparison result.
 */
int mstr_strcmp(mstring lhs, mstring rhs);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.  O(len(value)).
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
utf8_string xstr_cstr(xstring value);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.  The input string is explicitly deallocated.  O(len(value)).
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
utf8_string xstr_cstr_f(xstring value);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.  O(len(value)).
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
utf8_string mstr_cstr(mstring value);

/**
 * Convert a string into a C null-terminated character array and
 * return it.  The caller is responsible for freeing the returned
 * string.  The input string is explicitly deallocated.  O(len(value)).
 * @param value			The string.
 * @return				The null-terminated array of chars.
 */
utf8_string mstr_cstr_f(mstring value);

/**
 * Decode the internal UTF-8 representation of the string into a
 * sequence of Unicode code points.
 * @param value		The string to decode.
 * @param length	Number of code points in the returned array.
 * 					This is ignored if it is NULL.
 * @return			The null-terminated sequence of code points.
 */
utf32_string xstr_decode(xstring value, size_t * length);

/**
 * Decode the internal UTF-8 representation of the string into a
 * sequence of Unicode code points.
 * @param value		The string to decode.
 * @param length	Number of code points in the returned array.
 * 					This is ignored if it is NULL.
 * @return			The null-terminated sequence of code points.
 */
utf32_string mstr_decode(mstring value, size_t * length);

/**
 * Decode the internal UTF-8 representation of the string into a
 * sequence of Unicode code points.  The input string is explicitly
 * deallocated.
 * @param value		The string to decode.
 * @param length	Number of code points in the returned array.
 * 					This is ignored if it is NULL.
 * @return			The null-terminated sequence of code points.
 */
utf32_string xstr_decode_f(xstring value, size_t * length);

/**
 * Decode the internal UTF-8 representation of the string into a
 * sequence of Unicode code points.  The input string is explicitly
 * deallocated.
 * @param value		The string to decode.
 * @param length	Number of code points in the returned array.
 * 					This is ignored if it is NULL.
 * @return			The null-terminated sequence of code points.
 */
utf32_string mstr_decode_f(mstring value, size_t * length);

/**
 * Encode the sequence of Unicode code points into a string.
 * @param value		The sequence of code points.  If this is NULL,
 * 					it is treated as the empty sequence, and length
 * 					is ignored.
 * @param length	The length of the provided sequence.
 * @return			The string.
 */
xstring xstr_encode(utf32_string value, size_t length);

/**
 * Encode the sequence of Unicode code points into a string.
 * @param value		The sequence of code points.  If this is NULL,
 * 					it is treated as the empty sequence, and length
 * 					is ignored.
 * @param length	The length of the provided sequence.
 * @return			The string.
 */
mstring mstr_encode(utf32_string value, size_t length);

/**
 * Encode the sequence of Unicode code points into a string.
 * The input sequence is explicitly freed.
 * @param value		The sequence of code points.  If this is NULL,
 * 					it is treated as the empty sequence, and length
 * 					is ignored.
 * @param length	The length of the provided sequence.
 * @return			The string.
 */
xstring xstr_encode_f(utf32_string value, size_t length);

/**
 * Encode the sequence of Unicode code points into a string.
 * The input sequence is explicitly freed.
 * @param value		The sequence of code points.  If this is NULL,
 * 					it is treated as the empty sequence, and length
 * 					is ignored.
 * @param length	The length of the provided sequence.
 * @return			The string.
 */
mstring mstr_encode_f(utf32_string value, size_t length);

/**
 * Obtain an iterator over the code points in a string.  When finished with
 * the iterator, it must be freed.
 * @param xstr 		The string.
 * @return 			The iterator.
 */
xstr_iter xstr_iterator(xstring xstr);

/**
 * Obtain an iterator over the code points in a string.  When finished with
 * the iterator, it must be freed.
 * @param mstr 		The string.
 * @return 			The iterator.
 */
mstr_iter mstr_iterator(mstring mstr);

/**
 * Determine if the string iterator has further code points.
 * @param iter 		The string iterator.
 * @return 			True if there are more code points; false otherwise.
 */
bool xstr_has_next(xstr_iter iter);

/**
 * Determine if the string iterator has further code points.
 * @param iter 		The string iterator.
 * @return 			True if there are more code points; false otherwise.
 */
bool mstr_has_next(mstr_iter iter);

/**
 * Obtain the next code point from the iterator.
 * @param iter 		The string iterator.
 * @return 			The next code point.  Return null if there are no further
 * 					code points.
 */
utf32_char xstr_next(xstr_iter iter);

/**
 * Obtain the next code point from the iterator.
 * @param iter 		The string iterator.
 * @return 			The next code point.  Return null if there are no further
 * 					code points.
 */
utf32_char mstr_next(mstr_iter iter);

#endif /* SPSPS_XSTRING_H_ */
