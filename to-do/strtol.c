/* strtol.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char str[2] = "a\0";

    char * tmpPtr;
    int tmp = strtol( str, &tmpPtr, 10 );
    printf( "%d", tmp );
    
    return EXIT_SUCCESS;
}
