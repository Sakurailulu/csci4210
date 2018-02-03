/* global.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int SIZEOF = (int)sizeof( int );

int main() {
    printf( "SIZEOF = %d; sizeof( int ) = %d\n", SIZEOF, (int)sizeof( int ) );
    
    return EXIT_SUCCESS;
}
