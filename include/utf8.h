#ifndef SPSPS_UTF8_H_
#define SPSPS_UTF8_H_

/**
 * @file
 * Support for working with UTF-8.  This is based on version 7.0.0 of the
 * Unicode standard.  http://www.unicode.org/versions/Unicode7.0.0/
 *
 * @verbatim
 * SPSPS
 * Stacy's Pathetically Simple Parsing System
 * https://github.com/sprowell/spsps
 *
 * Copyright (c) 2015, Stacy Prowell
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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/// The 32-bit byte order mark (BOM) for this platform.
#define UTF32_BOM (0x0000FEFF)

/// The character type for a single character.
typedef uint32_t utf32_char;

/// The marker type of a UTF-8 encoded string.
typedef char * utf8_string;

/// The marker type of a UTF-32 encoded string.
typedef utf32_char * utf32_string;

/**
 * Determine if the provided code point is an ISO control character.  These are
 * the code points in the closed intervals [U+0000 - U+001F] and
 * [U+007F - U+009F].  Nothing above U+009F is considered an ISO control
 * character.
 *
 * This is not the same definition as is used for the C standard library
 * function iswcntrl.
 *
 * It is the same as Java's Character.isISOControl.
 *
 * @param code_point	The code point to test.
 * @return 				True if the code point is an ISO control character, and
 * 						false otherwise.
 */
bool is_ISO_control(utf32_char code_point);

/**
 * Determine if the provided code point is a whitespace character.
 *
 * There are two references for this.  The first is Table 6-2 in version 7.0.0
 * of the Unicode standard.  The second is the Unicode property list file
 * PropList.txt that is part of the standard.  This method relies solely on
 * the latter.
 *
 * The following are recognized as whitespace.
 *   - U+0009 horizontal tabulation
 *   - U+000A line feed
 *   - U+000B vertical tabulation
 *   - U+000C form feed
 *   - U+000D carriage return
 *   - U+0020 space
 *   - U+00A0 no-break space
 *   - U+1680 ogham space mark
 *   - U+2000 en quad
 *   - U+2001 em quad
 *   - U+2002 en space
 *   - U+2003 em space
 *   - U+2004 three-per-em space
 *   - U+2005 four-per-em space
 *   - U+2006 six-per-em space
 *   - U+2007 figure space
 *   - U+2008 punctuation space
 *   - U+2009 thin space
 *   - U+200A hair space
 *   - U+202F narrow no-break space
 *   - U+205F medium mathematical spacl
 *   - U+3000 ideographic space
 *
 * Note that this ignores the zero width space (U+200B).  U+180E is listed
 * in table 6.2, but is not included in the property list, and is therefore
 * not included here.
 *
 * The iswspace C library function does include the zero-length space.
 *
 * The Java Character.isWhitespace method excludes non-breaking space
 * characters (U+00A0, U+2007, U+202F) and includes record separators
 * (U+001C, U+001D, U+001E, U+001F).
 */
bool is_whitespace(utf32_char code_point);

/**
 * Given a single character, encode it into the sequence of UTF-8 bytes.
 * The input character is UTF-32.
 * 
 * This is based on RFC 3629. (http://tools.ietf.org/html/rfc3629)
 *
 * @param code_point    The input character.
 * @param used          The number of bytes for the encoded character.
 *                      This is zero iff the input code point is invalid.
 *                      This is ignored if it is NULL on input.
 * @return              The null-terminated encoded byte sequence.  This is
 *                      never NULL, and the caller is responsible for
 *                      deallocating it.
 */
utf8_string utf8encode(utf32_char code_point, size_t * used);

/**
 * Given a single Unicode code point (as UTF-32) determine how many bytes
 * are required to encode that code point as UTF-8.  This is the same as
 * `used` from `utf8encode`.
 *
 * @param code_point 	The input character.
 * @return 				The number of bytes needed to encode the code point.
 */
size_t utf8encode_size(utf32_char code_point);

/**
 * Given a sequence of bytes that may or may not be null-terminated,
 * try to convert that sequence of bytes into a single Unicode character.
 * The provided byte sequence is assumed to be at least six bytes in
 * length, so make sure that it is allocated in that manner.  The return
 * value is UTF-32.
 *
 * This is based on RFC 3629. (http://tools.ietf.org/html/rfc3629)
 *
 * @param input     The input bytes.  If NULL on input, nul (0) is returned.
 * @param used      Number of bytes consumed by this action.  This is
 *                  ignored if it is NULL on input.
 * @return          The decoded character.  This will be nul (0) if the
 *                  input byte sequence is empty, and 0xDChh if the next
 *					byte hh is invalid.
 */
utf32_char utf8decode(utf8_string input, size_t * used);

/**
 * Given a sequence of bytes determine how many of those bytes make up the
 * next UTF-8 encoded Unicode code point.  That is, determine how many bytes
 * would be consumed to generate the next code point.  This is used to count
 * code points in a byte sequence.  This is exactly the same as the `used`
 * parameter from `utf8decode`.
 *
 * @param input 	The bytes.
 * @return 			The number of bytes that make up the next UTF-8 character
 * 					(or 1 if the next character is invalid).
 */
size_t utf8decode_size(utf8_string input);

#endif //SPSPS_UTF8_H_
