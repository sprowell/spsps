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
#include "xstring.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ERR(m_parser, m_msg, ...) { \
	pLoc loc = spsps_loc(m_parser); \
	fprintf(stderr, "ERROR %s:%d:%d: " m_msg "\n", (loc)->name, \
			(loc)->line, (loc)->column, ## __VA_ARGS__); \
}

SPSPS_CHAR chbuf[15]; // U+002e (.)
char * printify(unsigned SPSPS_CHAR ch) {
	if (isprint(ch)) {
		sprintf(chbuf, "U+%04x (%1c)", ch, ch);
	} else {
		sprintf(chbuf, "U+%04x", ch);
	}
	return chbuf;
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
			puts("true\n");
		} else {
			ERR(parser, "Saw a value starting with 't' and expected "
					"true, but did not find that.  Did you forget to "
					"quote a string?");
			return;
		}
		break;
	case 'f':
		if (spsps_peek_and_consume(parser, "false")) {
			puts("false\n");
		} else {
			ERR(parser, "Saw a value starting with 'f' and expected "
					"false, but did not find that.  Did you forget to "
					"quote a string?");
			return;
		}
		break;
	case 'n':
		if (spsps_peek_and_consume(parser, "null")) {
			puts("null\n");
		} else {
			ERR(parser, "Saw a value starting with 'n' and expected "
					"null, but did not find that.  Did you forget to "
					"quote a string?");
			return;
		}
		break;
	default:
		// Check for a digit.
		if (isdigit(ch)) {
			parse_number(parser);
		} else {
			ERR(parser, "Expected to find a value, but instead found "
					"unexpected character %s, which does not start a "
					"value.  Did you forget to quote a string?", printify(ch));
			return;
		}
	}
}

void
parse_string(pParser parser) {
	// The first thing in the stream must be the quotation mark.
	if (! spsps_peek_and_consume(parser, "\"")) {
		ERR(parser, "Expected to find a quotation mark for a string, "
				"but instead found %s.", printify(spsps_peek(parser)));
		return;
	}
	// Read the rest of the string.
	mstring str = NULL;
	while (! spsps_peek_and_consume(parser, "\"") && !spsps_eof(parser)) {
		mstr_append(str, spsps_consume(parser));
	} // Loop over the characters in the string.
	char * cstring = mstr_cstr(str);
	printf("string: %s\n", cstring);
	free(cstring);
}

void
parse_number(pParser parser) {
	// Build the number by consuming the digits.
	bool neg = spsps_peek_and_consume(parser, "-");
	int value = 0;
	if (! isdigit(spsps_peek(parser))) {
		ERR(parser, "Expected to find a digit, but instead found %s.",
				printify(spsps_peek(parser)));
		return;
	}
	while (isdigit(spsps_peek(parser))) {
		value *= 10;
		value += spsps_consume(parser) - '0';
	} // Consume all digits.
	if (neg) value = -value;
	printf("number: %d\n", value);
}

void
parse_object(pParser parser) {
	// Arrays start with a curly brace.
	if (! spsps_peek_and_consume(parser, "{")) {
		ERR(parser, "Expected to find the start of an object (a curly "
				"brace), but instead found %s.", printify(spsps_peek(parser)));
		return;
	}
	// Now consume a sequence of string equal value pairs.
	puts("object start");
	while (! spsps_eof(parser)) {
		spsps_consume_whitespace(parser);
		SPSPS_CHAR ch = spsps_peek(parser);
		if (ch == '"') {
			// Start of a pair.  Parse the string.
			parse_string(parser);
			spsps_consume_whitespace(parser);
			// Expect an equal sign.
			if (! spsps_peek_and_consume(parser, "=")) {
				ERR(parser, "Expected to find an equal sign for a "
						"string = value pair, but instead found %s.",
						printify(spsps_peek(parser)));
				return;
			}
			spsps_consume_whitespace(parser);
			// Expect a value.
			parse_value(parser);
		} else if (ch == '}') {
			// End of object.
			break;
		} else {
			ERR(parser, "Expected to find the start of a "
					"string = value pair, or the end of the "
					"object (a closing curly brace), but "
					"instead found %s.", printify(ch));
			return;
		}
	} // Consume all pairs.
	puts("object end");
}

void
parse_array(pParser parser) {
	// Arrays start with a square bracket.
	if (! spsps_peek_and_consume(parser, "[")) {
		ERR(parser, "Expected to find the start of an array (a square "
				"bracket), but instead found %s.",
				printify(spsps_peek(parser)));
		return;
	}
	// Now consume a comma-separated list of items.
	puts("array start");
	spsps_consume_whitespace(parser);
	while (! spsps_eof(parser)) {
		parse_value(parser);
		spsps_consume_whitespace(parser);
		if (spsps_peek_and_consume(parser, ",")) {
			spsps_consume_whitespace(parser);
		} else if (spsps_peek_and_consume(parser, "]")) {
			break;
		}
	} // Consume until closing square bracket.
	puts("array end");
}
