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
#include <string.h>
#include <unistd.h>

typedef struct {
    int _x;
    int _y;
    char ** _grid;
} Board;


/* ------------------------------------------------------------------------- */

/* Board initialization method.
 * @param       x, width of board grid.
 *              y, height of board grid.
 * @return      new Board struct if no allocation fails, NULL otherwise.
 */
Board * initBoard( int x, int y ) {
    assert( ( x > 2 ) && ( y > 2 ) );

    Board b;
    b._x = x;
    b._y = y;
    b._grid = calloc( b._y, sizeof( char* ) );

    if ( b._grid == NULL ) {
        perror( "ERROR" );
        return b;
    } else {
        for ( int i = 0; i < b._y; ++i ) {
            b._grid[i] = calloc( b._x, sizeof( char ) );
            if ( b._grid[i] == NULL ) {
                perror( "ERROR" );
                return b;
            }
        }
    }

    for ( int i = 0; i < b._y; ++i ) {
        for ( int j = 0; j < b._x; ++j ) {
            b._grid[i][j] = '.';
        }
    }
    b._grid[0][0] = 'k';

    return &b;
}


/* Board printing method ( specifically used with DISPLAY_BOARD ).
 * @param       b, Board struct to print.
 */
void printBoard( pid_t pid, Board * b ) {
    for ( int i = 0; i < (*b)._y; ++i ) {
        printf( "PID %d:   %s\n", pid, (*b)._grid[i] );
    }
}

/* Board freeing method ( calls free() on _grid )
 * @param       b, Board struct to free.
 * @modifies    b
 * @effects     frees b._grid
 */
void freeBoard( Board * b ) {
    // Board tmp = *b;
    for ( int i = 0; i < (*b)._y; ++i ) {
        free( (*b)._grid[i] );
    }
    free( (*b)._grid );
    // b = &tmp;
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

            /* Board * touring = initBoard( n, m );
            pid_t pid = getpid();
            printf( "PID %d: Solving the knight's tour problem for a %dx%d
                        board\n", pid, m, n );

            printBoard( pid, touring ); */

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
