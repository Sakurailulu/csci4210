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
    int _cols;
    int _rows;
    int _moves;
    Pair _curr;
    char ** _grid;
} Board;

const Pair moves[8] = { (Pair){ ._x = +1, ._y = -2 },   /* up, then right */
                        (Pair){ ._x = +2, ._y = -1 },   /* right, then up */
                        (Pair){ ._x = +2, ._y = +1 },   /* right, then down */
                        (Pair){ ._x = +1, ._y = +2 },   /* down, then right */
                        (Pair){ ._x = -1, ._y = +2 },   /* down, then left */
                        (Pair){ ._x = -2, ._y = +1 },   /* left, then down */
                        (Pair){ ._x = -2, ._y = -1 },   /* left, then up */
                        (Pair){ ._x = -1, ._y = -2 } }; /* up, then left */


void printBoard( pid_t pid, Board b );
int findPossMoves( Board b, Pair * moveTo );
void tour( Board * b );


/* ------------------------------------------------------------------------- */

/* Board printing method ( specifically used with DISPLAY_BOARD ).
 * @param       b, Board struct to print.
 */
void printBoard( pid_t pid, Board b ) {
    for ( int i = 0; i < b._rows; ++i ) {
        printf( "PID %d:   %s\n", pid, b._grid[i] );
    }
}


/* Finds count of possible moves from current position.
 * @param       b, Board struct to search.
 * @return      count of possible moves found.
 */
int findPossMoves( Board b, Pair * moveTo ) {
    int numPoss = 0;
    //Pair moves[8] = { (Pair){ ._x = 1, ._y = -2 },      /* up, then right */
    //                  (Pair){ ._x = 2, ._y = -1 },      /* right, then up */
    //                  (Pair){ ._x = 2, ._y = 1 },       /* right, then down */
    //                  (Pair){ ._x = 1, ._y = 2 },       /* down, then right */
    //                  (Pair){ ._x = -1, ._y = 2 },      /* down, then left */
    //                  (Pair){ ._x = -2, ._y = 1 },      /* left, then down */
    //                  (Pair){ ._x = -2, ._y = -1 },     /* left, then up */
    //                  (Pair){ ._x = -1, ._y = -2 } };   /* up, then left */

    for ( int i = 0; i < 8; ++i ) {
        int tmpX = b._curr._x + moves[i]._x, tmpY = b._curr._y + moves[i]._y;
        if ( ( 0 <= tmpX <= b._cols ) && ( 0 <= tmpY <= b._rows ) &&
                    ( b._grid[tmpY][tmpX] != 'k' ) ) {
            ++numPoss;
            moveTo[i] = (Pair){ ._x = tmpX, ._y = tmpY };
        }
    }

    /*if ( ( b._curr._x + 2 ) <= b._cols ) {
        if ( ( b._curr._y + 1 ) <= b._rows ) {
#ifdef DEBUG_MODE
            printf( "        moving right, then down...\n" );
#endif

            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._x = ( b._curr._x + 2 ),
                                              ._y = ( b._curr._y + 1 ) };
        }

        if ( ( b._curr._y - 1 ) >= 0 ) {
#ifdef DEBUG_MODE
            printf( "        moving right, then up...\n" );
#endif
            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._x = ( b._curr._x + 2 ),
                                              ._y = ( b._curr._y - 1 ) };
        }
    }

    if ( ( b._curr._x - 2 ) >= 0 ) {
        if ( ( b._curr._y + 1 ) <= b._rows ) {
#ifdef DEBUG_MODE
            printf( "        moving left, then down...\n" );
#endif
            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._x = ( b._curr._x - 2 ),
                                              ._y = ( b._curr._y + 1 ) };
        }

        if ( ( b._curr._y - 1 ) >= 0 ) {
#ifdef DEBUG_MODE
            printf( "        moving left, then up...\n" );
#endif
            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._x = ( b._curr._x - 2 ),
                                              ._y = ( b._curr._y - 1 ) };
        }
    }

    if ( ( b._curr._y + 2 ) <= b._rows ) {
        if ( ( b._curr._x + 1 ) <= b._cols ) {
#ifdef DEBUG_MODE
            printf( "        moving down, then right...\n" );
#endif
            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._y = ( b._curr._y + 2 ),
                                              ._x = ( b._curr._x + 1 ) };
        }

        if ( ( b._curr._x - 1 ) >= 0 ) {
#ifdef DEBUG_MODE
            printf( "        moving down, then left...\n" );
#endif
            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._y = ( b._curr._y + 2 ),
                                              ._x = ( b._curr._x - 1 ) };
        }
    }

    if ( ( b._curr._y - 2 ) >= 0 ) {
        if ( ( b._curr._x + 1 ) <= b._cols ) {
#ifdef DEBUG_MODE
            printf( "        moving up, then right...\n" );
#endif
            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._y = ( b._curr._y - 2 ),
                                              ._x = ( b._curr._x + 1 ) };
        }

        if ( ( b._curr._x - 1 ) >= 0 ) {
#ifdef DEBUG_MODE
            printf( "        moving up, then left...\n" );
#endif
            ++numPoss;
            moveTo[( numPoss - 1 )] = (Pair){ ._y = ( b._curr._y - 2 ),
                                              ._x = ( b._curr._x - 1 ) };
        }
    } */

    return numPoss;
}


