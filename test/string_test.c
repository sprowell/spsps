/*
 * @file
 * Public interface to the simple immutable string.
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

#define MSTRING_DEBUG 1

#include "xstring.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "test_frame.h"

START_TEST;

	// Define some C strings to work with.
	int size = 6;
	utf8_string strings[] = {
			/* 0 */ NULL,
			/* 1 */ "",
			/* 2 */ NULL,
			/* 3 */ "\"Right,\" said ",
			/* 4 */ "Fred.",
			/* 5 */ "This is a larger string that is long enough that "
					"it will cross a boundary (for the usual byte boundary) "
					"and contain more than one block.  In fact, it will "
					"probably contain more than two blocks.  This is good, "
					"since we need examples of that."
	};
	mstring m[size];
	xstring x[size];
	xstring y[size];
	xstring x1;
	xstring x2;
	utf8_string cm[size];
	utf8_string cx[size];
	utf8_string cy[size];
	utf8_string cc1;

	START_ITEM(roundtrip);
	// Create immutable and mutable strings, and convert both to
	// C strings so we test the "round trip."  After this point both
	// m and x are populated, along with cm and cx.
	for (int index = 0; index < size; ++index) {
		size_t len = strlen(strings[index]);
		m[index] = mstr_wrap(strings[index]);
		if (mstr_length(m[index]) != len) {
			FAIL("The mstring %d has length %lu, but the original string "
					"has length %lu.", index, mstr_length(m[index]), len);
		}
		x[index] = xstr_wrap(strings[index]);
		if (xstr_length(x[index]) != len) {
			FAIL("The xstring %d has length %lu, but the original string "
					"has length %lu.", index, xstr_length(x[index]), len);
		}
		cm[index] = mstr_cstr(m[index]);
		if (strcmp(cm[index], strings[index]) != 0) {
			FAIL("The mstring %d was converted to a C string, but it "
					"does not match the original.\n"
					"  -> Converted: %s\n"
					"  -> Original: %s", index, cm[index], strings[index]);
		}
		cx[index] = xstr_cstr(x[index]);
		if (strcmp(cx[index], strings[index]) != 0) {
			FAIL("The xstring %d was converted to a C string, but it "
					"does not match the original.\n"
					"  -> Converted: %s\n"
					"  -> Original: %s", index, cx[index], strings[index]);
		}
	} // Initial basic test.
	IF_FAIL_STOP;
	END_ITEM(roundtrip);

	START_ITEM(nullstring);
	// Make sure we get NULL if we have an empty string.
	for (int index = 0; index < size; ++index) {
		if (strlen(strings[index]) == 0) {
			if (x[index] != NULL) {
				FAIL("The xstring %d incorrectly allocated space even "
						"though the string is empty.", index);
			}
		}
	} // Check for worthless allocations.
	IF_FAIL_STOP;
	END_ITEM(nullstring);

	START_ITEM(compare);
	// Perform comparisons.  This performs all cross comparisons
	// and uses the standard strcmp of the raw strings to validate
	// the comparison results.
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			int v1 = xstr_strcmp(x[i], x[j]);
			int v2 = strcmp(strings[i], strings[j]);
			if ((v1 < 0 && v2 < 0) ||
					(v1 > 0 && v2 > 0) ||
					(v1 == 0 && v2 == 0)) {
				// Nothing to do.
			} else {
				FAIL("The xstrings %d and %d compare with result %d, "
						"but the original strings compare with %d.",
						i, j, v1, v2);
			}
		} // Inner compare loop.
	} // Outer compare loop.
	IF_FAIL_STOP;
	END_ITEM(compare);

	START_ITEM(copy);
	// Copy some strings.
	for (int index = 0; index < size; ++index) {
		y[index] = xstr_copy(x[index]);
		if (xstr_strcmp(y[index], x[index]) != 0) {
			FAIL("The string %d was copied, but the xstring compare"
					"reports that the original and copy do not match.",
					index);
		}
		xstr_free(y[index]);
		y[index] = NULL;
	} // Loop over string copy.
	IF_FAIL_STOP;
	END_ITEM(copy);

	START_ITEM(leak1);
	// This should not cause a memory leak thanks to the automated
	// free in the inner operation.
	xstr_free(xstr_wrap_f(xstr_cstr(x[5])));
	END_ITEM(leak1);

	START_ITEM(concatenate);
	// Concatenate some strings.  The empty string is NULL.  We don't
	// want to allocate space unless we need to.
	xstring c1 = xstr_concat(x[0], x[1]);
	if (c1 != NULL) {
		FAIL("Concatenation of empty strings does not yield NULL.");
	}
	xstr_free(c1);
	c1 = xstr_concat(x[0], x[3]);
	cc1 = xstr_cstr(c1);
	if (xstr_strcmp(x[3], c1) != 0) {
		FAIL("Concatenation of strings 0 and 3 did not yield the "
				"expected result.\n  --> Expected: %s%s\n  --> Got: %s",
				strings[0], strings[3], cc1);
	}
	free(cc1);
	xstr_free(c1);
	c1 = xstr_concat(x[3], x[4]);
	cc1 = xstr_cstr(c1);
	if (strcmp(cc1, "\"Right,\" said Fred.") != 0) {
		FAIL("Concatenation of strings 3 and 4 did not yield the "
				"expected result.\n  --> Expected: %s%s\n  --> Got: %s",
				strings[3], strings[4], cc1);
	}
	free(cc1);
	xstr_free(c1);
	IF_FAIL_STOP;
	END_ITEM(concatenate);

	START_ITEM(leak2);
	// Deallocate.  After this, everything is freed.
	for (int index = 0; index < size; ++index) {
		mstr_free(m[index]); m[index] = NULL;
		xstr_free(x[index]); x[index] = NULL;
		free(cm[index]); cm[index] = NULL;
		free(cx[index]); cx[index] = NULL;
	} // Deallocate loop.
	END_ITEM(leak2);

	START_ITEM(roundtrip2);
	// Perform round-trip testing, converting between xstring and mstring.
	for (int index = 0; index < size; ++index) {
		x[index] = xstr_wrap(strings[index]);
		m[index] = xstr_to_mstr(x[index]);
		y[index] = mstr_to_xstr(m[index]);
		if (xstr_strcmp(x[index], y[index]) != 0) {
			cc1 = xstr_cstr(y[index]);
			FAIL("Round-trip testing failed for string %d.\n"
					"  --> Original: %s\n"
					"  --> Got: %s", index, strings[index], cc1);
		}
	} // Round-trip testing loop.
	END_ITEM(roundtrip2);

	START_ITEM(leak3);
	// Deallocate everything.
	for (int index = 0; index < size; ++index) {
		mstr_free(m[index]); m[index] = NULL;
		xstr_free(x[index]); x[index] = NULL;
		xstr_free(y[index]); y[index] = NULL;
	} // Deallocate loop.
	END_ITEM(leak3);

	START_ITEM(xappend);
	// Append test with xstrings.  These are immutable, so we
	// have to free each time through.
	x1 = NULL;
	x2 = xstr_append_cstr(x1, "Counting down:");
	xstr_free(x1);
	for (int index = 9; index >= 0; --index) {
		x1 = xstr_append(x2, '0'+index);
		xstr_free(x2);
		x2 = x1;
	} // Countdown loop.
	x2 = xstr_append_cstr(x1, " Done!");
	xstr_free(x1);
	x1 = xstr_wrap("Counting down:9876543210 Done!");
	if (xstr_strcmp(x1, x2) != 0) {
		char * cx1 = xstr_cstr(x1);
		char * cx2 = xstr_cstr(x2);
		FAIL("Append generated the wrong xstring.\n"
				"  --> Expected: %s\n"
				"  --> Got: %s", cx1, cx2);
		free(cx1);
		free(cx2);
	}
	xstr_free(x1);
	xstr_free(x2);
	END_ITEM(xappend);

	START_ITEM(mappend);
	// Append test with mstrings.  These are mutable, so we
	// do not free.
	mstring m1 = NULL;
	mstr_append_cstr(m1, "Counting down:");
	for (int index = 9; index >= 0; --index) {
		mstr_append(m1, '0'+index);
	} // Countdown loop.
	mstr_append_cstr(m1, " Done!");
	mstring m2 = mstr_wrap("Counting down:9876543210 Done!");
	if (mstr_length(m1) != mstr_length(m2)) {
		FAIL("Append generated the wrong length of string.\n"
			"  --> Expected: %ld\n"
			"  --> Got: %ld", mstr_length(m1), mstr_length(m2));
	}
	if (mstr_strcmp(m1, m2) != 0) {
		char * cm1 = mstr_cstr(m1);
		char * cm2 = mstr_cstr(m2);
		FAIL("Append generated the wrong mstring.\n"
				"  --> Expected: %s\n"
				"  --> Got: %s", cm1, cm2);
		free(cm1);
		free(cm2);
	}
	mstr_free(m1);
	mstr_free(m2);

	// Now append to mstrings again, but sufficiently that we cross
	// a boundary.
	m1 = NULL;
	x1 = xstr_wrap("100");
	mstr_append_cstr(m1, "100");
	for (int index = 99; index >= 0; --index) {
		char buf[10];
		sprintf(buf, ", %d", index);
		mstr_append_cstr(m1, buf);
		x2 = xstr_append_cstr(x1, buf);
		xstr_free(x1);
		x1 = x2;
	}
	mstr_free(m1);
	xstr_free(x1);
	END_ITEM(mappend);

END_TEST;
