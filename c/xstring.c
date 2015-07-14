/**
 * @file
 * Definitions for the simple string.
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

#include "xstring.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Internal storage is in bytes, so we use arrays of chars.  If we use
// either uint8_t or int8_t we run into sign difficulties.
typedef char xch;

/* Some notes.
 *
 * Never allocate any space for an xstring unless it is going to
 * contains something.  That is, an empty xstring is equivalent to
 * NULL.  The same (mostly) goes for mstring, but it may make sense
 * to allocate space there, since mstrings are mutable.
 */
struct xstring_ {
	/* How this works.
	 *
	 * The cstr points to a character array of xchs.  It can be
	 * NULL, which should be treated the same as the empty string.
	 * The length is the length of the string.  Storing the length
	 * allows the character array to contain null characters (\0).
	 *
	 * It is assumed throughout this library that if length is not
	 * zero, then cstr is not NULL.  Pay attention to this invariant!
	 */
	xch * cstr;
	size_t length;
};

struct mstring_ {
	/* How this works.
	 *
	 * Mutable strings always allocate memory, even when the string
	 * is empty.  This allows them to be quickly added to, which is
	 * the primary use case for a mutable string.  The cstr points
	 * to a character array of xchs.  It cannot be NULL; it is
	 * always allocated, except when the string is deallocated.  The
	 * local_capacity is the length of the cstr.  The local_length is
	 * the number of characters used in the cstr.  When the capacity
	 * must expand, this is done by adding another new mstring_ struct
	 * via next (a linked list).  The length is the total length of
	 * the string from this block forward along the chain.
	 *
	 * It is assumed throughout this library that if length is not zero
	 * then cstr is not NULL.  Pay attention to this invariant!
	 */
	xch * cstr;
	size_t local_capacity;
	size_t local_length;
	size_t length;
	struct mstring_ * next;
};

void
mstr_inspect(mstring str) {
	printf("Inspecting mstring:\n");
	while (str != NULL) {
		printf("  mstring {\n");
		printf("    local_capacity = %lu\n", str->local_capacity);
		printf("      local_length = %lu\n", str->local_length);
		printf("            length = %lu\n", str->length);
		printf("              cstr = [");
		int count = 16;
		size_t truelength = str->local_length;
		for (int index = 0; index < truelength; ++index) {
			unsigned char ch = * ((unsigned char *) str->cstr + index);
			if (isprint(ch)) {
				printf("%02x (%1c),", ch, ch);
			} else {
				printf("%02x (?),", ch);
			}
			--count;
			if (count <= 0) {
				printf("\n                      ");
				count = 16;
			}
		}
		if (count != 16) {
			printf("\n                      ");
			count = 16;
		}
		printf("/* local_length boundary */");
		printf("\n                      ");
		size_t fulllength = str->local_capacity;
		for (size_t index = truelength; index < fulllength; ++index) {
			unsigned char ch = * ((unsigned char *) str->cstr + index);
			if (isprint(ch)) {
				printf("%02x (%1c),", ch, ch);
			} else {
				printf("%02x (?),", ch);
			}
			--count;
			if (count <= 0) {
				printf("\n                      ");
				count = 16;
			}
		}
		printf("]\n");
		printf("  }\n");
		str = str->next;
	}
	printf("  NULL\n");
}

xstring
xstr_new() {
	// Allocate the new structure, but do not allocate any space
	// for characters.
	xstring empty = (xstring) malloc(sizeof(struct xstring_));
	empty->length = 0;
	empty->cstr = NULL;
	return empty;
}

mstring
mstr_new(size_t capacity) {
	// Figure out whether to use the default capacity.  Then
	// allocate to the defined capacity.  Note that the character
	// array is allocated immediately.
	if (capacity == 0) capacity = MSTR_INC;
	mstring empty = (mstring) malloc(sizeof(struct mstring_));
	empty->local_length = 0;
	empty->length = 0;
	empty->next = NULL;
	empty->cstr = (xch *) malloc(capacity);
	empty->local_capacity = capacity;
	return empty;
}

void
xstr_free(xstring value) {
	if (value == NULL) return;
	// We set the length before we wipe out the cstr, since we
	// always check the length in lieu of the cstr.
	value->length = 0;
	if (value->cstr != NULL) {
		free(value->cstr);
		value->cstr = NULL;
	}
	free(value);
}

