#ifndef SPSPS_PARSER_H_
#define SPSPS_PARSER_H_

/**
 * @file
 * The main public interface to the SPSPS library.
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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "utf8.h"
#ifndef SPSPS_READ
 	#define NEED_STDIO
#endif
#ifndef SPSPS_ERROR
 	#define NEED_STDIO
#endif
#ifdef NEED_STDIO
	#include <stdio.h>
#endif

/// This is the method used to read bytes.  By default it uses `fread` and
/// requires `stdio.h`.  If you `\#define` this prior to inclusion, then you
/// can override it with whatever you want.
///
/// It must have the required argument structure, and must return the number
/// of bytes successfully read.  That is, it must behave like `fread`.
///
/// If you define `SPSPS_READ`, then you must also define `SPSPS_FILE` to be
/// whatever you want passed along as the last argument to `SPSPS_READ`.  If
/// you don't need it, `/#define` it to `void` and pass `NULL` for the `stream`
/// argument to `spsps_new`.
#ifndef SPSPS_READ
	#define SPSPS_READ(m_dest, m_size, m_number, m_source) \
 		fread(m_dest, m_size, m_number, m_source)
 	#define SPSPS_FILE FILE
#endif
#ifndef SPSPS_FILE
 	#error You defined SPSPS_READ, but did not define SPSPS_FILE.
#endif

/// The number of bytes to read at once.  This is also the lookahead limit.
/// To override this value \#define it prior to inclusion.  Note that this
/// is a byte-based value, not a character-based value.
#ifndef SPSPS_LOOK
    #define SPSPS_LOOK (4096)
#endif

/// The end of file marker.  Note that this cannot be 0x3 or 0x4 because
/// these are valid code points.  So instead we use this value, which is
/// not a valid code point.
#define SPSPS_EOF 0xFFFFFFFF

// If you #define SPSPS_SHORTHAND, then you get shorter names for the methods
// that might conflict with other names.  It's up to you.
#ifdef SPSPS_SHORTHAND
#define consume()				spsps_consume()
#define consume_n(m_n)			spsps_consume_n(m_n)
#define consume_whitespace()	spsps_consume_whitespace()
#define eof()					spsps_eof()
#define loc()					spsps_loc()
#define peek()					spsps_peek()
#define peek_n(m_n)				spsps_peek_n(parser, m_n)
#define peek_str(m_str)			spsps_peek_str(parser, m_str)
#define peek_and_consume(m_str)	spsps_peek_and_consume(parser, m_str)
#endif

/**
 * A structure to communicate the current position in the input stream.
 */
typedef struct spsps_loc_ {
	/**
	 * The name of the source.  This pointer is shared with the parser,
	 * so do not deallocate it.
	 */
	char * name;

	/** The line number of the next character to read. */
	uint32_t line;

	/** The column number of the next character to read. */
	uint32_t column;
} Loc;

/**
 * Error codes returned by the parser.
 */
typedef enum spsps_errno {
	/// No errors.
	OK = 0,
	/// Lookahead past the SPSPS_LOOK limit.
	LOOKAHEAD_TOO_LARGE,
	/// The parser has likely stalled at the end of file.  This indicates that
	/// too many attempts have been made to peek at the next character after
	/// the end of file has been reached.
	STALLED_AT_EOF,
	/// The parser has likely stalled.  This indicates too many peeks at the
	/// next character without anything ever being consumed.
	STALLED
} spsps_errno;

/**
 * Convert this location to a short string.  The caller assumes the
 * responsibility for deallocating the returned string.
 * @return 		This location as a string.
 */
char * spsps_loc_to_string(Loc * loc);

/// The destination for error messages.  To override this \#define it prior to
/// inclusion.  It must specify an open FILE* destination.  By default this
/// will be stderr.
#ifndef SPSPS_STDERR
	#define SPSPS_STDERR (stderr)
#endif

