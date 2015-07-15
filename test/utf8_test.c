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
#include <wctype.h> // For wchar_t and wint_t.
#include <locale.h> // For setlocale.
#include <utf8.h>

START_TEST;

// If we don't set the locale, things won't work.
setlocale(LC_ALL, "en_US.UTF-8");

START_ITEM(checks);
// Test all the predicates.
for (wchar_t value = 0; value != 65535; ++value) {
	int v1 = iswcntrl(value);
	int v2 = is_ISO_control(value);
	// Skip this check for certain values.
	if (value < 0x0100) {
		if (v1 != v2) {
			FAIL("Error for code point U+%04X: "
				"iswcntrl = %d, is_ISO_control = %d",
				value, v1, v2);
		}
	}
	v1 = iswspace(value);
	v2 = is_whitespace(value);
	// Skip this check for U+200B, the zero-length space.
	if (value != 0x200B) {
		if (v1 != v2) {
			FAIL("Error for code point U+%04X: "
				"iswspace = %d, is_whitespace = %d",
				value, v1, v2);
		}
	}
} // Test every wide character.
END_ITEM;

START_ITEM(conversion);
// Test conversion by "round tripping" the values.  Watch out for invalid
// Unicode values.
utf8_string encoded = NULL;
uint32_t testvalue = 0;
size_t usedE = 0;
size_t usedD = 0;
for (uint32_t value = 0; value < 0x110000; ++value) {
	// UTF-32 to UTF-8.
	encoded = utf8encode(value, &usedE);
	// UTF-8 to UTF-32.
	testvalue = utf8decode(encoded, &usedD);
	// Check the result.
	if (usedE != usedD) {
		FAIL("Error for code point u+%08X: "
			"encoded as %ld bytes, decoded %ld bytes",
			value, usedE, usedD);
	}
	if (value != testvalue) {
		FAIL("Error for code point u+%08X: "
			"encode/decode yielded u+%08X",
			value, testvalue);
	}
	IF_FAIL {
		if (usedE > 0) {
			WRITE("Encoded u+%08X as:", value);
			for (size_t index = 0; index < usedE; ++index) {
				WRITE("%02X", encoded[index]);
			} // Write the encoded characters.
			WRITELN("");
		}
	}
	// Discard the bytes.
	free(encoded);
	IF_FAIL_STOP;
} // Test all the valid Unicode range.
END_ITEM;

START_ITEM(badencode);
// Encode bad code points.
uint32_t value = 0x110000;
uint32_t oldvalue = 0;
while (value > oldvalue) {
	uint32_t testvalue = 0;
	size_t used = 0;
	utf8_string encoded = utf8encode(value, &used);
	if (used != 0) {
		FAIL("utf8encode did not detect bad code point u+%08X", value);
	}
	free(encoded);
	IF_FAIL_STOP;
	oldvalue = value;
	value += rand() % 200 + 1;
} // Randomly sample from the rest of the space.
END_ITEM;

START_ITEM(baddecode);
// Make an invalid byte stream.
char encoded[][6] = {
	{0xF9, 0x21, 0x23, 0x32, 0x81, 0x43,}, // F9 is an invalid start.
	{0xF1, 0x80, 0xC0, 0x80, 0x81, 0x43,}, // C0 is an invalid intermediate.
};
for (int index = 0; index < 2; ++index) {
	uint32_t result = utf8decode(encoded[index], NULL);
	if ((result & 0xFF00) != 0xDC00) {
		FAIL("utf8decode did not detect bad encoding");
		WRITE("Bytes:")
		for (size_t index2 = 0; index2 < 6; ++index2) {
			WRITE("%02X", encoded[index][index2]);
		} // Write the encoded characters.
		WRITELN("");
	}
	IF_FAIL_STOP;
} // Try all bad encodings.
END_ITEM;

END_TEST;