void
mstr_free(mstring value) {
	mstring here = value;
	while (here != NULL) {
		here->local_capacity = 0;
		here->local_length = 0;
		// We set the length before we wipe out the cstr, since we
		// always check the length in lieu of the cstr.
		here->length = 0;
		if (here->cstr != NULL) {
			free(here->cstr);
			here->cstr = NULL;
		}
		value = here;
		here = value->next;
		free(value);
	} // Deallocate all blocks in the chain.
}

xstring
xstr_wrap(utf8_string value) {
	size_t len = (value == NULL) ? 0 : strlen((char *) value);
	// If the length is zero, don't allocate anything.
	if (len == 0) return NULL;
	xstring str = xstr_new();
	str->length = len;
	str->cstr = (utf8_string) malloc(len);
	for (size_t index = 0; index < len; ++index) {
		str->cstr[index] = (xch) value[index];
	} // Copy all characters, converting if necessary.
	return str;
}

xstring
xstr_wrap_f(utf8_string value) {
	xstring ret = xstr_wrap(value);
	if (value != NULL) free(value);
	return ret;
}

mstring
mstr_wrap(utf8_string value) {
	size_t len = (value == NULL) ? 0 : strlen((char *) value);
	if (len == 0) return NULL;
	mstring str = mstr_new(len + MSTR_INC);
	str->length = len;
	str->local_length = len;
	for (size_t index = 0; index < len; ++index) {
		str->cstr[index] = (xch) value[index];
	} // Copy all characters, converting if necessary.
	return str;
}

mstring
mstr_wrap_f(utf8_string value) {
	mstring ret = mstr_wrap(value);
	if (value != NULL) free(value);
	return ret;
}

xstring
xstr_copy(xstring other) {
	if (other == NULL || other->length == 0) return NULL;
	xstring str = xstr_new();
	str->length = other->length;
	str->cstr = (xch *) malloc(other->length);
	memcpy(str->cstr, other->cstr, other->length);
	return str;
}

mstring
mstr_copy(mstring other) {
	size_t len = other == NULL ? 0 : other->length;
	if (len == 0) return NULL;
	mstring str = mstr_new(len + MSTR_INC);
	str->length = len;
	str->local_length = len;
	mstring here = other;
	xch * there = str->cstr;
	while (here != NULL) {
		if (here->cstr != NULL)
			memcpy(there, here->cstr, here->local_length);
		there += here->local_length;
		here = here->next;
	} // Copy all blocks to the new string.
	return str;
}

xstring
mstr_to_xstr(mstring other) {
	size_t len = other == NULL ? 0 : other->length;
	if (len == 0) return NULL;
	xstring ret = xstr_new();
	ret->length = len;
	ret->cstr = (xch *) malloc(len);
	mstring here = other;
	xch * there = ret->cstr;
	while (here != NULL) {
		if (here->cstr != NULL)
			memcpy(there, here->cstr, here->local_length);
		there += here->local_length;
		here = here->next;
	} // Copy all blocks to the new string.
	return ret;
}

mstring
xstr_to_mstr(xstring other) {
	size_t len = other == NULL ? 0 : other->length;
	if (len == 0) return NULL;
	mstring ret = mstr_new(len + MSTR_INC);
	memcpy(ret->cstr, other->cstr, len);
	ret->local_length = len;
	ret->length = len;
	return ret;
}

size_t
xstr_length(xstring value) {
	if (value == NULL) return 0;
	return value->length;
}

size_t
mstr_length(mstring value) {
	if (value == NULL) return 0;
	return value->length;
}

xstring
xstr_append(xstring value, utf32_char ch) {
    size_t length;
    utf8_string utf8ch = utf8encode(ch, &length);
    xstring retval =  xstr_append_cstr(value, (utf8_string) utf8ch);
    free(utf8ch);
    return retval;
}

mstring
mstr_append(mstring value, utf32_char ch) {
	size_t length;
	utf8_string utf8ch = utf8encode(ch, &length);
    mstring retval = mstr_append_cstr(value, (utf8_string) utf8ch);
    free(utf8ch);
    return retval;
}

xstring
xstr_append_f(xstring value, utf32_char ch) {
    size_t length;
    utf8_string utf8ch = utf8encode(ch, &length);
    return  xstr_append_cstr_f(value, (utf8_string) utf8ch);
}

