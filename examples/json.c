/**
 * @file
 * Implementation of the parser for JSON.
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
#include "json.h"
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

json_value * parse_value(pParser parser);
json_value * parse_string(pParser parser);
json_value * parse_number(pParser parser);
json_value * parse_object(pParser parser);
json_value * parse_array(pParser array);

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

/**
 * Come here to read a JSON value from the stream.  Leading whitespace is
 * allowed.
 * @verbatim
 * value = string | number | object | array
 *       | "true" | "false" | "null"
 * @endverbatim
 * @param parser			The parser object.
 * @return					The parsed value, or NULL if an error occurs.
 */
json_value *
parse_value(pParser parser) {
	// Allow whitespace here.
	spsps_consume_whitespace(parser);

	// The next thing in the stream must be a quotation mark (a string), a
	// minus sign or digit (number), a curly brace (object) or a square
	// bracket (array).  It might also be true, false, or null.  That's it!
	SPSPS_CHAR ch = spsps_peek(parser);
	switch (ch) {
	case '"':
		return parse_string(parser);
		break;
	case '-':
		return parse_number(parser);
		break;
	case '{':
		return parse_object(parser);
		break;
	case '[':
		return parse_array(parser);
		break;
	case 't':
		if (spsps_peek_and_consume(parser, "true")) {
			return json_new_boolean(true);
		} else {
			ERR(parser, "Saw a value starting with 't' and expected "
					"true, but did not find that.  Did you forget to "
					"quote a string?");
			return NULL;
		}
		break;
	case 'f':
		if (spsps_peek_and_consume(parser, "false")) {
			return json_new_boolean(false);
		} else {
			ERR(parser, "Saw a value starting with 'f' and expected "
					"false, but did not find that.  Did you forget to "
					"quote a string?");
			return NULL;
		}
		break;
	case 'n':
		if (spsps_peek_and_consume(parser, "null")) {
			return json_new_null();
		} else {
			ERR(parser, "Saw a value starting with 'n' and expected "
					"null, but did not find that.  Did you forget to "
					"quote a string?");
			return NULL;
		}
		break;
	default:
		// Check for a digit.
		if (isdigit(ch)) {
			return parse_number(parser);
		} else {
			ERR(parser, "Expected to find a value, but instead found "
					"unexpected character %s, which does not start a "
					"value.  Did you forget to quote a string?", printify(ch));
			return NULL;
		}
	}
}

/* Private function to convert a character to a nibble.  If the character
 * is not a valid hex character (upper- or lower-case) then the return value
 * is 255.  Otherwise it is going to be in the range [0,15].
 */
char
unhex_(char ch) {
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	} else if (ch >= 'a' && ch <= 'f') {
		return ch - 'a' + 10;
	} else if (ch >= 'A' && ch <= 'F') {
		return ch - 'A' + 10;
	} else {
		return 255;
	}
}

/**
 * Come here to parse a quoted string from the stream.  The stream must
 * be pointing to the open quotation mark.
 * @verbatim
 * string = '"' ( CHARACTER | ESCAPE )* '"'
 * @endverbatim
 * A `CHARACTER` is any character other than `\` and `"`.  An escape can
 * be any of `\r`, `\n`, `\t`, or a protected character like `\` and `"`.
 * It can also be used to join lines if it is the last character on a line,
 * and it can introduce an 8-bit hex escape (`\x0a`, for instance).
 * @param parser			The parser.
 * @return					The parsed string value, or NULL on error.
 */
json_value *
parse_string(pParser parser) {
	// The first thing in the stream must be the quotation mark.
	if (! spsps_peek_and_consume(parser, "\"")) {
		ERR(parser, "Expected to find a quotation mark for a string, "
				"but instead found %s.", printify(spsps_peek(parser)));
		return NULL;
	}
	// Read the rest of the string.
	mstring str = NULL;
	SPSPS_CHAR highc, lowc;
	char high, low;
	while (! spsps_peek_and_consume(parser, "\"") && !spsps_eof(parser)) {
		if (spsps_peek_and_consume(parser, "\\")) {
			// Process an escape.
			SPSPS_CHAR ch = spsps_consume(parser);
			switch (ch) {
			case 'n':
				str = mstr_append(str, '\n');
				break;
			case 'r':
				str = mstr_append(str, '\r');
				break;
			case 't':
				str = mstr_append(str, '\t');
				break;
			case '\r':
				spsps_peek_and_consume(parser, "\n");
				break;
			case '\n':
				break;
			case 'x':
				// Extract the two hexadecimal characters.
				highc = spsps_consume(parser);
				lowc = spsps_consume(parser);
				high = unhex_((char) highc);
				low = unhex_((char) lowc);
				if (high > 15) {
					ERR(parser, "Expected to find two hexadecimal digits "
							"in an escape (starting with \\x) but "
							"instead found %s.", printify(highc));
				}
				if (low > 15) {
					ERR(parser, "Expected to find two hexadecimal digits "
							"in an escape (starting with \\x) but "
							"instead found %s.", printify(lowc));
				}
				str = mstr_append(str, (char) ((high << 4)|low));
				break;
			default:
				str = mstr_append(str, ch);
				break;
			}
		} else {
			str = mstr_append(str, spsps_consume(parser));
		}
	} // Loop over the characters in the string.
	char * cstring = mstr_cstr(str);
	return json_new_string(cstring);
}