/// This is the method used to write an error message.  By default it uses
/// `fprintf`, writes to `SPSPS_STDERR`, and requires `stdio.h`.  If you
/// `\#define` this prior to inclusion, then you may override it with whatever
/// you want.  Arguments must look like `printf` (a format string followed by
/// arguments).
///
/// Note that `SPSPS_ERR` calls this, and that is the method you want to use
/// to report an error from inside your parser, not this one!
///
/// @param m_msg			The message (a format string) plus arguments.
#ifndef _SPSPS_ERROR
 	#define _SPSPS_ERROR(m_msg, ...) \
 		fprintf(SPSPS_STDERR, m_msg, ##__VA_ARGS__)
#endif

/**
 * Print an error message from a parser.  The message is sent to the
 * SPSPS_STDERR stream.  If the parser is not NULL, then the location is
 * obtained and printed.  If the message is not NULL, then it is printed
 * (as a format string) and subsequent arguments are the arguments to the
 * format string.  If you wish to use a different stream, either redirect
 * standard error, or \#define SPSPS_STDERR to your stream.
 *
 * @param m_parser			The parser.
 * @param m_msg				The message (a format string) plus arguments.
 */
#define SPSPS_ERR(m_parser, m_msg, ...) { \
	if ((m_parser) != NULL) { \
		Loc * loc = spsps_loc(m_parser); \
		if ((m_msg) != NULL) { \
			_SPSPS_ERROR("ERROR %s:%d:%d: " m_msg "\n", (loc)->name, \
					(loc)->line, (loc)->column, ## __VA_ARGS__); \
		} else { \
			_SPSPS_ERROR("ERROR %s:%d:%d: Unspecified error.\n", \
					(loc)->name, (loc)->line, (loc)->column); \
		} \
		free(loc); \
	} else { \
		if ((m_msg) != NULL) { \
			_SPSPS_ERROR("ERROR: " m_msg "\n", ## __VA_ARGS__); \
		} else { \
			_SPSPS_ERROR("ERROR: Unspecified error.\n"); \
		} \
	} \
}

/**
 * A parser object is an opaque pointer.
 */
typedef struct spsps_parser_ * Parser;

/**
 * Format and return a string representation of the given Unicode code point.
 * It is assumed that the provided character is valid.  The return value will
 * have the format "U+HHHH (c)", where HHHH is the four-digit (for BMP),
 * five-digit, or six-digit code point hexadecimal value.  (Note: At the time
 * of writing, this is the standard way to represent Unicode code points.)
 *
 * If the character is not an ISO control character (the code points U+0000
 * through U+001F and U+007F through U+009F, inclusive) then it is considered
 * printable, and it is printed in place of the c.  Otherwise the " (c)" is
 * omitted.  Note this is not perfect, as it ignores combining code points and
 * the current locale.
 *
 * If the input character is not a valid Unicode code point, then the results
 * are unpredictable.
 *
 * @param xch			The character.
 * @return				The formatted display for the character.
 */
char * spsps_printchar(utf32_char xch);

/**
 * Create a new parser instance.  The caller is responsible for freeing the
 * returned parser instance by calling spsps_free.  The provided file name is
 * copied to new allocated memory, and the caller is responsible for freeing
 * the first argument, if necessary.
 *
 * @param name 			The name of the stream.  Typically a file name.
 *                      If the name is NULL, then the default name "(console)"
 *                      is used.
 * @param stream 		A stream to parse.  If the stream is NULL, then the
 *                      stream is treated as if it were empty.  This is given
 *						as a pointer to a "thing" that makes sense to whatever
 *						implementation of `SPSPS_READ` is used.  If you have
 * 						not overwritten `SPSPS_READ`, then it must be a
 *						`FILE` pointer.
 * @return 				The new parser instance.
 */
Parser spsps_new(char * name, FILE * stream);

/**
 * Free a parser instance previously created with spsps_new.
 * @param parser 		The parser to free.
 */
void spsps_free(Parser parser);

/**
 * Consume and return the next character in the stream.
 * @param parser 		The parser.
 * @return 				The next character in the stream.
 */
utf32_char spsps_consume(Parser parser);

/**
 * Consume and discard the next few characters from the stream.  Note that this
 * may be more efficient than calling spsps_consume if you just want to consume
 * and discard the next character, since it does not perform UTF-8 decoding.
 * @param parser 		The parser.
 * @param n 			The number of characters to discard.
 */
void spsps_consume_n(Parser parser, size_t n);

/**
 * Consume and discard all whitespace.  When this method returns the next
 * character is the first non-whitespace character, or the end of file has
 * been reached.  This uses the Unicode definition of whitespace.
 * @param parser 		The parser.
 */
void spsps_consume_whitespace(Parser parser);

/**
 * Determine if the end of file has been consumed.
 * @param parser 		The parser.
 * @return 				True iff the end of file has been consumed.
 */
bool spsps_eof(Parser parser);

/**
 * Get the current location in the stream.  This is the location of the next
 * character to be read, unless the end of stream has been reached.  The caller
 * is responsible for freeing the returned location via free.
 * @param parser		The parser.
 * @return				The location of the next character to be read.
 */
Loc * spsps_loc(Parser parser);

/**
 * Peek and return the next character in the stream.  The character is not
 * consumed.
 * @param parser		The parser.
 * @return				The next character.
 */
utf32_char spsps_peek(Parser parser);

/**
 * Peek ahead at the next few characters in the stream, and return them.  The
 * return value is a string, so nulls may cause an issue.  The number of
 * characters must be below the lookahead limit.  End of file causes the
 * returned string to be populated by EOF characters.  The caller is
 * responsible for freeing the returned fixed-length string.
 * @param parser		The parser.
 * @param n				The number of characters to look ahead.
 * @return				The next characters.
 */
utf8_string spsps_peek_n(Parser parser, size_t n);

/**
 * Peek ahead and determine if the next characters in the stream are the given
 * characters, in sequence.  That is, the given string must be the next thing
 * in the stream.
 * @param parser		The parser
 * @param next			The characters.
 * @return				True iff the stream contains the given string next.
 */
bool spsps_peek_str(Parser parser, utf8_string next);

/**
 * Peek ahead at the next few characters and if they are a given string, then
 * consume them.  Otherwise leave the stream unchanged.
 * @param parser 		The parser.
 * @param next 			The characters to check for.
 * @return 				False iff the stream was left unchanged.
 */
bool spsps_peek_and_consume(Parser parser, utf8_string next);

#endif /* SPSPS_PARSER_H_ */
