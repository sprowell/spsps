#ifndef SPSPS_UTF8_H_
#define SPSPS_UTF8_H_

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

#include <_types/_uint32_t.h>
#include <_types/_uint8_t.h>
#include <stddef.h>

/// The character type for a single character.
typedef uint32_t spsps_char;

/**
 * Given a single character, encode it into the sequence of UTF-8 bytes.
 * This is based on RFC 3629. (http://tools.ietf.org/html/rfc3629)
 * @param code_point    The input character.
 * @param used          The number of bytes for the encoded character.
 *                      This is zero iff the input code point is invalid.
 *                      This is ignored if it is NULL on input.
 * @return              The null-terminated encoded byte sequence.  This is
 *                      never NULL, and the caller is responsible for
 *                      deallocating it.
 */
uint8_t * utf8encode(spsps_char input, size_t * used);

/**
 * Given a sequence of bytes that may or may not be null-terminated,
 * try to convert that sequence of bytes into a single Unicode character.
 * The provided byte sequence is assumed to be at least six bytes in
 * length, so make sure that it is allocated in that manner.
 * This is based on RFC 3629. (http://tools.ietf.org/html/rfc3629)
 * @param input     The input bytes.  If NULL on input, nul (0) is returned.
 * @param used      Number of bytes consumed by this action.  This is
 *                  ignored if it is NULL on input.
 * @return          The decoded character.  This will be nul (0) if the
 *                  input byte sequence is not a valid UTF-8 character.
 */
spsps_char utf8decode(uint8_t * input, size_t * used);

#endif //SPSPS_UTF8_H_
