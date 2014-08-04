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

/* Some notes.
 *
 * Never allocate any space for an xstring unless it is going to
 * contains something.  That is, an empty xstring is equivalent to
 * NULL.
 */

struct xstring_ {
	/* How this works.
	 * The cstr points to a character array of xchars.  It can be
	 * NULL, which should be treated the same as the empty string.
	 * The length is the length of the string.  Storing the length
	 * allows the character array to contain null characters (\0).
	 *
	 * It is assumed throughout this library that if length is not
	 * zero, then cstr is not NULL.  Pay attention to this invariant!
	 */
	xchar * cstr;
	size_t length;
};

struct mstring_ {
	/* How this works.
	 * Mutable strings always allocate memory, even when the string
	 * is empty.  This allows them to be quickly added to, which is
	 * the primary use case for a mutable string.  The cstr points
	 * to a character array of xchars.  It cannot be NULL; it is
	 * always allocated, except when the string is deallocated.  The
	 * local_capacity is the length of the cstr.  The local_length is
	 * the number of characters used in the cstr.  When the capacity
	 * must expand, this is done by adding another new mstring_ struct
	 * via next (a linked list).  The length is the total length of
	 * the string from this block forward along the chain.
	 *
	 * If is assumed throughout this library that if length is not zero
	 * then cstr is not NULL.  Pay attention to this invariant!
	 */
	xchar * cstr;
	size_t local_capacity;
	size_t local_length;
	size_t length;
	struct mstring_ * next;
};

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
	empty->cstr = (xchar *) malloc(sizeof(xchar) * capacity);
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
xstr_wrap(char * value) {
	size_t len = (value == NULL) ? 0 : strlen(value);
	// If the length is zero, don't allocate anything.
	if (len == 0) return NULL;
	xstring str = xstr_new();
	str->length = len;
	str->cstr = (xchar *) malloc(len * sizeof(xchar));
	for (size_t index = 0; index < len; ++index) {
		xchar ch = value[index];
		str->cstr[index] = ch;
	} // Copy all characters, converting if necessary.
	return str;
}

xstring
xstr_wrap_f(char * value) {
	xstring ret = xstr_wrap(value);
	if (value != NULL) free(value);
	return ret;
}

mstring
mstr_wrap(char * value) {
	size_t len = value == NULL ? 0 : strlen(value);
	mstring str = mstr_new(len + MSTR_INC);
	if (value == NULL) return str;
	str->length = len;
	for (size_t index = 0; index < str->length; ++index) {
		xchar ch = value[index];
		str->cstr[index] = ch;
	} // Copy all characters, converting if necessary.
	return str;
}

mstring
mstr_wrap_f(char * value) {
	mstring ret = mstr_wrap(value);
	if (value != NULL) free(value);
	return ret;
}

xstring
xstr_copy(xstring other) {
	if (other == NULL || other->length == 0) return NULL;
	xstring str = xstr_new();
	str->length = other->length;
	str->cstr = (xchar *) malloc(other->length * sizeof(xchar));
	memcpy(str->cstr, other->cstr, str->length * sizeof(xchar));
	return str;
}

