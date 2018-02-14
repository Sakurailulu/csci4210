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
 *
 * Compilable with the following -D flags:
 *   -- DEBUG_MODE, provides debug output.
 *   -- DISPLAY_BOARD, provides output of current board.
 *   -- NO_PARALLEL, attempts to prevent interleaving by forcing waits.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int _x;
    int _y;
    char ** _grid;
} Board;


/* ------------------------------------------------------------------------- */

/* Board printing method ( specifically used with DISPLAY_BOARD ).
 * @param       b, Board struct to print.
 */
void printBoard( pid_t pid, Board * b ) {
    for ( int i = 0; i < (*b)._y; ++i ) {
        printf( "PID %d:   %s\n", pid, (*b)._grid[i] );
    }
}


/* Touring simulation.
 * @param b, Board struct pointer.
 */
int tour( Board * b ) {
    Board tmp = *b;
#ifdef DISPLAY_BOARD
    printBoard( getpid(), &tmp );
#endif

    b = &tmp;
    return EXIT_SUCCESS;
}


/* ------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    setbuf( stdout, NULL );         /* Prevent buffering in stdout. */
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

            /* Initializing Board struct to be used throughout. */
            Board touring;
            touring._x = m;         touring._y = n;
            touring._grid = calloc( touring._y, sizeof( char* ) );
            if ( touring._grid == NULL ) {
                fprintf( stderr, "ERROR: calloc() failed." );
            }
            for ( int i = 0; i < touring._y; ++i ) {
                touring._grid[i] = calloc( touring._x, sizeof( char ) );
            }

            /* Touring simulation. */
            pid_t pid = getpid();
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board\n", pid, m, n );

            int failed = tour( &touring );
            if ( !failed ) {
                return EXIT_SUCCESS;
            } else {
                fprintf( stderr, "ERROR: Tour failed.\n" );
                return EXIT_FAILURE;
            }

            /* Freeing allocated memory in board. */
            for ( int i = 0; i < touring._y; ++i ) {
                free( touring._grid[i] );
            }
            free( touring._grid );

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
