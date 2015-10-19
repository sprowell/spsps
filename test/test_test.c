/**
 * @file
 * Simple test framework.
 *
 * To use this, include this header file.  You can define a test using
 * START_TEST and END_TEST.  Within a test you can define individual
 * items with the START_ITEM(name_m) and END_ITEM(name_m) macros, where
 * the item name you use must match, and is significant - it is used to
 * generate messages.
 *
 * Use FAIL_ITEM(name_m) to fail an item, and the item name must match
 * there, too.  If you get really odd errors, check that names match
 * in your macros.  You can't reuse names within a test, or bad things
 * will happen!
 *
 * @author sprowell@gmail.com
 *
 * @verbatim
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

#include "test_frame.h"

START_TEST;

START_ITEM(first);
WRITE("Trying... ");
WRITE("1");
WRITE("2");
WRITE("3");
WRITELN(" Done!");
VALIDATE("172", "172");
END_ITEM;

START_ITEM(second);
WRITELN("Doing stuff.");
END_ITEM;

START_ITEM(third);
WRITELN("Don't run this item.");
FAIL_ITEM("Always fails.");
END_ITEM;

// Uncomment this line to disable the next item.
// DISABLE;
START_ITEM(fourth);
int x = 181, y = 182;
if (x != y) FAIL("%d != %d", x, y);
END_ITEM;

END_TEST;
