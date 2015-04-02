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
