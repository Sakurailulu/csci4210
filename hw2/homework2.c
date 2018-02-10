/* homework2.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Emulating a solution finding algorithm to the knight's tour problem, we 
 * take a grid of m x n squares and iterate through children processes to 
 * determine the maximum number of moves possible. This is called by the 
 * command:
 *
 *   bash$ ./a.out <m> <n>
 *
 * Where *m* and *n* are the respective grid lengths.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    
} Tour;

typedef struct {
    int _x;
    int _y;
    char ** _grid;
} Board;


/* ------------------------------------------------------------------------- */

int initBoard( Board * b, int x, int y ) {
    assert( ( x > 2 ) && ( y > 2 ) );
    
    Board loc = *b;
    loc._grid = calloc( x, sizeof( char* ) );
    if ( loc._grid == NULL ) {
        perror( "ERROR" );
        return EXIT_FAILURE;
    } else {
        for ( int i = 0; i < x; ++i ) {
            loc._grid[i] = calloc( y, sizeof( char ) );
            if ( loc._grid[i] == NULL ) {
                perror( "ERROR" );
                return EXIT_FAILURE;
            }
        }
    }

    b = &loc;
    return EXIT_SUCCESS;
}

/* ------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    if ( argc == 3 ) {
#ifdef DEBUG_MODE
        printf( "started...\n" );
#endif

        char * tmp;
        int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 );
        if ( ( m > 2 ) && ( n > 2 ) ) {
#ifdef DEBUG_MODE
            printf( "\tvalid args... continuing...\n" );
#endif

            /* hopefully */
            return EXIT_SUCCESS;
        } else {
            /* ( m <= 2 ) && ( n <= 3 ) */
            fprintf( stderr, "ERROR: Invalid argument(s)\n" );
            fprintf( stderr, "USAGE: %s <m> <n>\n", argv[0] );
            return EXIT_FAILURE;
        }
    } else {
        /* argc != 3 */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: %s <m> <n>\n", argv[0] );
        return EXIT_FAILURE;
    }

    /* We should never be able to get to this point. */
    return EXIT_FAILURE;
}
