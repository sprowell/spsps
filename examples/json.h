#ifndef JSON_H_
#define JSON_H_

/**
 * @file
 * Definitions for the JSON parser.
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

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

/**********************************************************************
* Data structures for JSON.
**********************************************************************/

/// Size of the map.  To override this #define it prior to inclusion.
#ifndef MAP_SIZE
	#define MAP_SIZE 256
#endif

/// The kind of a JSON value.
typedef enum json_kind_ {
	OBJECT, ARRAY, NUMBER, STRING, BOOL, NOTHING
	// Note we use NOTHING instead of NULL, because its reserved.
} json_kind;

/// A single entry in a JSON object.  The object is a hash map, so an entry is
/// actually going to be a singly linked list of entries that have the same
/// hash code.
typedef struct json_object_entry_ {
	char * key; //< The key for the value.
	struct json_value_ * value; //< The value.
	struct json_object_entry_ * next; //< A pointer to the next entry.
} json_object_entry;

/// A simple JSON object.
typedef struct json_object_ {
	struct json_object_entry_ * map[MAP_SIZE];
} json_object;

/// A simple JSON array.
typedef struct json_array_ {
	size_t size; //< Size of the array.
	struct json_value_ ** array; //< The array.
} json_array;

/// The content of a JSON value.
typedef union json_content_ {
	double numvalue; //< A number value.
	char * strvalue; //< A string value.
	bool boolvalue; //< A Boolean value.
	json_array * arrayvalue; //< An array value.
	json_object * objectvalue; //< An object value.
} json_content;

/// A JSON value.
typedef struct json_value_ {
	json_kind kind; //< The kind of the value.
	json_content content; //< The actual value.
} json_value;

/**********************************************************************
* JSON manipulators.
**********************************************************************/

/**
 * Free a JSON value.  Values can be quite complicated, and this will
 * free the value and all subordinate values (for an array or object).
 * @param value			The value to free.
 */
void json_free_value(json_value * value);

/**
 * Compute a hash function over the string.
 * @param str			The string.
 * @return				The computed hash.
 */
uint32_t hash_string_(unsigned char * str);

/**
 * Insert an entry into a JSON object.  The object is implemented as
 * a very simple hash table, with collisions resolved by chaining.
 * @param object		The object to modify.  Can be NULL.
 * @param key			The key to insert.
 * @param value			The value to insert.
 * @return				The modified object, created iff NULL was given.
 */
json_object * json_object_insert(json_object * object, char * key,
		json_value * value);

/**
 * Make and return a new JSON Boolean value.
 * @param flag			The value.
 * @return				The new JSON value.
 */
json_value * json_new_boolean(bool flag);

/**
 * Make and return a new JSON null value.
 * @return				The new JSON null value.
 */
json_value * json_new_null();

/**
 * Make and return a new JSON number value.
 * @param number		The value.
 * @return				The new JSON value.
 */
json_value * json_new_number(double number);

/**
 * Make and return a new JSON string value.  The string is stored here,
 * and will be deallocated when this entry is deallocated.
 * @param str			The value.
 * @return				The new JSON value.
 */
json_value * json_new_string(char * str);

/**
 * Make and return a new JSON array.  The array size is set, but the
 * array is not populated.
 * @param size			The array size.
 * @return				The new JSON array.
 */
json_value * json_new_array(size_t size);

/**
 * Obtain a pointer to an element of a JSON array.  If the element
 * does not exist, or the value given is not an array, then NULL is
 * returned, so watch for that.
 * @param value			The JSON array.
 * @param index			The zero-based index of the element.
 * @return				The array.
 */
json_value * json_array_element(json_value * value, size_t index);

/**
 * Set an element in a JSON array.  If the element does not exist
 * or the value given is not an array, then NULL is returned, so
 * watch for that.  If the array has an element already at this
 * position, then it is replaced, but NOT deallocated!  Be aware!
 * @param value			The JSON array.
 * @param index			The zero-based index of the element.
 * @param entry			The entry to store.
 * @return				The array.
 */
json_value * json_set_array_element(json_value * value, size_t index,
		json_value * entry);

/**
 * Obtain a pointer to an entry in a JSON object.  If the entry
 * does not exist, or the value given is not an object, then NULL is
 * returned, so watch for that.
 */
json_value * json_get_entry(json_value * value, char * key);

/**
 * Format and print a JSON value to the given stream.
 * @param stream		The stream to get the data.
 * @param value			The JSON value.
 * @param depth			Indentation depth.
 */
void json_stream(FILE * stream, json_value * value, int depth);

/**********************************************************************
* JSON parser.
**********************************************************************/

/**
 * Parse a JSON value from the given parser instance.
 * @param parser		The parser.
 * @return				The next JSON value parsed.
 */
json_value * parse_value(Parser parser);

#endif /* JSON_H_ */