mstring
mstr_copy(mstring other) {
	size_t len = other == NULL ? 0 : other->length;
	mstring str = mstr_new(len + MSTR_INC);
	if (len == 0) return str;
	str->length = other->length;
	mstring here = other;
	xchar * there = str->cstr;
	while (here != NULL) {
		if (here->cstr != NULL)
			memcpy(there, here->cstr, here->local_length * sizeof(xchar));
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
	ret->cstr = (xchar *) malloc(sizeof(xchar) * len);
	mstring here = other;
	xchar * there = ret->cstr;
	while (here != NULL) {
		if (here->cstr != NULL)
			memcpy(there, here->cstr, here->local_length * sizeof(xchar));
		there += here->local_length;
		here = here->next;
	} // Copy all blocks to the new string.
	return ret;
}

mstring
xstr_to_mstr(xstring other) {
	size_t len = other == NULL ? 0 : other->length;
	mstring ret = mstr_new(len + MSTR_INC);
	if (other == NULL || other->length == 0) return ret;
	memcpy(ret->cstr, other->cstr, len * sizeof(xchar));
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
xstr_append(xstring value, xchar ch) {
	// We are appending a character, so the resulting string will not
	// be empty.  Go ahead and allocate it.
	xstring str = xstr_new();
	if (value == NULL || value->length == 0) {
		str->length = 1;
		str->cstr = (xchar *) malloc(sizeof(xchar));
		str->cstr[0] = ch;
	} else {
		str->length = value->length + 1;
		str->cstr = (xchar *) malloc(str->length * sizeof(xchar));
		memcpy(str->cstr, value->cstr, value->length * sizeof(xchar));
		str->cstr[value->length] = ch;
	}
	return str;
}

mstring
mstr_append(mstring value, xchar ch) {
	if (value == NULL) {
		mstring str = mstr_new(0);
		str->length = 1;
		str->local_length = 1;
		str->cstr[0] = ch;
		return str;
	} else {
		// Find the last block of the string, since that is where
		// we need to append.
		value->length++;
		mstring here = value;
		while (here->next != NULL) here = here->next;
		if (here->local_length >= value->local_capacity) {
			// We have to create a new block at this point.
			here->next = mstr_new(0);
			here = here->next;
		}
		here->cstr[here->local_length] = ch;
		here->local_length += 1;
		here->length += 1;
		return value;
	}
}

xstring
xstr_append_f(xstring value, xchar ch) {
	xstring ret = xstr_append(value, ch);
	xstr_free(value);
	return ret;
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
	str->cstr = (xchar *) malloc(sizeof(xchar) * len);
	memcpy(str->cstr, first->cstr, first->length * sizeof(xchar));
	memcpy(str->cstr + first->length, second->cstr,
			second->length * sizeof(xchar));
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
	// Find the last block.
	while (here->next != NULL) here = here->next;
	// Truncate the capacity of this block.  No need to reallocate
	// any bytes; we just waste them, since it is faster.  If this
	// is a big problem for you, choose a smaller MSTR_INC.
	here->local_capacity = here->local_length;
	here->next = second;
	first->length += second->length;
	return first;
}

xstring
xstr_concat_f(xstring first, xstring second) {
	xstring ret = xstr_concat(first, second);
	xstr_free(first);
	xstr_free(second);
	return ret;
}

xchar
xstr_char(xstring value, size_t index) {
	if (value == NULL) return 0;
	if (index >= value->length) return 0;
	return value->cstr[index];
}

xchar
mstr_char(mstring value, size_t index) {
	if (value == NULL) return 0;
	if (index >= value->length) return 0;
	while (index >= value->local_length && value->next != NULL) {
		index -= value->local_length;
		value = value->next;
	} // Find the segment that contains the character.
	if (index >= value->local_length) {
		return 0;
	}
	return value->cstr[index];
}

xstring
xstr_substr(xstring value, size_t start, size_t num) {
	if (num == 0 || value == NULL) return NULL;
	// The requested substring is not empty, so allocate.
	xstring str = xstr_new();
	str->cstr = (xchar *) malloc(sizeof(xchar) * num);
	str->length = num;
	// Now there are two ranges.  The first is characters that are
	// in the original string and should be copied.  The second is
	// indices that are past the end of the original string, and
	// should be set to zero.  This happens at index value->length.
	size_t second = start + num - value->length;
	size_t first = second <= 0 ? num : num - second;
	if (first > 0) {
		memcpy(str->cstr, value->cstr + start, sizeof(xchar) * first);
	}
	if (second > 0) {
		memset(str->cstr + first, 0, sizeof(xchar) * second);
	}
	// Some cases to show the above calculations are correct.
	// There are three cases:
	// + start and end are in range.
	// + start is in range, but end is not.
	// + both start and end are out of range.
	//
	// value->length   start   num   first   second
	// 10              2       4     4       -4
	// 10              2       9     8       1
	// 10              10      4     0       4
	return str;
}

mstring
mstr_substr(mstring value, size_t start, size_t num) {
	// TODO This is a slightly inefficient way to do this.
	mstring str = mstr_new(num + MSTR_INC);
	if (num == 0 || value == NULL) return str;
	for (size_t index = 0; index < num; ++index) {
		str->cstr[index] = mstr_char(value, index + start);
	} // Gather the substring.
	return str;
}

xstring
xstr_substr_f(xstring value, size_t start, size_t num) {
	xstring ret = xstr_substr(value, start, num);
	xstr_free(value);
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
	xchar * lhsp = lhs->cstr;
	xchar * rhsp = rhs->cstr;
	while (lhslen > 0) {
		if (rhslen == 0) {
			// The right hand side is exhausted.  The left hand
			// side must come later in the ordering: lhs > rhs.
			return 1;
		}
		// The character types might be any type, so we can't just
		// subtract.  Actually do the comparisons.
		if (lhsp < rhsp) return -1;
		if (lhsp > rhsp) return 1;
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
	size_t lhslen = lhs == NULL ? 0 : lhs->length;
	size_t rhslen = rhs == NULL ? 0 : rhs->length;
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
	size_t lhsll = lhs->local_length;
	size_t rhsll = rhs->local_length;
	xchar * lhsp = lhs->cstr;
	xchar * rhsp = rhs->cstr;
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

char *
xstr_cstr(xstring value) {
	if (value == NULL || value->length == 0) {
		// We have to return an array the caller can free.
		char * empty = (char *) malloc(1);
		empty[0] = 0;
		return empty;
	}
	char * cstr = (char *) malloc(value->length + 1);
	// For performance handle the special case that the user is
	// using chars.
	if (sizeof(xchar) == sizeof(char)) {
		memcpy(cstr, value->cstr, value->length);
	} else {
		// We have to copy so each character is downconverted to a
		// simple char.
		for (size_t index = 0; index < value->length; ++index) {
			cstr[index] = (char) xstr_char(value, index);
		} // Convert and copy all characters.
	}
	// Null terminate.
	cstr[value->length] = 0;
	return cstr;
}

char *
xstr_cstr_f(xstring value) {
	char * ret = xstr_cstr(value);
	xstr_free(value);
	return ret;
}

char *
mstr_cstr(mstring value) {
	if (value == NULL || value->length == 0) {
		// We have to return an array the caller can free.
		char * empty = (char *) malloc(1);
		empty[0] = 0;
		return empty;
	}
	char * cstr = (char *) malloc(value->length + 1);
	mstring here = value;
	xchar * there = cstr;
	while (here != NULL) {
		if (here->cstr != NULL)
			memcpy(there, here->cstr, here->local_length * sizeof(char));
		there += here->local_length;
		here = here->next;
	} // Copy all blocks to the new string.
	// Null terminate.
	cstr[value->length] = 0;
	return cstr;
}

char *
mstr_cstr_f(mstring value) {
	char * ret = mstr_cstr(value);
	mstr_free(value);
	return ret;
}

wchar_t *
xstr_wcstr(xstring value) {
	if (value == NULL || value->length == 0) {
		// We have to return something the caller can free.
		wchar_t * empty = (wchar_t *) malloc(sizeof(wchar_t));
		empty[0] = 0;
		return empty;
	}
	wchar_t * wcstr = (wchar_t *) malloc(sizeof(wchar_t) * (value->length + 1));
	// For performance handle the special case that the user is
	// using wchar_t.
	if (sizeof(xchar) == sizeof(wchar_t)) {
		memcpy(wcstr, value->cstr, value->length * sizeof(wchar_t));
	} else {
		// We have to copy so each character is converted to a
		// wide character.
		for (size_t index = 0; index < value->length; ++index) {
			wcstr[index] = (wchar_t) xstr_char(value, index);
		} // Convert and copy all characters.
	}
	// Null terminate.
	wcstr[value->length] = 0;
	return wcstr;
}

wchar_t *
xstr_wcstr_f(xstring value) {
	wchar_t * ret = xstr_wcstr(value);
	xstr_free(value);
	return ret;
}
