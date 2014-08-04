/**
 * @file
 * Eample parser for JSON.
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

#define SPSPS_SHORTHAND
#include "parser.h"
#include <stdlib.h>
#include <string.h>

#define ERRF(m_parser, m_msg, ...) { \
	pLoc loc = spsps_loc(m_parser); \
	fprintf(stderr, "ERROR %s:%d:%d: " m_msg "\n", (loc)->name, \
			(loc)->line, (loc)->column, __VA_ARGS__); \
}
#define ERR(m_parser, m_msg) { \
	pLoc loc = spsps_loc(m_parser); \
	fprintf(stderr, "ERROR %s:%d:%d: " m_msg "\n", (loc)->name, \
			(loc)->line, (loc)->column); \
}


void parse_value(pParser parser);
void parse_string(pParser parser);
void parse_number(pParser parser);
void parse_object(pParser parser);
void parse_array(pParser array);


int main(int argc, char * argv[]) {
	// If a first argument is provided, it is the file name.  If no first
	// argument is provided, then read from standard in.
	FILE * input = stdin;
	if (argc > 1) {
		input = fopen(argv[1], "rt");
		if (input == NULL) {
			fprintf(stderr, "ERROR: Unable to read from file %s.\n", argv[1]);
			exit(1);
		}
	}

	// Now we have to read from the stream.  We read and parse JSON.
	pParser parser = spsps_new(argv[1], input);
	parse_value(parser);

	// Done.
	if (argc > 1) {
		fclose(input);
	}
	exit(0);
}

void
parse_value(pParser parser) {
	// Allow whitespace here.
	spsps_consume_whitespace(parser);

	// The next thing in the stream must be a quotation mark (a string), a
	// minus sign or digit (number), a curly brace (object) or a square
	// bracket (array).  It might also be true, false, or null.  That's it!
	SPSPS_CHAR ch = spsps_peek(parser);
	switch (ch) {
	case '"':
		parse_string(parser);
		break;
	case '-':
		parse_number(parser);
		break;
	case '{':
		parse_object(parser);
		break;
	case '[':
		parse_array(parser);
		break;
	case 't':
		if (spsps_peek_and_consume(parser, "true")) {
			puts("true");
		} else {
			ERR(parser, "Saw a value starting with 't' and expected "
					"true, but did not find that.  Did you forget to "
					"quote a string?");
			return;
		}
		break;
	case 'f':
		if (spsps_peek_and_consume(parser, "false")) {
			puts("false");
		} else {
			ERR(parser, "Saw a value starting with 'f' and expected "
					"false, but did not find that.  Did you forget to "
					"quote a string?");
			return;
		}
		break;
	case 'n':
		if (spsps_peek_and_consume(parser, "null")) {
			puts("false");
		} else {
			ERR(parser, "Saw a value starting with 'n' and expected "
					"null, but did not find that.  Did you forget to "
					"quote a string?");
			return;
		}
		break;
	default:
		// Check for a digit.
		if (ch >= '0' && ch <= '9') {
			parse_number(parser);
		} else {
			ERRF(parser, "Expected to find a value, but instead found "
					"unexpected character %c, which does not start a "
					"value.  Did you forget to quote a string?", ch);
		}
	}
}

void
parse_string(pParser parser) {
	// The first thing in the stream must be the quotation mark.
	if (! spsps_peek_and_consume(parser, "\"")) {
		ERRF(parser, "Expected to find a quotation mark for a string, "
				"but instead found '%c'.", spsps_peek(parser));
		return;
	}
	// Read the rest of the string.
	size_t limit = 64;
	size_t pos = 0;
	SPSPS_CHAR * str = malloc(sizeof(SPSPS_CHAR) * limit);
	while (! spsps_peek_and_consume(parser, "\"") && !spsps_eof(parser)) {
		str[pos++] = spsps_consume(parser);
		if (pos >= limit) {
			int oldlimit = limit;
			limit += 64;
			SPSPS_CHAR * newstr = malloc(sizeof(SPSPS_CHAR) * limit);
			memcpy(newstr, str, oldlimit);
			str = newstr;
		}
	} // Loop over the characters in the string.

	printf("string: %s", str);
}

void
parse_number(pParser parser) {

}

void
parse_object(pParser parser) {

}

void
parse_array(pParser array) {

}
