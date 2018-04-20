/* segfault.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    void * ptr = NULL;
    printf( "%d\n", ptr[2] );
    return EXIT_SUCCESS;
}