/**
 * Come here to parse a number.  The first character in the stream is
 * expected to be the first character of the number.  This could be a
 * minus sign, or it could be a digit.  There must be at least one
 * digit.  The digits are interpreted in base 10, and stored in a
 * C `int`.
 * @verbatim
 * number = '-'? ( '0'..'9' )+
 * @endverbatim
 * @param parser			The parser.
 */
json_value *
parse_number(pParser parser) {
	// Build the number by consuming the digits.
	bool neg = spsps_peek_and_consume(parser, "-");
	int value = 0;
	if (! isdigit(spsps_peek(parser))) {
		ERR(parser, "Expected to find a digit, but instead found %s.",
				printify(spsps_peek(parser)));
		return NULL;
	}
	while (isdigit(spsps_peek(parser))) {
		value *= 10;
		value += spsps_consume(parser) - '0';
	} // Consume all digits.
	if (neg) value = -value;
	return json_new_number(value);
}

/**
 * Come here to parse a JSON object.  The first character in the stream is
 * expected to be the opening curly brace for the object.  Objects can be
 * empty.
 * @verbatim
 * object = '{' ( string '=' value )* '}'
 * @endverbatim
 * @param parser			The parser.
 */
json_value *
parse_object(pParser parser) {
	// Arrays start with a curly brace.
	if (! spsps_peek_and_consume(parser, "{")) {
		ERR(parser, "Expected to find the start of an object (a curly "
				"brace), but instead found %s.", printify(spsps_peek(parser)));
		return NULL;
	}
	// Now consume a sequence of string equal value pairs.
	json_object * object = NULL;
	while (! spsps_eof(parser)) {
		spsps_consume_whitespace(parser);
		SPSPS_CHAR ch = spsps_peek(parser);
		if (ch == '"') {
			// Start of a pair.  Parse the string.
			json_value * key = parse_string(parser);
			spsps_consume_whitespace(parser);
			// Expect an equal sign.
			if (! spsps_peek_and_consume(parser, "=")) {
				ERR(parser, "Expected to find an equal sign for a "
						"string = value pair, but instead found %s.",
						printify(spsps_peek(parser)));
				return NULL;
			}
			spsps_consume_whitespace(parser);
			// Expect a value.
			json_value * value = parse_value(parser);
		} else if (ch == '}') {
			// End of object.
			break;
		} else {
			ERR(parser, "Expected to find the start of a "
					"string = value pair, or the end of the "
					"object (a closing curly brace), but "
					"instead found %s.", printify(ch));
			return NULL;
		}
	} // Consume all pairs.
	puts("object end");
}

/**
 * Come here to parse a JSON array.  The first character in the stream is
 * expected to be the opening square bracket.  Arrays can be empty.
 * @verbatim
 * array = '[' ( value ( ',' value )* )? ']'
 * @endverbatim
 * @param parser				The parser.
 */
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

void
json_free_value(json_value * value) {
	json_object * object;
	json_array * array;
	switch (value->kind) {
	case OBJECT:
		// Free all pairs in the object.
		object = value->content.objectvlaue;
		for (int index = 0; index < MAP_SIZE; ++index) {
			json_object_entry * entry = object->map[index];
			while (entry != NULL) {
				free(entry->key);
				json_free_value(entry->value);
				entry = entry->next;
			} // Free all entries with this hash code.
		} // Free all entries.
		free(object);
		free(value);
		break;
	case ARRAY:
		// Free all elements of the array.
		array = value->content.arrayvalue;
		for (int index = 0; index < array->size; ++index) {
			json_free_value(array->array[index]);
		} // Deallocate all entries.
		free(array->array);
		free(array);
		free(value);
		break;
	case STRING:
		// Free the string.
		free(value->content.strvalue);
		free(value);
		break;
	case NUMBER:
	case BOOL:
	case NOTHING:
		// These three cases are easy.  Just deallocate the value.
		free(value);
		break;
	}
}

uint32_t
hash_string_(unsigned char * str) {
	uint32_t hash = 5381;
	uint32_t ch;
	for (uint32_t ch = *str; ch != 0; str++) {
		// Use shift to multiply by 32, then subtract once to get
		// multiplication by 31... my favorite hash multiplier.
		// No reason.  Then we add the character.  Note that this
		// preserves some low bits from prior characters, so the
		// effect of earlier characters never really washes out.
		hash = (hash << 5) - hash + ch;
	} // Compute the hash over the string.
	return hash;
}