xstring
xstr_append_cstr(xstring value, utf8_string cstr) {
	if (value == NULL) return xstr_wrap(cstr);
	size_t len = (cstr == NULL) ? 0 : strlen((char *) cstr);
	if (len == 0) return value;
	utf8_string newstr = (utf8_string) malloc(len + value->length);
	memcpy(newstr, value->cstr, value->length);
    memcpy(newstr + value->length, cstr, len);
	xstring ret = xstr_new();
	ret->cstr = newstr;
	ret->length = value->length + len;
	return ret;
}

xstring
xstr_append_cstr_f(xstring value, utf8_string cstr) {
	xstring ret = xstr_append_cstr(value, cstr);
	if (cstr != NULL) free(cstr);
	if (value != NULL) xstr_free(value);
	return ret;
}

mstring
mstr_append_cstr(mstring value, utf8_string cstr) {
	if (value == NULL) {
		return mstr_wrap(cstr);
	}
	size_t len = (cstr == NULL) ? 0 : strlen((char *) cstr);
	if (len == 0) return value;
	// Go to the last block of the string.
	mstring here = value;
	while (here->next != NULL) {
		here->length += len;
		here = here->next;
	}
	here->length += len;
	size_t excess = here->local_capacity - here->local_length;
	if (excess >= len) {
		// The current block can hold this string with no
		// additional allocation.  Copy and convert.
        memcpy(here->cstr + here->local_length, cstr, len);
		here->local_length += len;
		return value;
	}
	// There is not enough excess capacity to hold the string.
	if (excess > 0) {
		// We can copy a portion of the string in now.
		here->local_length += excess;
        memcpy(here->cstr + here->local_length, cstr, excess);
		cstr += excess;
	}
	// We must allocate a new block to hold the rest.
	mstring there = mstr_wrap(cstr);
	here->next = there;
	return value;
}

mstring
mstr_append_cstr_f(mstring value, utf8_string cstr) {
	if (cstr != NULL) {
		mstr_append_cstr(value, cstr);
		free(cstr);
	}
	return value;
}

xstring
xstr_concat(xstring first, xstring second) {
	// The user expects to be able to deallocate both strings after
	// this method completes, so we cannot return either one as the
	// value; we have to make a new string in all cases.
	if (second == NULL || second->length == 0) return xstr_copy(first);
	if (first == NULL || first->length == 0) return xstr_copy(second);
	// Neither string is empty, so go ahead and allocate.
	size_t len = first->length + second->length;
	xstring str = xstr_new();
	str->cstr = (xch *) malloc(len);
	memcpy(str->cstr, first->cstr, first->length);
	memcpy(str->cstr + first->length, second->cstr, second->length);
	str->length = len;
	return str;
}

mstring
mstr_concat(mstring first, mstring second) {
	// The user expects first to be modified, and for neither to
	// need to be deallocated.  The two must be "fused" by this
	// operation, unless one is NULL, of course.
	if (second == NULL) return first;
	if (first == NULL) return second;
	mstring here = first;
	// Find the last block and correct the lengths along the way.
	while (here->next != NULL) {
		here->length += second->length;
		here = here->next;
	}
	here->length += second->length;
	// Truncate the capacity of this block.  No need to reallocate
	// any bytes; we just waste them, since it is faster.  If this
	// is a big problem for you, choose a smaller MSTR_INC.
	here->local_capacity = here->local_length;
	here->next = second;
	return first;
}

xstring
xstr_concat_f(xstring first, xstring second) {
	xstring ret = xstr_concat(first, second);
	xstr_free(first);
	xstr_free(second);
	return ret;
}

int
xstr_strcmp(xstring lhs, xstring rhs) {
	if (lhs == rhs) return 0;
	size_t lhslen = (lhs == NULL) ? 0 : lhs->length;
	size_t rhslen = (rhs == NULL) ? 0 : rhs->length;
	if (lhslen == 0) {
		if (rhslen > 0) {
			// Then lhs < rhs.
			return -1;
		}
		// Then lhs = rhs.
		return 0;
	}
	if (rhslen == 0) {
		// Then lhs > rhs.
		return 1;
	}
	xch * lhsp = lhs->cstr;
	xch * rhsp = rhs->cstr;
	while (lhslen > 0) {
		if (rhslen == 0) {
			// The right hand side is exhausted.  The left hand
			// side must come later in the ordering: lhs > rhs.
			return 1;
		}
		// The character types might be any type, so we can't just
		// subtract.  Actually do the comparisons.
		if (*lhsp < *rhsp) return -1;
		if (*lhsp > *rhsp) return 1;
		++lhsp;
		++rhsp;
		--lhslen;
		--rhslen;
	} // Continue until the lhs is exhausted.
	if (rhslen > 0) {
		// The right hand side is longer than the left hand side,
		// and everything else is the same.  Thus lhs < rhs.
		return -1;
	}
	return 0;
}

