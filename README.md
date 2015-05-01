# SPSPS (Stacy's Pathetically Simple Parsing System)
This is a little project to create three things:

  - A nice string implementation for both mutable (time-efficient) and immutable (space-efficient) strings.
  - A simple framework for creating recursive-descent parsers.
  - A parser and data structures for JSON.

These are all being written in straight ANSI C (not C++).  Why C?  Well, many embedded platforms still rely on C, and also, why not?  See an answer to the question of why someone might want to use C [here][why-c].

## Building
This requires [CMake][cmake] to build.  If you want to build the documentation then you will also need [Doxygen][doxygen].

To build, run `cmake` in the root directory, and then you can build everything (except the documentation) with `make` and you can build the documentation with `make doc`.

On a Mac and under Linux:

	cmake -G 'Unix Makefiles'
	make
	make doc

On Windows you should do whatever is the Windows equivalent.  I think it is something like:

	cmake .
	make
	make doc

Want an out-of-source build?  Something else?  Read the documentation for CMake and have fun!

## xstring and mstring
To use the string library, just `#include "xstring.h"` and link with the `spsps` library.  The header file defines two new data types: `xstring` and `mstring`.

  - `xstring` holds immutable strings.  It is space-efficient, but if you need to build up a string by appending characters, this will be slow.  Every time you modify an instance a new instance is allocated.  Character addressing is fast.

  - `mstring` holds mutable strings.  These are strings that are efficient to append and concatenate, but not necessarily space efficient, because they are allocated in chunks.  The size of the chunk is controlled by the `MSTR_INC` macro.  Character addressing is slower than for`xstring` instances.

**The empty string is `NULL`.**

`NULL` is perfectly valid `mstring` and `xstring` value (the empty string).  In fact, the library prefers to avoid allocation until it is necessary.  All library functions accept `NULL` as a string value.

~~~~~~~~~~~~~~~{.c}
	#include "xstring.h"
	int main(int argc, char * argv[]) {
		xstring str = NULL;
		printf("The length of the empty string is %lu.\n", xstr_length(str));
		str = xstr_concat(NULL, NULL);
		if (str == NULL) {
			printf("The empty string is NULL.\n");
		}
		// Does nothing in this case, but a good defensive tactic.
		xstr_free(str);
	}
~~~~~~~~~~~~~~~

**You can use other character types.**

The characters of the strings can be of type `char`, or they can be of some other type, such as `wchar_t`, or even `uint32_t`, if you like.  They are converted to and from `char` values if necessary.  To use something other than `char`, `#define` the macro `SPSPS_CHAR` to the type you want before including the header file.  For example:

~~~~~~~~~~~~~~~{.c}
	#include <wchar.h>
	#include <stdlib.h>
	#define SPSPS_CHAR wchar_t
	#include "xstring.h"
	int main(int argc, char * argv[]) {
		xstring first = xstr_wwrap(L"¡Buenos días, mundo!");
		wchar_t * str = xstr_wcstr_f(first);
		wprintf(L"%ls\n", str);
		free(str);
	}
~~~~~~~~~~~~~~~

**Functions are prefixed with the kind of string they take.**

The functions for working with instances of `xstring` start with `xstr_`, while the functions for working with instances of `mstring` start with `mstr_`.

~~~~~~~~~~~~~~~{.c}
	#include <wchar.h>
	#include <stdlib.h>
	#define SPSPS_CHAR wchar_t
	#include "xstring.h"
	int main(int argc, char * argv[]) {
		mstring first = mstr_wwrap(L"¡Buenos días, mundo!");
		wchar_t * str = mstr_wcstr_f(first);
		wprintf(L"%ls\n", str);
		free(str);
	}
~~~~~~~~~~~~~~~

**Strings are non-trivial.  You can't just call `free` on them.**

**Always** deallocate instances of `xstring` and `mstring` with the `xstr_free` and `mstr_free` functions.  (These accept `NULL`.)

