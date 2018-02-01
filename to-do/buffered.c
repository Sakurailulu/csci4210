/* buffered.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    /* stdout = buffering, stderr = no buffering */

    printf( "123\n" );
    // fflush( stdout );

    fprintf( stderr, "I'm first!\n" );
    printf( "456\n" );


    return EXIT_SUCCESS;
}
