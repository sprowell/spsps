/*
 * @file
 * Unit tests for the UTF-8 conversions.
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

#include "test_frame.h"
#include "xstring.h"

START_TEST;

START_ITEM(null);
	xstring xs = NULL;

	// Try to free the string.  Should do nothing.
	xstr_free(xs);

	// Copy this string.  Should yield NULL.
	xstring xsc = xstr_copy(xs);
	if (xsc != NULL) {
		FAIL("Copied empty string should be NULL, but is not.");
	}

	// Length of string should be zero.
	size_t len = xstr_length(xs);
	if (len != 0) {
		FAIL("Length of NULL xstring should be 0, but is %ld.", len);
	}

	// Concatenating NULLs should yield NULL.
	xsc = xstr_concat(NULL, NULL);
	if (xsc != NULL) {
		FAIL("Concatenating NULLs yields non-NULL xstring.");
	}
	xsc = xstr_concat_f(NULL, NULL);
	if (xsc != NULL) {
		FAIL("Concatenating-with-free NULLs yields non-NULL xstring.");
	}

	// NULLs are the same string.
	if (xstr_strcmp(NULL, NULL) != 0) {
		FAIL("Compare of NULL strings does not yield 0.");
	}

	// Convert to a C string.
	utf8_string cstr = xstr_cstr(xs);
	if (cstr == NULL) {
		FAIL("Converting a NULL xstring to a C string yields NULL.");
	} else {
		len = strlen(cstr);
		if (len != 0) {
			FAIL("Converting a NULL xstring to a C string gives strlen %ld.",
				len);
		}
		free(cstr);
	}
	cstr = xstr_cstr_f(xs);
	if (cstr == NULL) {
		FAIL("Converting-with-free a NULL xstring to a C string yields NULL.");
	} else {
		len = strlen(cstr);
		if (len != 0) {
			FAIL("Converting-with-free a NULL xstring to a C string gives "
				"strlen %ld.", len);
		}
		free(cstr);
	}

	// Test conversion from mstring.
	if (mstr_to_xstr(NULL) != NULL) {
		FAIL("Premature allocation of xstring on conversion from mstring.");
	}

	// Test conversion from C string.
	if (xstr_wrap("") != NULL) {
		FAIL("Premature allocation of xstring on conversion from C string.");
	}

	// // Test decoding.
	utf32_string u32 = xstr_decode(xs, &len);
	if (u32 == NULL) {
		FAIL("Decoding of empty xstring yielded NULL.");
	} else {
		if (len != 0) {
			FAIL("Decoding of empty xstring yielded %ld code points.", len);
		}
		free(u32);
	}
END_ITEM;

START_ITEM(strings);
	utf8_string strings[] = {
		"",
		"\"Right,\" said Fred.",
		"     ",
		"This is a longer string.",
		"κόσμε",
	};

	// Convert to xstrings.
	size_t len = sizeof(strings) / sizeof(utf8_string);
	xstring * xstrings = (xstring *) malloc(sizeof(xstring) * len);
	for (size_t index = 0; index < len; ++index) {
		// Wrap the string.
		xstrings[index] = xstr_wrap(strings[index]);

		// Convert back to C string and check.
		utf8_string cstr = xstr_cstr(xstrings[index]);
		if (strcmp(strings[index], cstr) != 0) {
			FAIL("Round trip of %s fails; yields %s.", cstr, strings[index]);
		}

		// Check length.
		if (xstr_length(xstrings[index]) != strlen(strings[index])) {
			FAIL("Length of xstring is wrong (%ld for %s).",
				xstr_length(xstrings[index]), cstr);
		}
	} // Convert all strings.

	// Free everything.
	for (size_t index = 0; index < len; ++index) {
		xstr_free(xstrings[index]);
	} // Free all xstrings.
END_ITEM;

END_TEST;
