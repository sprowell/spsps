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
