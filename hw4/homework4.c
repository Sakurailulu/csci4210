/* homework4.c
 * Griffin Melnick, melnig@rpi.edu
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Macro defintions for length of user ids and messages. */
#define MIN_ID 3
#define MAX_ID 20
#define MAX_MSG 994

/* Method declarations. ----------------------------------------------------- */



/* Method definitions. ------------------------------------------------------ */



/* Main. -------------------------------------------------------------------- */

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
