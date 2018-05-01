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
        /* Check that both port numbers are valid. */
        char * tmp;
        const int tcp = strtol( argv[1], &tmp, 10 ),
                  udp = strtol( argv[2], &tmp, 10 );
        if ( ( (tcp >= 1024) && (tcp <= 65535) ) &&
                ( (udp >= 1024) && (udp <= 65535) ) ) {
            /* ... */
        } else {
            /* ( (tcp < 1024) || (tcp > 65535) ) ||
             * ( (udp < 1024) || (udp > 65535) ) :: invalid port number */
            fprintf( stderr, "ERROR: Invalid port number(s)\n" );
        }
    } else {
        /* argc != 3 :: too few/many arguments */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <tcp-port> <udp-port>\n" );
    }
    return EXIT_FAILURE;
}