int
mstr_strcmp(mstring lhs, mstring rhs) {
	if (lhs == rhs) return 0;
	size_t lhslen = (lhs == NULL) ? 0 : lhs->length;
	size_t rhslen = (rhs == NULL) ? 0 : rhs->length;
	if (lhslen == 0) {
		if (rhslen > 0) {
			// Then lhs < rhs.
			return -1;
		}
		// Then lhs = rhs.
		return 0;
	}
	if (rhslen == 0) {
		// Then lhs > rhs.
		return 1;
	}
	// We never use lhslen and rhslen after this point.  Note that
	// we have handled the case of either being NULL above.
	// Move to the first block with content.
	while (lhs != NULL && lhs->local_length == 0) lhs = lhs->next;
	while (rhs != NULL && rhs->local_length == 0) rhs = rhs->next;
	// If we come here then both the strings have non-zero length
	// and we are pointing to a block with content.
	size_t lhsll = (lhs == NULL) ? 0 : lhs->local_length;
	size_t rhsll = (rhs == NULL) ? 0 : rhs->local_length;
	xch * lhsp = (lhs == NULL) ? NULL : lhs->cstr;
	xch * rhsp = (rhs == NULL) ? NULL : rhs->cstr;
	while (lhs != NULL && rhs != NULL) {
		if (*lhsp < *rhsp) return -1;
		if (*lhsp > *rhsp) return 1;
		// Use up the next local characters.
		--lhsll;
		--rhsll;
		++lhsp;
		++rhsp;
		// We have to run both loops so that the logic after this
		// main loop will be correct.
		while (lhsll == 0 && lhs != NULL) {
			// Move to the next block.
			lhs = lhs->next;
			if (lhs != NULL) {
				lhsll = lhs->local_length;
				lhsp = lhs->cstr;
			}
		} // Find the next block with content.
		while (rhsll == 0 && rhs != NULL) {
			// Move to the next block.
			rhs = rhs->next;
			if (rhs != NULL) {
				rhsll = rhs->local_length;
				rhsp = rhs->cstr;
			}
		} // Find the next block with content.
	} // Continue until we find a difference or run out of string.
	if (lhs == NULL) {
		if (rhs != NULL) {
			// Then lhs < rhs.
			return -1;
		}
		// Then lhs = rhs.
		return 0;
	}
	if (rhs == NULL) {
		// Then lhs > rhs.
		return 1;
	}
	return 0;
}

utf8_string
xstr_cstr(xstring value) {
	if (value == NULL || value->length == 0) {
		// We have to return an array the caller can free.
		utf8_string empty = (utf8_string) malloc(1);
		empty[0] = 0;
		return empty;
	}
	utf8_string cstr = (utf8_string) malloc(value->length + 1);
	// For performance handle the special case that the user is
	// using chars.
    memcpy(cstr, value->cstr, value->length);
	// Null terminate.
	cstr[value->length] = 0;
	return cstr;
}

utf8_string
xstr_cstr_f(xstring value) {
	utf8_string ret = xstr_cstr(value);
	xstr_free(value);
	return ret;
}

utf8_string
mstr_cstr(mstring value) {
	if (value == NULL || value->length == 0) {
		// We have to return an array the caller can free.
		utf8_string empty = (utf8_string) malloc(1);
		empty[0] = 0;
		return empty;
	}
	utf8_string cstr = (utf8_string) malloc(value->length + 1);
	mstring here = value;
	utf8_string there = cstr;
	while (here != NULL) {
		if (here->cstr != NULL) {
			// For performance handle the special case that the user is
			// using chars.
            memcpy(there, here->cstr, here->local_length);
		}
		there += here->local_length;
		here = here->next;
	} // Copy all blocks to the new string.
	// Null terminate.
	cstr[value->length] = 0;
	return cstr;
}

utf8_string
mstr_cstr_f(mstring value) {
	utf8_string ret = mstr_cstr(value);
	mstr_free(value);
	return ret;
}

