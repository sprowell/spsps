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

#include "xstring.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[]) {
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
	for (int index = 0; index < size; ++index) {
		x[index] = xstr_wrap(strings[index]);
	}
	// Get C strings.
	char * cs[size];
	for (int index = 0; index < size; ++index) {
		cs[index] = xstr_cstr(x[index]);
	}
	// Perform comparisons.
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			int v1 = xstr_strcmp(x[i], x[j]);
			int v2 = strcmp(cs[i], cs[j]);
			printf("(%d,%d)=(%d,%d) ", i, j, v1, v2);
			if ((v1 < 0 && v2 < 0) ||
					(v1 > 0 && v2 > 0) ||
					(v1 == 0 && v2 == 0)) {
				printf("OK   ");
			} else {
				printf("ERR  ");
			}
		}
		printf("\n");
	}
	// Print lengths.
	for (int index = 0; index < size; ++index) {
		printf("%d: Length %ld: %s\n", index, xstr_length(x[index]), cs[index]);
		free(cs[index]);
		cs[index] = NULL;
	}
	// Copy some strings.
	xstring y[size];
	for (int index = 0; index < size; ++index) {
		y[index] = xstr_copy(x[index]);
		cs[index] = xstr_cstr(y[index]);
	}
	for (int index = 0; index < size; ++index) {
		printf("Length %ld: %s\n", xstr_length(y[index]), cs[index]);
		free(cs[index]);
		cs[index] = NULL;
	}
	// This should not cause a memory leak.
	xstr_free(xstr_wrap_f(xstr_cstr(x[5])));
	// Build some mstrings.
	mstring m[size];
	for (int index = 0; index < size; ++index) {
		m[index] = xstr_to_mstr(x[index]);
		xstring back = mstr_to_xstr(m[index]);
		cs[index] = mstr_cstr(m[index]);
		if (xstr_strcmp(back, x[index]) != 0) {
			printf("Round trip error at index %d.", index);
		}
		xstr_free(back);
	}
	for (int index = 0; index < size; ++index) {
		printf("Length %ld: %s\n", mstr_length(m[index]), cs[index]);
		free(cs[index]);
		cs[index] = NULL;
	}
	// Done.
	for (int index = 0; index < size; ++index) {
		xstr_free(x[index]);
		x[index] = NULL;
		xstr_free(y[index]);
		y[index] = NULL;
		mstr_free(m[index]);
		m[index] = NULL;
	}
}
