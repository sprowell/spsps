// Use wide characters.
#include <stdlib.h>
#define MSTRING_DEBUG 1
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
    mstr_inspect(str);
    val = mstr_wcstr_f(str);
    wprintf(L"%ls\n", val);
    free(val);
}