**Any value, including a null (0), is fine in a string.**

Strings store an array and the length, so you could have a string that contains nulls, if you wish.  Of course, conversion to a C string will not necessarily give the results you want in this case.

**Some methods free their arguments for you.**

Some methods end with `_f`.  Those free their arguments automatically, so pay attention.  This is especially useful for `xstring` instances, since when you concatenate or append to them you get a completely *new* instance, and if you forget to deallocate the old instance you have a memory leak.  So... you can use the `_f` form to solve this problem.

The following short program should compile and run with no memory leaks.  Only the final null-terminated string has to be explicitly deallocated.

~~~~~~~~~~~~~~~{.c}
	#include <stdlib.h>
	#include "xstring.h"
	int main(int argc, char * argv[]) {
		char * val;
		xstring first = xstr_wrap("Hello ");
		xstring second = xstr_wrap("xstring ");
		xstring third = xstr_wrap("world!");
		first = xstr_concat_f(first, second);
		first = xstr_concat_f(first, third);
		val = xstr_cstr_f(first);
		puts(val);
		free(val);
	}
~~~~~~~~~~~~~~~

**The library is really easy to use.**

The really, really, really quick guide in the form of just one more example.  This uses `mstring` instances, since they allocate in chunks.

~~~~~~~~~~~~~~~{.c}
	// Use wide characters.
	#include <stdlib.h>
	#define SPSPS_CHAR wchar_t
	#include "xstring.h"
	int main(int argc, char * argv[]) {
		// Use mstrings to concatenate all the arguments.
		wchar_t * val;
		mstring str = NULL;
		for (int index = 0; index < argc; ++index) {
			str = mstr_append_cstr(str, argv[index]);
			str = mstr_append(str, L' ');
		} // Add all the strings.
		// Print the result.
		val = mstr_wcstr_f(str);
		wprintf(L"%ls\n", val);
		free(val);
	}
~~~~~~~~~~~~~~~

## SPSPS
The main purpose of this library is to provide a simple framework for writing recursive descent parsers in C.  To find out more about such parsers, see [Wikipedia][rd].

In this section we will present a short example that parses the [TOML][toml] file format.

Parsers are constructed from a set of parsing primitives that provide the ability to "look ahead" at the input stream, and to "consume" characters from the input stream.  More specifically, SPSPS provides the following parsing primitives.

### Lookahead

  * `spsps_peek(parser)`
    Look ahead at the next character that will be read, and return it.  Nothing is consumed by this method.
  * `spsps_peek_n(parser, n)`
  	Look ahead at the next `n` characters that will be read, and return them as a fixed-length string.
  * `spsps_peek_str(parser, str)`
    Look ahead at the next characters that will be read, and determine if they exactly match the provided string.

### Consumption

  * `spsps_consume(parser)`
    Read (consume) the next character from the stream and return it.
  * `spsps_consume(parser, n)`
    Read (consume) the next `n` characters from the stream (consume them) and discard them.

### Combinations

As it happens you typically want to do more sophisticated things, so the following combination functions are provided.

  * `spsps_consume_whitespace(parser)`
    Consume all whitespace (spaces, tabs, carriage returns, and newlines) at the current position in the stream.  The stream points to the first non-whitespace character.
  * `spsps_peek_and_consume(parser, next)`
    Peek ahead at the input stream.  If the next few characters to be read match the string `next`, then consume them and return `true`.  Othewise simply return `false`.

### Infrastructure

You also have to create and free parser instances.  There are functions to do that, as well as to test for end of input stream.

  * `spsps_new(name, stream)`
    Construct and return a new `Parser` instance with the given `name`, wrapping the given input `stream`.  The `name` is typically an input file name and is used by `Loc`.
  * `spsps_free(parser)`
    Deallocate the parser instance.  This does not close the underlying stream; the caller is responsible for that.
  * `spsps_eof(parser)`
    Return `true` iff the input stream is at the end of stream, and `false` otherwise.
  * `spsps_loc(parser)`
    Return a `Loc` instance that tells the current location within the input stream (from where parsing started).  This contains the parser name, the line number (measured by newlines), and the column number.  Both line and column are zero-based.

