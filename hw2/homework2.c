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
    
} Tour;

typedef struct {
    int _x;
    int _y;
    char ** _grid;
} Board;


/* ------------------------------------------------------------------------- */

/* Board initialization method.
 * @param   x, width of board grid.
 *          y, height of board grid.
 * @return  new Board struct if no allocation fails, NULL otherwise.
 */
Board initBoard( int x, int y ) {
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

    return b;
}


/* Board printing method ( specifically used with DISPLAY_BOARD ).
 * @param   b, Board struct to print.
 */
void printBoard( Board b ) {
    char * tmp = calloc( b._x, sizeof( char ) );
    for ( int i = 0; i < b._y; ++i ) {
        printf( "%s\n", tmp[i] );
    }
    for ( int i = 0; i < b._x; ++i ) {
        for ( int j = 0; j < b._y; ++j ) {
            tmp[j] = b._grid[i][j];
        }
        printf( "%s\n", tmp );
        memset( tmp, 0, b._x );
    }
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

            Board touring = initBoard( m, n );
            printBoard( touring );
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
