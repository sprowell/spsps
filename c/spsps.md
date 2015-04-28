# SPSPS (Stacy's Pathetically Simple Parsing System)
This is a little project to create three things:

  - A nice string implementation for both mutable (time-efficient) and immutable (space-efficient) strings.
  - A simple framework for creating recursive-descent parsers.
  - A parser and data structures for JSON.

These are all being written in straight ANSI C (not C++).  Why C?  Well, many embedded platforms still rely on C, and also, why not?  See an answer to the question of why someone might want to use C [here][why-c].

## Building
This requires [CMake][cmake] to build.  If you want to build the documentation then you will also need [Doxygen][doxygen].

To build, run `cmake` in the root directory, and then you can build everything (except the documentation) with `make` and you can build the documentation with `make doc`.

On my Mac and under Linux:

```
cmake -G 'Unix Makefiles'
make
make doc
```

On Windows you should do whatever is the Windows equivalent.  I think it is something like:

```
cmake .
make
make doc
```

Want an out-of-source build?  Something else?  Read the documentation for CMake and have fun!

## xstring and mstring
To use the string library, just `#include "xstring.h"` and link with the `spsps` library.  The header file defines two new data types: `xstring` and `mstring`.

  - `xstring` holds immutable strings.  It is space-efficient, but if you need to build up a string by appending characters, this will be slow.  Every time you modify an instance a new instance is allocated.  Character addressing is fast.

  - `mstring` holds mutable strings.  These are strings that are efficient to append and concatenate, but not necessarily space efficient, because they are allocated in chunks.  The size of the chunk is controlled by the `MSTR_INC` macro.  Character addressing is slower than for`xstring` instances.

**The empty string is `NULL`.**

`NULL` is perfectly valid `mstring` and `xstring` value (the empty string).  In fact, the library prefers to avoid allocation until it is necessary.  All library functions accept `NULL` as a string value.

```c
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
```

**You can use other character types.**

The characters of the strings can be of type `char`, or they can be of some other type, such as `wchar_t`, or even `uint32_t`, if you like.  They are converted to and from `char` values if necessary.  To use something other than `char`, `#define` the macro `SPSPS_CHAR` to the type you want before including the header file.  For example:

```c
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
```

**Functions are prefixed with the kind of string they take.**

The functions for working with instances of `xstring` start with `xstr_`, while the functions for working with instances of `mstring` start with `mstr_`.

```c
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
```

**Strings are non-trivial.  You can't just call `free` on them.**

**Always** deallocate instances of `xstring` and `mstring` with the `xstr_free` and `mstr_free` functions.  (These accept `NULL`.)

**Any value, including a null (0), is fine in a string.**

Strings store an array and the length, so you could have a string that contains nulls, if you wish.  Of course, conversion to a C string will not necessarily give the results you want in this case.

**Some methods free their arguments for you.**

Some methods end with `_f`.  Those free their arguments automatically, so pay attention.  This is especially useful for `xstring` instances, since when you concatenate or append to them you get a completely *new* instance, and if you forget to deallocate the old instance you have a memory leak.  So... you can use the `_f` form to solve this problem.

The following short program should compile and run with no memory leaks.  Only the final null-terminated string has to be explicitly deallocated.

```c
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
```

**The library is really easy to use.**

The really, really, really quick guide in the form of just one more example.  This uses `mstring` instances, since they allocate in chunks.

```c
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
```

## SPSPS

## JSON

[cmake]: http://www.cmake.org
[why-c]: http://stackoverflow.com/questions/497786/why-would-anybody-use-c-over-c
[doxygen]: http://www.stack.nl/~dimitri/doxygen/