### A Simple Parser

To illustrate how all this works, we are going to build a simple parser to parse floating point values.  These will have the following form.

<pre>
  number ::= DIGIT+ ( "." DIGIT+ )? ( ( "e" | "E" ) ( "+" | "-" )? DIGIT+ )?
</pre>

We'll modify this a bit to make the grammar more obviously [left-linear][linear].

<pre>
  number ::= "-"? digits frac? exp?
  digits ::= DIGIT+
  frac   ::= "." digits
  exp    ::= ("e" | "E") ("+" | "-")? digits
</pre>

We will next implement each piece as a function.  First, let's write a function that parses `digits`.  We will return the parsed digits as an integer, but we will also need to know the number of digits parsed, so we'll return that, too, through a pointer.  We can use `isdigit` (a C library function) to decide if something is a digit.

~~~~~~~~~~~~~~~{.c}
int
parse_digits(Parser parser, int* count) {
    *count = 0;
    if (! isdigit(spsps_peek(parser))) {
        SPSPS_ERR(parser, "Expected to find a digit, but instead found U+%04x.",
                  spsps_peek(parser));
        return 0;
    }
    int value = 0;
    do {
        value *= 10;
        value += spsps_consume(parser) - '0';
        *count += 1;
    } while (isdigit(spsps_peek(parser)));
    return value;
}
~~~~~~~~~~~~~~~

Let's walk through this line by line.  The first line of the function sets the number of digits parsed (`count`) to zero.  Next we check to see if the next character on the input stream is a digit.  We do this with the `isdigit(spsps_peek(parser))` test.  The `spsps_peek` tells us the next character on the input without consuming (reading) it.  If we don't find a digit, then we generate an error, using the `SPSPS_ERR` macro.

The `SPSPS_ERR` macro creates an error message that contains the current line and column number, along with the message provided by the user.  This message is formatted just as with `printf`, and is written to the `SPSPS_STDERR`, which  points to `stderr` unless you override it.

Next we set `value` to zero and start a loop to accumulate all digits.  For each digit, we multiply the prior value by 10 and add the decimal value of the digit.  Note that we check for a digit with `isdigit(spsps_peek(parser))`, just as before, and consume (read) the digit with `spsps_consume(parser)`.  We count the digits read by incrementing `count`.

So if a sequence of digits is successfully read, `count` will contain the number of digits read and the return value will be the decimal value of the digits.  If there is an error, then `count` will be zero (since no digits were read).

We can call this as follows.

~~~~~~~~~~~~~~~{.c}
    int digits = 0;
    double value = (double) parse_digits(parser, &digits);
    if (digits == 0) {
        return NAN;
    }
~~~~~~~~~~~~~~~

We declare the `digit` variable, and then parse the digits.  Since ultimately we will want a `double` value, we cast the return to a `double` here, and save it as `value`.  Note that if we did not parse any digits, we return NaN via the `NAN` macro.  The `parse_digits` function will have generated an error message for us.

Next, lets implement `frac`.  Instead of implementing a function for this, we'll just parse the fractional part if we see a period.  The code might look like this.

~~~~~~~~~~~~~~~{.c}
    if (spsps_peek_and_consume(parser, ".")) {
        // We have found a fractional part.  Parse it.
        double fracpart = (double) parse_digits(parser, &digits);
        if (digits == 0) {
            return NAN;
        }
        value += fracpart / pow(10, digits);
    }
~~~~~~~~~~~~~~~

Here we look for a period.  If we find one we consume it and then parse all the subsequent digits.  The fractional part is then added to `value`.  We do this by dividing it by ten to the power of the number of digits, using the C library's `pow` function.

Next we can look for an exponent.  Again there is no need to implement a function for this.  We can just look for an `e` or `E`.  Parsers are case-sensitive, so we have to check for both.