/* Touring simulation.
 * @param       b, Board struct pointer.
 * @return      EXIT_SUCCESS or EXIT_FAILURE depending on children processes.
 * @modifies    b
 * @effects     marks spots visited by changing from '.' to 'k'.
 */
void tour( Board * b ) {
#ifdef DEBUG_MODE
    printf( "    touring...\n" );
#endif

    Board tmp = *b;

    Pair * moveTo = calloc( 8, sizeof( Pair ) );
    int poss = findPossMoves( tmp, moveTo );
    // moveTo = realloc( moveTo, ( poss * sizeof( Pair ) ) );
#ifdef DEBUG_MODE
    printf( "            poss = %d\n", poss );
    for ( int i = 0; i < poss; ++i ) {
        printf( "                move %d: (%d, %d)\n", ( i + 1 ), moveTo[i]._x,
                    moveTo[i]._y );
    }
#endif

    if ( poss > 0 ) {
        if ( poss > 1 ) {
            printf( "PID %d: Multiple moves possible after move #%d\n",
                        getpid(), tmp._moves );
#ifdef DISPLAY_BOARD
            printBoard( getpid(), tmp );
#endif

            pid_t pids[poss];
            for ( int i = 0; i < poss; ++i ) {
                if ( ( pids[i] = fork() ) < 0 ) {
                    fprintf( stderr, "ERROR: fork() failed." );
                } else if ( pids[i] == 0 ) {
                    /* CHILD */
                }
            }

            pid_t pid;
            while ( poss > 0 ) {
                pid = wait( NULL );
                printf( "Child exited." );
                --poss;
            }
        } else {

        }
    } else {
        printf( "PID %d: Dead end after move #%d", getpid(), tmp._moves );
    }

    free( moveTo );
    b = &tmp;
    // return countMoves;
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
            touring._cols = m;                  touring._rows = n;
            touring._grid = calloc( touring._rows, sizeof( char* ) );
            if ( touring._grid == NULL ) {
                fprintf( stderr, "ERROR: calloc() failed." );
                return EXIT_FAILURE;
            } else {
                for ( int i = 0; i < touring._rows; ++i ) {
                    touring._grid[i] = calloc( touring._cols, sizeof( char ) );
                    if ( touring._grid[i] == NULL ) {
                        fprintf( stderr, "ERROR: calloc() failed." );
                        return EXIT_FAILURE;
                    }
                }
            }

            /** Fill grid with marker values. */
            for ( int i = 0; i < touring._rows; ++i ) {
                for ( int j = 0; j < touring._cols; ++j ) {
                    touring._grid[i][j] = '.';
                }
            }
            touring._grid[0][0] = 'k';          touring._moves = 1;
            touring._curr = (Pair){ ._x = 0, ._y = 0 };

#ifdef DEBUG_MODE
            printf( "        Board details:\n" );
            printf( "        _x = %d, _y = %d, _moves = %d\n", touring._cols,
                        touring._rows, touring._moves );
            printf( "        _curr = (%d, %d)\n", touring._curr._x,
                        touring._curr._y );
#endif

            /* Touring simulation. */
            pid_t pid = getpid();
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board.\n",
                        pid, touring._cols, touring._rows );

            tour( &touring );

            /* Freeing allocated memory in board. */
#ifdef DEBUG_MODE
            printf( "    freeing memory...\n" );
#endif

            for ( int i = 0; i < touring._rows; ++i ) {
                free( touring._grid[i] );       touring._grid[i] = NULL;
            }
            free( touring._grid );              touring._grid = NULL;
            return EXIT_SUCCESS;
        } else {
            /* ( m <= 2 ) && ( n <= 2 ) */
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
