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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Helper structs. */
/** Provides representation of coordinate pair to help with positioning. */
typedef struct {
    int _x;
    int _y;
} Pair;


/** Provides representation of board on which to perform the knight's tour. */
typedef struct {
    int _rows;
    int _cols;
    int _moves;
    Pair _curr;
    char ** _grid;
} Board;


void printBoard( pid_t pid, Board b );
int findPossMoves( Board b );
int tour( Board * b );


/* ------------------------------------------------------------------------- */

/* Board printing method ( specifically used with DISPLAY_BOARD ).
 * @param       b, Board struct to print.
 */
void printBoard( pid_t pid, Board b ) {
    for ( int i = 0; i < b._cols; ++i ) {
        printf( "PID %d:   %s\n", pid, b._grid[i] );
    }
}


/* Finds count of possible moves from current position.
 * @param       b, Board struct to search.
 * @return      count of possible moves found.
 */
int findPossMoves( Board b ) {
    return 2;
}


/* Touring simulation.
 * @param       b, Board struct pointer.
 * @return      EXIT_SUCCESS or EXIT_FAILURE depending on children processes.
 * @modifies    b
 * @effects     marks spots visited by changing from '.' to 'k'.
 */
int tour( Board * b ) {
    Board tmp = *b;

    int poss = findPossMoves( tmp );
    if ( poss > 1 ) {
        printf( "PID %d: Multiple moves possible after move #%d\n",
                    getpid(), tmp._moves );

#ifdef DISPLAY_BOARD
    printBoard( getpid(), tmp );
#endif

        /* fork */
    } else {
        /* handle return / child exit */
    }

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
            printf( "    valid args, continuing...\n" );
#endif

            /* Initializing Board struct to be used throughout. */
            /** Memory allocation. */
#ifdef DEBUG_MODE
            printf( "    intializing board...\n" );
#endif

            Board touring;
            touring._rows = m;                 touring._cols = n;
            touring._grid = calloc( touring._cols, sizeof( char* ) );
            if ( touring._grid == NULL ) {
                fprintf( stderr, "ERROR: calloc() failed." );
                return EXIT_FAILURE;
            } else {
                for ( int i = 0; i < touring._cols; ++i ) {
                    touring._grid[i] = calloc( touring._rows, sizeof( char ) );
                    if ( touring._grid[i] == NULL ) {
                        fprintf( stderr, "ERROR: calloc() failed." );
                        return EXIT_FAILURE;
                    }
                }
            }

            /** Fill grid with marker values. */
            for ( int i = 0; i < touring._cols; ++i ) {
                for ( int j = 0; j < touring._rows; ++j ) {
                    touring._grid[i][j] = '.';
                }
            }
            touring._grid[0][0] = 'k';      touring._moves = 1;
            touring._curr = (Pair){ ._x = 0, ._y = 0 };

#ifdef DEBUG_MODE
            printf( "        Board details:\n" );
            printf( "        _x = %d, _y = %d, _moves = %d\n", touring._rows,
                        touring._cols, touring._moves );
            printf( "        _curr = (%d, %d)\n", touring._curr._x,
                        touring._curr._y );
#endif

            /* Touring simulation. */
#ifdef DEBUG_MODE
            printf( "    touring...\n" );
#endif

            pid_t pid = getpid();
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board\n",
                        pid, touring._rows, touring._cols );

            tour( &touring );

            /* Freeing allocated memory in board. */
#ifdef DEBUG_MODE
            printf( "    freeing memory...\n" );
#endif

            for ( int i = 0; i < touring._cols; ++i ) {
                free( touring._grid[i] );   touring._grid[i] = NULL;
            }
            free( touring._grid );          touring._grid = NULL;
            return EXIT_SUCCESS;

            /* Check for failure while touring. */
            /* int failed = tour( &touring ); 
            if ( !failed ) {
                return EXIT_SUCCESS;
            } else {
                fprintf( stderr, "ERROR: Tour failed.\n" );
                return EXIT_FAILURE;
            } */
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