~~~~~~~~~~~~~~~{.c}
    if (spsps_peek_and_consume(parser, "E")
        	|| spsps_peek_and_consume(parser, "e")) {
        // We have found an exponent part.  Parse it.
        bool negexp = false;
        if (spsps_peek_and_consume(parser, "-")) {
            negexp = true;
        } else {
            spsps_peek_and_consume(parser, "+");
        }
        double exppart = (double) parse_digits(parser, &digits);
        if (digits == 0) {
            return NAN;
        }
        if (negexp) exppart = -exppart;
        value *= pow(10, exppart);
    }
~~~~~~~~~~~~~~~

We use `spsps_peek_and_consume` several times here.  This is a very cheap method to call - it just checks a memory buffer.  We have to check for both `e` and `E`.  We might do this as follows.

~~~~~~~~~~~~~~~{.c}
	// Alternate method.
	SPSPS_CHAR ch = spsps_peek(parser);
	if (ch == 'E' || ch == 'e') {
		spsps_consume(parser);
		// ... everything else
	}
~~~~~~~~~~~~~~~

Here we use `spsps_peek` to see the next character, and then check it against the two possible values.  We have to then call `spsps_consume` to actually read the character, since `spsps_peek` does not consume anything.

Next we check for a minus sign or a plus sign, and consume them if we find them.  Without a sign we assume the value is positive.

Next we parse the digits after the exponent.  If we don't have any we return NaN via the `NAN` macro.  We negate the exponent if necessary, and then we multiply the value by the exponent.

At this point we have the value, and can simply return it.  We can pull all this together into a method that parses and returns a `double`.  It will return `NaN` if the number cannot be parsed.  We also need to deal with the possibility of a negative number.  We just check for a leading minus sign, and set a flag accordingly.  At return we negate the number, if needed.

~~~~~~~~~~~~~~~{.c}
double
parse_double(Parser parser) {
    bool neg = spsps_peek_and_consume(parser, "-");
    int digits = 0;
    double value = (double) parse_digits(parser, &digits);
    if (digits == 0) {
        return NAN;
    }
    if (spsps_peek_and_consume(parser, ".")) {
        // We have found a fractional part.  Parse it.
        double fracpart = (double) parse_digits(parser, &digits);
        if (digits == 0) {
            return NAN;
        }
        value += fracpart / pow(10, digits);
    }
    if (spsps_peek_and_consume(parser, "E")
            || spsps_peek_and_consume(parser, "e")) {
        // We have found an exponent part.  Parse it.
        bool negexp = false;
        if (spsps_peek_and_consume(parser, "-")) {
            negexp = true;
        } else {
            spsps_peek_and_consume(parser, "+");
        }
        double exppart = (double) parse_digits(parser, &digits);
        if (digits == 0) {
            return NAN;
        }
        if (negexp) exppart = -exppart;
        value *= pow(10, exppart);
    }
    return (neg ? -value : value);
}
~~~~~~~~~~~~~~~

This function requires that we have a `parser` instance, so let's do that next.  We will wrap the standard input stream and parse doubles the user types.

~~~~~~~~~~~~~~~{.c}
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <parser.h>

int
main(int argc, char * argv[]) {
    Parser parser = spsps_new("(console)", stdin);
    spsps_consume_whitespace(parser);
    double value = parse_double(parser);
    printf("Parsed: %lf\n", value);
    spsps_free(parser);
}
~~~~~~~~~~~~~~~


## JSON

[cmake]: http://www.cmake.org
[why-c]: http://stackoverflow.com/questions/497786/why-would-anybody-use-c-over-c
[doxygen]: http://www.stack.nl/~dimitri/doxygen/
[rd]: http://en.wikipedia.org/wiki/Recursive_descent_parser
[toml]: https://github.com/toml-lang/toml
[linear]: http://en.wikipedia.org/wiki/Linear_grammar

