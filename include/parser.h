#ifndef PARSER_H_
#define PARSER_H_

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
#include <stdio.h>
#include <stdlib.h>

/// The number of characters to read at once.  This is also the lookahead limit.
/// To override this value \#define it prior to inclusion.
#ifndef SPSPS_LOOK
    #define SPSPS_LOOK (4096)
#endif

/// The kind of character to parse.  To override this \#define it prior to
/// inclusion.
#ifndef SPSPS_CHAR
 	#define SPSPS_CHAR char
#endif

/// The end of file marker.
#define SPSPS_EOF ((SPSPS_CHAR)-1)

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
	/// The parser has likely stalled at the end of file.
	STALLED_AT_EOF,
	/// The parser has likely stalled.
	STALLED
} spsps_errno;

/**
 * Convert this location to a short string.  The caller assumes the
 * responsibility for deallocating the returned string.
 * @return 		This location as a string.
 */
char * spsps_loc_to_string(Loc * loc);

/// The destination for error messages.  To override this \#define it prior to
/// inclusion.  It must specify an open FILE* destination.
#ifndef SPSPS_STDERR
	#define SPSPS_STDERR stderr
#endif

/**
 * Print an error message from a parser.  The message is sent to the
 * standard error stream.  If the parser is not NULL, then the location is
 * obtained and printed.  If the message is not NULL, then it is printed
 * (as a format string) and subsequent arguments are the arguments to the
 * format string.  If you wish to use a different stream, either redirect
 * standard error, or \#define SPSPS_STDERR to your stream.
 * @param m_parser			The parser.
 * @param m_msg				The message (a format string) plus arguments.
 */
#define SPSPS_ERR(m_parser, m_msg, ...) { \
	if ((m_parser) != NULL) { \
		Loc * loc = spsps_loc(m_parser); \
		if ((m_msg) != NULL) { \
			fprintf(SPSPS_STDERR, "ERROR %s:%d:%d: " m_msg "\n", (loc)->name, \
					(loc)->line, (loc)->column, ## __VA_ARGS__); \
		} else { \
			fprintf(SPSPS_STDERR, "ERROR %s:%d:%d: Unspecified error.\n", \
					(loc)->name, (loc)->line, (loc)->column); \
		} \
		free(loc); \
	} else { \
		if ((m_msg) != NULL) { \
			fprintf(SPSPS_STDERR, "ERROR: " m_msg "\n", ## __VA_ARGS__); \
		} else { \
			fprintf(SPSPS_STDERR, "ERROR: Unspecified error.\n"); \
		} \
	} \
}

/**
 * A parser object is an opaque pointer.
 */
typedef struct spsps_parser_ * Parser;

/**
 * Create a new parser instance.  The caller is responsible for freeing the
 * returned parser instance by calling spsps_free.  The provided file name is
 * copied to new allocated memory, and the caller is responsible for freeing
 * the first argument, if necessary.
 * @param name 			The name of the stream.  Typically a file name.
 * @param stream 		A stream to parse.
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
SPSPS_CHAR spsps_consume(Parser parser);

/**
 * Consume and discard the next few characters from the stream.
 * @param parser 		The parser.
 * @param n 			The number of characters to discard.
 */
void spsps_consume_n(Parser parser, size_t n);

/**
 * Consume and discard all whitespace.  When this method returns the next
 * character is the first non-whitespace character, or the end of file has
 * been reached.
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
SPSPS_CHAR spsps_peek(Parser parser);

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
SPSPS_CHAR * spsps_peek_n(Parser parser, size_t n);

/**
 * Peek ahead and determine if the next characters in the stream are the given
 * characters, in sequence.  That is, the given string must be the next thing
 * in the stream.
 * @param parser		The parser
 * @param next			The characters.
 * @return				True iff the stream contains the given string next.
 */
bool spsps_peek_str(Parser parser, SPSPS_CHAR * next);

/**
 * Peek ahead at the next few characters and if they are a given string, then
 * consume them.  Otherwise leave the stream unchanged.
 * @param parser 		The parser.
 * @param next 			The characters to check for.
 * @return 				False iff the stream was left unchanged.
 */
bool spsps_peek_and_consume(Parser parser, char * next);

#endif /* PARSER_H */