json_object *
json_object_insert(json_object * object, char * key, json_value * value) {
	if (object == NULL) {
		// Allocate an object.
		object = (json_object *) malloc(sizeof(json_object));
	}
	if (key == NULL) {
		// The key is NULL.  Replace it with the empty string.
		// We have to allocate, so that the caller can later
		// deallocate.
		key = (char *) malloc(sizeof(char)*1);
		key[0] = 0;
	}
	if (value == NULL) {
		// The value is NULL.  Replace this with an "official"
		// JSON null.
		value = (json_value *) malloc(sizeof(json_value));
		value->kind = NOTHING;
	}
	// Compute the hash code for the string.
	uint32_t hash = hash_string_((unsigned char *) key) % MAP_SIZE;
	// Get the map entry.
	json_object_entry * entry = object->map[hash];
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			// Free any prior value.
			json_free_value(entry->value);
			entry->value = value;
			return object;
		}
		entry = entry->next;
	} // Find an entry to replace.
	// Did not find an existing entry; insert the new entry.
	entry = (json_object_entry *) malloc(sizeof(json_object_entry));
	entry->key = key;
	entry->value = value;
	entry->next = object->map[hash];
	object->map[hash] = entry;
	return object;
}

json_value *
json_new_boolean(bool flag) {
	json_value * value = (json_value *) malloc(sizeof(json_value));
	value->kind = BOOL;
	value->content.boolvalue = flag;
	return value;
}

json_value *
json_new_null() {
	json_value * value = (json_value *) malloc(sizeof(json_value));
	value->kind = NOTHING;
	return value;
}

json_value *
json_new_number(int number) {
	json_value * value = (json_value *) malloc(sizeof(json_value));
	value->kind = NUMBER;
	value->content.numvalue = number;
	return value;
}

json_value *
json_new_string(char * str) {
	json_value * value = (json_value *) malloc(sizeof(json_value));
	value->kind = STRING;
	value->content.strvalue = str;
	return value;
}

json_value *
json_new_array(size_t size) {
	json_value * value = (json_value *) malloc(sizeof(json_value));
	value->kind = ARRAY;
	value->content.arrayvalue = (json_array *) malloc(sizeof(json_array));
	value->content.arrayvalue->size = size;
	value->content.arrayvalue->array =
			(json_value **) malloc(sizeof(json_value *) * size);
	return value;
}

json_value *
json_array_element(json_value * value, size_t index) {
	if (value == NULL || value->kind != ARRAY) {
		// There is no array.
		return NULL;
	}
	json_array * array = value->content.arrayvalue;
	if (index >= array->size) {
		return NULL;
	}
	return array->array[index];
}

json_value *
json_get_entry(json_value * value, char * key) {
	if (value == NULL || value->kind != OBJECT) {
		return NULL;
	}
	if (key == NULL) return NULL;
	json_object * object = value->content.objectvlaue;
	// Compute the hash code for the string.
	uint32_t hash = hash_string_((unsigned char *) key) % MAP_SIZE;
	// Get the map entry.
	json_object_entry * entry = object->map[hash];
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			return entry->value;
		}
		entry = entry->next;
	} // Find the entry.
	// Did not find the entry.
	return NULL;
}

void
json_stream(FILE * stream, json_value * value, int depth) {
	if (stream == NULL) stream = stdout;
	if (value == NULL) return;
	switch (value->kind) {
	if (depth > 0) {
		for (int advance = 0; advance < depth; ++advance) {
			fprintf(stream, "  ");
		} // Indent.
	}
	case NOTHING:
		fprintf(stream, "null\n");
		break;
	case BOOL:
		fprintf(stream, value->content.boolvalue ? "true\n" : "false\n");
		break;
	case NUMBER:
		fprintf(stream, "%d\n", value->content.numvalue);
		break;
	case STRING:
		fprintf(stream, "\"%s\"", value->content.strvalue);
		break;
	case ARRAY:
		fprintf(stream, "[\n");
		for (int index = 0; index < value->content.arrayvalue->size; ++index) {
			json_stream(stream, value->content.arrayvalue->array[index], depth+1);
		} // Write all entries.
		if (depth > 0) {
			for (int advance = 0; advance < depth; ++advance) {
				fprintf(stream, "  ");
			} // Indent.
		}
		fprintf(stream, "]\n");
		break;
	case OBJECT:
		fprintf(stream, "{\n");
		for (int index = 0; index < MAP_SIZE; ++index) {
			json_object_entry * entry = value->content.objectvlaue->map[index];
			while (entry != NULL) {
				fprintf(stream, "\"%s\" = ", entry->key);
				json_stream(stream, entry->value, depth+1);
				entry = entry->next;
			} // Write all entries off this hash.
		} // Write all entries.
		if (depth > 0) {
			for (int advance = 0; advance < depth; ++advance) {
				fprintf(stream, "  ");
			} // Indent.
		}
		fprintf(stream, "}\n");
		break;
	}
}
