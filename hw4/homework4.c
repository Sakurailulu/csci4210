/* homework4.c
 * Griffin Melnick, melnig@rpi.edu
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main( int argc, char * argv[] ) {
    if ( argc == 3 ) {
        /* ... */
    } else {
        /* argc != 3 :: too few/many arguments */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <tcp-port> <udp-port>\n" );
        return EXIT_FAILURE;
    }
}
