/**
 * @file
 * Support for working with UTF-8.
 *
 * @verbatim
 * SPSPS
 * Stacy's Pathetically Simple Parsing System
 * https://github.com/sprowell/spsps
 *
 * Copyright (c) 2015, Stacy Prowell
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

#include <stdlib.h>
#include "utf8.h"

uint8_t *
utf8encode(spsps_char code_point, size_t * used) {
    size_t scratch = 0;
    if (used == NULL) used = &scratch;

    /*
     * This function relies on the following table taken from RFC 3629.
     *
     * Char. number range  |        UTF-8 octet sequence
     *    (hexadecimal)    |              (binary)
     * --------------------+---------------------------------------------
     * 0000 0000-0000 007F | 0xxxxxxx
     * 0000 0080-0000 07FF | 110xxxxx 10xxxxxx
     * 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
     * 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     *
     * Interpreting this gives:
     * < 0x80 --> The byte itself.
     * < 0x800 --> 11 bits, broken into:
     *   0xC0 | (code_point >> 6),
     *   0x80 | code_point & 0x3F
     * < 0x1000 --> 16 bits, broken into:
     *   0xE0 | (code_point >> 12),
     *   0x80 | (code_point >> 6) & 0x3F,
     *   0x80 | code_point & 0x3F
     * < 0x110000 --> 21 bits, broken into:
     *   0xF0 | (code_point >> 18),
     *   0x80 | (code_point >> 12) & 0x3F,
     *   0x80 | (code_point >> 6) & 0x3F,
     *   0x80 | code_point & 0x3F
     * >= 0x11000 --> Invalid
     */

    // Using calloc prevents the need to zero-terminate the array.
    uint8_t * value = (uint8_t *) calloc(7, 1);
    if (code_point < 0x80) {
        // This is a single-byte encoding.
        value[0] = (uint8_t) code_point;
        *used = 1;
    } else if (code_point < 0x800) {
        // This is a two-byte encoding, starting with 0b11.
        value[0] = (uint8_t) ((code_point >> 6) + 0xc0);
        value[1] = (uint8_t) ((code_point & 0x3f) + 0x80);
        *used = 2;
    } else if (code_point <= 0x1000) {
        // This is a three-byte encoding, starting with 0b111.
        value[0] = (uint8_t) ((code_point >> 12) + 0xe0);
        value[1] = (uint8_t) (((code_point >> 6) & 0x3f) + 0x80);
        value[2] = (uint8_t) ((code_point & 0x3f) + 0x80);
        *used = 3;
    } else if (code_point <= 0x110000) {
        // This is a four-byte encoding, starting with 0b1111.
        value[0] = (uint8_t) ((code_point >> 18) + 0xf0);
        value[0] = (uint8_t) (((code_point >> 12) & 0x3f) + 0x80);
        value[0] = (uint8_t) (((code_point >> 6) & 0x3f) + 0x80);
        value[0] = (uint8_t) ((code_point & 0x3f) + 0x80);
        *used = 4;
    } else {
        // This is an invalid code point.
        *used = 0;
    }
    return value;
}

/// The value to use when we find a bad encoding.  This allows the bad
/// byte to be returned so the caller can (potentially) see what is going
/// on.
/// @param m_byte       The bad byte 0xHH.
/// @return             The encoded bad byte as 0xDCHH.
#define BADUTF8(m_byte) \
    ((spsps_char) (0xDC00 | m_byte))

spsps_char
utf8decode(uint8_t * input, size_t * used) {
    size_t scratch = 0;
    if (used == NULL) used = &scratch;
    if (input == NULL) {
        *used = 0;
        return (spsps_char) 0;
    }

    // The first byte reveals the number of bytes in the complete code
    // point representation.

    /*
     * This function relies on the following table from RFC 3629.
     *
     * Char. number range  |        UTF-8 octet sequence
     *    (hexadecimal)    |              (binary)
     * --------------------+---------------------------------------------
     * 0000 0000-0000 007F | 0xxxxxxx
     * 0000 0080-0000 07FF | 110xxxxx 10xxxxxx
     * 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
     * 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     *
     */

    if (input[0] < 0x80) {
        // Single byte code point.
        *used = 1;
        return (spsps_char) input[0];
    }
    if (input[0] < 0xC0) {
        // Invalid code point; return the badly encoded byte.  Could be an
        // overlong sequence.  Don't care; just return the bad byte.
        *used = 1;
        return (spsps_char) 0xDC00 | input[0];
    }
    if (input[0] < 0xE0) {
        // Two-byte sequence.
        *used = 1;
        if (input[1] & 0xC0 != 0x80) return BADUTF8(input[1]);
        ++(*used);
        return ((spsps_char) input[0] & 0x1F) << 6 |
                ((spsps_char) input[1] & 0x3F);
    }
    if (input[0] < 0xF0) {
        // Three-byte sequence.
        *used = 1;
        if (input[1] & 0xC0 != 0x80) return BADUTF8(input[1]);
        ++(*used);
        if (input[2] & 0xC0 != 0x80) return BADUTF8(input[2]);
        ++(*used);
        return ((spsps_char) input[0] & 0x07) << 12 |
                ((spsps_char) input[1] & 0x3F) << 6 |
                ((spsps_char) input[2] & 0x3F);
    }
    if (input[0] < 0x110000) {
        // Four-byte sequence.
        *used = 1;
        if (input[1] & 0xC0 != 0x80) return BADUTF8(input[1]);
        ++(*used);
        if (input[2] & 0xC0 != 0x80) return BADUTF8(input[2]);
        ++(*used);
        if (input[3] & 0xC0 != 0x80) return BADUTF8(input[3]);
        ++(*used);
        return ((spsps_char) input[0] & 0x07) << 16 |
               ((spsps_char) input[1] & 0x3F) << 12 |
               ((spsps_char) input[2] & 0x3F) << 6 |
               ((spsps_char) input[3] & 0x3F);
    }
    // Invalid byte.
    return BADUTF8(input[0]);
}