/**
 * Count the code points in a byte sequence.
 * @param bytes		The byte sequence.
 * @param length 	The length of the byte sequence.
 * @return 			The number of code points found, if any.
 */
static size_t
count_code_points(utf8_string bytes, size_t length) {
	size_t points = 0;
	while (length > 0) {
		size_t used = utf8decode_size(bytes);
		bytes += used;
		length -= used;
		++points;
	} // Count all code points.
	return points;
}

static utf32_string
decode(utf8_string bytes, size_t bytelen, size_t * length) {
	if (bytes == NULL) return (utf32_string) calloc(1, sizeof(utf32_char));
	// Convert every character of the xstring into a character.
	size_t scratch = 0;
	if (length == NULL) length = &scratch;
	*length = count_code_points(bytes, bytelen);
	// Allocate the receiving array plus space for the terminating zero.
	utf32_string retval = calloc((1 + *length), sizeof(utf32_char));
	// Convert and copy.
	utf32_string target = retval;
	size_t used = 0;
	for (size_t index = 0; index < *length; ++index) {
		*(target + index) = utf8decode(bytes, &used);
		bytes += used;
	} // Convert and copy.
    return retval;
}

utf32_string
xstr_decode(xstring value, size_t * length) {
	return decode(value->cstr, value->length, length);
}

utf32_string
mstr_decode(mstring value, size_t * length) {
    return decode(mstr_cstr(value), mstr_length(value), length);
}

utf32_string
xstr_decode_f(xstring value, size_t * length) {
    utf32_string retval = xstr_decode(value, length);
    xstr_free(value);
    return retval;
}

utf32_string
mstr_decode_f(mstring value, size_t * length) {
	utf32_string retval = mstr_decode(value, length);
	mstr_free(value);
    return retval;
}

static size_t
count_bytes(utf32_string value, size_t length) {
	if (value == NULL) return 0;
	size_t bytecount = 0;
	while (bytecount < length) {
		size_t count = utf8encode_size(*value);
		bytecount += count;
		++value;
	} // Count bytes.
	return bytecount;
}

static uint8_t *
encode(utf32_string value, size_t length, size_t * used) {
	// Figure out how many bytes to allocate.
	*used = count_bytes(value, length);
	if (*used == 0) return NULL;
	// Allocate.
	uint8_t * retval = (uint8_t *) calloc((1 + *used), sizeof(uint8_t));
	// Convert all the characters.
	for (int index = 0; index < length; ++index) {
		// TODO Implement.
	} // Convert all characters.
	// Done.
	return retval;
}

xstring
xstr_encode(utf32_string value, size_t length) {
	// Figure out how many bytes to allocate.
	size_t count = count_bytes(value, length);
	uint8_t * bytes = calloc((count + 1), sizeof(uint8_t));
	// TODO Implement.
    return NULL;
}

mstring
mstr_encode(utf32_string value, size_t length) {
	// TODO Implement.
    return NULL;
}

xstring
xstr_encode_f(utf32_string value, size_t length) {
    xstring retval = xstr_encode(value, length);
    free(value);
    return retval;
}

mstring
mstr_encode_f(utf32_string value, size_t length) {
    mstring retval = mstr_encode(value, length);
    free(value);
    return retval;
}

struct xstr_iter_ {
	size_t position;
	xstring string;
	utf8_string pointer;
};

struct mstr_iter_ {
	size_t position;
	mstring string;
	utf8_string pointer;
};

xstr_iter
xstr_iterator(xstring xstr) {
	xstr_iter iter = (xstr_iter) malloc(sizeof(struct xstr_iter_));
	iter->position = 0;
	iter->string = xstr;
	iter->pointer = xstr != NULL ? xstr->cstr : NULL;
	return iter;
}

mstr_iter
mstr_iterator(mstring mstr) {
	mstr_iter iter = (mstr_iter) malloc(sizeof(struct mstr_iter_));
	iter->position = 0;
	iter->string = mstr;
	iter->pointer = mstr != NULL ? mstr->cstr : NULL;
	return iter;
}

inline bool
xstr_has_next(xstr_iter iter) {
	return iter->position < iter->string->length;
}

inline bool
mstr_has_next(mstr_iter iter) {
	return iter->position < iter->string->length;
}

utf32_char
xstr_next(xstr_iter iter) {
	// TODO Implement.
	return 0;
}

utf32_char
mstr_next(mstr_iter iter) {
	// TODO Implement.
	return 0;
}
