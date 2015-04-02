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

