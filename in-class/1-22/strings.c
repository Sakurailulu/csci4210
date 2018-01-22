/* strings.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    char name[80];              /* char == byte */
    strcpy( name, "Melnick" );
    name[4] = '\0';

    /*       +---------------------------------+
     * name: | Melnick\0                       |
     *       +---------------------------------+ */

    /* '\0' is the end-of-string (EOS) character */

    printf( "Hello, %s.\n", name );
    return EXIT_SUCCESS;
}

