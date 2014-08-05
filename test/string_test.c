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

int raw_test();

int main(int argc, char * argv[]) {
	// Run the raw test to get a baseline.  If this fails then there
	// is really no reason to continue at all.
	int err_count = raw_test();
	if (err_count > 0) {
		fprintf(stderr, "Stopping due to errors in raw test.\n");
	}
}

int raw_test() {
	int error_count = 0;
	int size = 6;
	char * strings[] = {
			NULL,
			"",
			NULL,
			"\"Right,\" said ",
			"Fred.",
			"This is a larger string that is long enough that "
			"it will cross a boundary (for the usual byte boundary) "
			"and contain more than one block.  In fact, it will "
			"probably contain more than two blocks.  This is good, "
			"since we need examples of that."
	};
	// Make some strings.
	xstring x[size];
	mstring m[size];
	char * cx[size];
	for (int index = 0; index < size; ++index) {
		x[index] = xstr_wrap(strings[index]);
		m[index] = mstr_wrap(strings[index]);
		mstr_inspect(m[index]);
		cx[index] = xstr_cstr(x[index]);
	}
	// Perform comparisons.
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			int v1 = xstr_strcmp(x[i], x[j]);
			int v2 = strcmp(cx[i], cx[j]);
			printf("(%d,%d)=(%d,%d) ", i, j, v1, v2);
			if ((v1 < 0 && v2 < 0) ||
					(v1 > 0 && v2 > 0) ||
					(v1 == 0 && v2 == 0)) {
				printf("OK   ");
			} else {
				printf("ERR  ");
				++error_count;
			}
		}
		printf("\n");
	}
	// Print the strings.
	for (int index = 0; index < size; ++index) {
		printf("%d: Length %ld: ", index, xstr_length(x[index]));
		XPRINT(stdout, x[index]);
		printf("\n");
	}
	// Copy some strings.
	xstring y[size];
	char * cy[size];
	for (int index = 0; index < size; ++index) {
		y[index] = xstr_copy(x[index]);
		cy[index] = xstr_cstr(y[index]);
		if (xstr_strcmp(y[index], x[index]) != 0) {
			printf("String copy error.");
			++error_count;
		}
	}
	// Perform comparisons.
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			int v1 = xstr_strcmp(y[i], x[j]);
			int v2 = strcmp(cx[i], cy[j]);
			printf("(%d,%d)=(%d,%d) ", i, j, v1, v2);
			if ((v1 < 0 && v2 < 0) ||
					(v1 > 0 && v2 > 0) ||
					(v1 == 0 && v2 == 0)) {
				printf("OK   ");
			} else {
				printf("ERR  ");
				++error_count;
			}
		}
		printf("\n");
	}
	for (int index = 0; index < size; ++index) {
		printf("Length %ld: ", xstr_length(y[index]));
		XPRINT(stdout, y[index]);
		printf("\n");
	}
	// Free the C strings.
	for (int index = 0; index < size; ++index) {
		free(cx[index]);
		cx[index] = NULL;
		free(cy[index]);
		cy[index] = NULL;
	}
	// This should not cause a memory leak.
	xstr_free(xstr_wrap_f(xstr_cstr(x[5])));
	// Concatenate some strings.
	xstring c1 = xstr_concat(x[0], x[1]);
	if (c1 != NULL) {
		printf("First empty concatenation does not yeild NULL.\n");
	}
	xstr_free(c1);
	c1 = xstr_concat(x[0], x[2]);
	if (c1 != NULL) {
		printf("Second empty concatenation does not yeild NULL.\n");
	}
	xstr_free(c1);
	c1 = xstr_concat(x[0], x[3]);
	if (xstr_strcmp(x[3], c1) != 0) {
		printf("Third concatenation failed.\n");
	}
	xstr_free(c1);
	c1 = xstr_concat(x[3], x[4]);
	char * c2 = xstr_cstr(c1);
	if (strcmp(c2, "\"Right,\" said Fred.") != 0) {
		printf("Fourth concatenation failed.\n");
	}
	free(c2);
	xstr_free(c1);
	// Build some mstrings.
	for (int index = 0; index < size; ++index) {
		mstr_free(m[index]);
		m[index] = xstr_to_mstr(x[index]);
		xstring back = mstr_to_xstr(m[index]);
		cx[index] = mstr_cstr(m[index]);
		if (xstr_strcmp(back, x[index]) != 0) {
			printf("Round trip error at index %d.\n", index);
			printf("  --> %s\n", cx[index]);
		}
		xstr_free(back);
	}
	for (int index = 0; index < size; ++index) {
		printf("Length %ld: %s\n", mstr_length(m[index]), cx[index]);
		free(cx[index]);
		cx[index] = NULL;
	}
	// Done.
	for (int index = 0; index < size; ++index) {
		xstr_free(x[index]);
		xstr_free(y[index]);
		x[index] = NULL;
		y[index] = NULL;
		mstr_free(m[index]);
		m[index] = NULL;
	}
	// Concatenate test.  This should cause no memory leaks.
	xstring xx = xstr_wrap("Counting down:");
	mstring mm = mstr_wrap("Counting down:");
	mstring mm2 = mstr_new(0);
	mstr_append_cstr(mm2, "Counting down:");
	for (int index = 10; index > 0; --index) {
		char buf[10];
		sprintf(buf, " %d", index);
		xstring xadd = xstr_wrap(buf);
		mstring madd = mstr_wrap(buf);
		xstring xy = xstr_concat(xx, xadd);
		mstr_concat(mm, madd);
		mstr_append_cstr(mm2, buf);
		xstr_free(xx);
		xstr_free(xadd);
		xx = xy;
		mstring mconv = xstr_to_mstr(xx);
		if (mstr_strcmp(mconv, mm) != 0) {
			++error_count;
			fprintf(stderr, "Failed in concatenate test.\n");
		}
		if (mstr_strcmp(mconv, mm2) != 0) {
			++error_count;
			fprintf(stderr, "Failed in concatenate test.\n");
		}
		mstr_free(mconv);
	}
	MPRINT(stdout, mm); printf("\n");
	MPRINT(stdout, mm2); printf("\n");
	mstr_free(mm);
	mstr_free(mm2);
	xstr_free(xx);
	// Done.
	return error_count;
}
