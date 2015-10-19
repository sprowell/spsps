/**
 * @file
 * Implementation of the SPSPS library.
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

#include "parser.h"
#include <string.h>
#include <ctype.h>
#include <wchar.h>

//======================================================================
// Definition of the parser struct.
//======================================================================

struct spsps_parser_ {
	/// Whether the parser has consumed the end of file.
	bool at_eof;
	/// The current zero-based block index.
	uint8_t block;
	/// The blocks.
	char blocks[2][SPSPS_LOOK];
	/// How many times we have consumed the EOF.
	uint16_t eof_count;
	/// How many times we have peeked without consuming.
	uint16_t look_count;
	/// The name of the source.
	char * name;
	/// The next entry in the current block.
	size_t next;
	/// The stream providing characters.
	FILE * stream;
	/// The current column number.
	uint32_t column;
	/// The current line number.
	uint32_t line;
	/// The most recent error code.
	spsps_errno errno;
	/// Whether the buffer been initialized the first time.
	bool initialized;
};

//======================================================================
// Helper functions.
//======================================================================

// Reserve plenty of space for the string and the terminating nul.  Note
// that we reserve a larger chunk than is necessary, since Unicode characters
// are only valid up to six digits at present.  However, we will produce
// (potentially) 8 character results for invalid code points.
char chbuf[24]; // u+12345678 (c)
// hex digits (8) + "u+" (2) + space (1) + parens (2) + Unicode code point in
// UTF-8 (4) + terminating nul (1) = 18.

char *
spsps_printchar(utf32_char ch) {
	// Allocation: Nothing is allocated or deallocated by this method.
	char * format = ch < 0x10000 ? "U+%04lX (%lc)" : "u+%lX (%lc)";
	// Check to see if this is an ISO control character and, if so, suppress
	// printing the character.
	if (is_ISO_control(ch)) {
		// This is an ISO control character.
		sprintf(chbuf, format, ch, '?');
	} else {
		// This is in the BMP range.
		sprintf(chbuf, format, ch, ch);
	}
	return chbuf;
}

void
spsps_read_other_(Parser parser) {
	// Allocation: Nothing is allocated or deallocated by this method.
	size_t count = SPSPS_READ(parser->blocks[parser->block^1], 1,
		SPSPS_LOOK, parser->stream);
	if (count < SPSPS_LOOK) {
		memset(parser->blocks[parser->block^1] + count, SPSPS_EOF,
			SPSPS_LOOK - count);
	}
}

void spsps_initialize_parser_(Parser parser) {
	// Allocation: Nothing is allocated or deallocated by this method.
	spsps_read_other_(parser);
	parser->block ^= 1;
	parser->initialized = true;
}

//======================================================================
// Primitives.
//======================================================================

char *
spsps_loc_to_string(Loc * loc) {
	if (loc == NULL) {
		// Return something the caller can deallocate.
		char * ret = (char *) malloc(1);
		ret[0] = 0;
		return ret;
	}
	// Generate the string.  An unsigned 32 bit integer can produce (with no
	// bugs) a value up to 2^32-1.  This is 4*2^30-1, or roughly 4*10^9.  We
	// allocate 12 digits for each number (which is sufficient), two for
	// colons, and one for the terminating null.  This is 27.  We then add the
	// strlen of the name.
	size_t mlen = 27 + strlen(loc->name);
	char * buf = (char *) malloc(mlen);
	sprintf(buf, "%s:%d:%d", loc->name, loc->line, loc->column);
	if (strlen(buf) > mlen) {
		// This should never, never, never happen.
		_SPSPS_ERROR("Internal error in loc string construction.\n");
		exit(1);
	}
	return buf;
}

utf32_char
spsps_look_(Parser parser, size_t n) {
	// Allocation: Nothing is allocated or deallocated by this method.
	if (n >= SPSPS_LOOK) {
		// Lookahead too large.
		parser->errno = LOOKAHEAD_TOO_LARGE;
		return 0;
	}
	// If we look too long without progressing, the parser may be stalled.
	parser->look_count++;
	if (parser->look_count > 1000) {
		// Stalled.
		parser->errno = STALLED;
		return SPSPS_EOF;
	}

	if(!parser->initialized){
		spsps_initialize_parser_(parser);
	}

	// Look in the correct block.
	if (n + parser->next < SPSPS_LOOK) {
		return parser->blocks[parser->block][n + parser->next];
	} else {
		return parser->blocks[parser->block^1][n + parser->next - SPSPS_LOOK];
	}
}

//======================================================================
// Implementation of public interface.
//======================================================================

Parser
spsps_new(char * name, FILE * stream) {
	// Allocate a new parser.  Duplicate the name.
	Parser parser = (Parser) calloc(1, sizeof(struct spsps_parser_));
	parser->at_eof = false;
	parser->block = 0;
	parser->eof_count = 0;
	parser->look_count = 0;
	if (name != NULL) parser->name = strdup(name);
	else parser->name = strdup("(unknown)");
	parser->next = 0;
	if (stream != NULL) parser->stream = stream;
	else parser->stream = stdin;
	parser->column = 1;
	parser->line = 1;
	parser->errno = OK;
	parser->initialized = false;
	return parser;
}

void
spsps_free(Parser parser) {
	// Free the parser name and the parser itself.
	free(parser->name);
	parser->at_eof = true;
	parser->name = NULL;
	parser->stream = NULL;
	free(parser);
}

utf32_char
spsps_consume(Parser parser) {
	// Nothing is allocated or deallocated by this method.
	utf32_char ch = spsps_look_(parser, 0);
	spsps_consume_n(parser, 1);
	parser->errno = OK;
	return ch;
}

void
spsps_consume_n(Parser parser, size_t n) {
	// Nothing is allocated or deallocated by this method.
	if (n >= SPSPS_LOOK) {
		// Lookahead too large.
		parser->errno = LOOKAHEAD_TOO_LARGE;
		return;
	}

	if(!parser->initialized){
		spsps_initialize_parser_(parser);
	}

	parser->errno = OK;
	parser->look_count = 0;
	if (parser->at_eof) {
		parser->eof_count++;
		if (parser->eof_count > 1000) {
			// Stalled at EOF.
			parser->errno = STALLED_AT_EOF;
			return;
		}
	}
	for (size_t count = 0; count < n; ++count) {
		if (parser->blocks[parser->block][parser->next] == SPSPS_EOF) {
			parser->at_eof = true;
			return;
		}
		parser->column++;
		if (parser->blocks[parser->block][parser->next] == '\n') {
			parser->line++;
			parser->column = 1;
		}
		parser->next++;
		if (parser->next >= SPSPS_LOOK) {
			parser->next -= SPSPS_LOOK;
			parser->block ^= 1;
			spsps_read_other_(parser);
		}
	} // Loop to consume characters.
}

void
spsps_consume_whitespace(Parser parser) {
	// Nothing is allocated or deallocated by this method.
	while (strchr(" \t\r\n", spsps_peek(parser)) != NULL)
		spsps_consume(parser);
	parser->errno = OK;
}

bool
spsps_eof(Parser parser) {
	// Nothing is allocated or deallocated by this method.
	parser->errno = OK;
	return parser->at_eof;
}

Loc *
spsps_loc(Parser parser) {
	// A loc instance is allocated by this method.
	parser->errno = OK;
	Loc * loc = (Loc *) malloc(sizeof(struct spsps_loc_));
	loc->name = parser->name;
	loc->line = parser->line;
	loc->column = parser->column;
	return loc;
}

utf32_char
spsps_peek(Parser parser) {
	// Nothing is allocated or deallocated by this method.
	parser->errno = OK;
	return spsps_look_(parser, 0);
}

utf8_string
spsps_peek_n(Parser parser, size_t n) {
	// Allocates and returns a fixed-length string.
	if (n >= SPSPS_LOOK) {
		// Lookahead too large.
		parser->errno = LOOKAHEAD_TOO_LARGE;
		return "";
	}
	parser->errno = OK;
	utf8_string buf = (utf8_string) malloc(sizeof(char) * n);
	for (size_t index = 0; index < n; ++index) {
		buf[index] = spsps_look_(parser, index);
	} // Read characters into buffer.
	return buf;
}

bool
spsps_peek_str(Parser parser, utf8_string next) {
	// Nothing is allocated or deallocated by this method.
	size_t n = strlen(next);
	if (n >= SPSPS_LOOK) {
		// Lookahead too large.
		parser->errno = LOOKAHEAD_TOO_LARGE;
		return false;
	}
	parser->errno = OK;
	for (size_t index = 0; index < n; ++index) {
		if (next[index] != spsps_look_(parser, index)) return false;
	} // Check all characters.
	return true;
}

bool
spsps_peek_and_consume(Parser parser, utf8_string next) {
	// Nothing is allocated or deallocated by this method.
	parser->errno = OK;
	if (spsps_peek_str(parser, next)) {
		spsps_consume_n(parser, strlen(next));
		return true;
	} else {
		return false;
	}
}
