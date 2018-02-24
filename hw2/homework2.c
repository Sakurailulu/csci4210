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
#include <sys/wait.h>
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


/* Initial method declarations. */
void printBoard( pid_t pid, Board b );
int findPossMoves( Board b, Pair * moveTo );
int tour( Board * b );

/* Constant array with all possible movements. */
const Pair moves[8] = { (Pair){ ._x = +1, ._y = -2 },   /* up, then right */
                        (Pair){ ._x = +2, ._y = -1 },   /* right, then up */
                        (Pair){ ._x = +2, ._y = +1 },   /* right, then down */
                        (Pair){ ._x = +1, ._y = +2 },   /* down, then right */
                        (Pair){ ._x = -1, ._y = +2 },   /* down, then left */
                        (Pair){ ._x = -2, ._y = +1 },   /* left, then down */
                        (Pair){ ._x = -2, ._y = -1 },   /* left, then up */
                        (Pair){ ._x = -1, ._y = -2 } }; /* up, then left */


/* ------------------------------------------------------------------------- */

/* Board printing method ( specifically used with DISPLAY_BOARD ).
 * @param       b, Board struct to print.
 */
void printBoard( pid_t pid, Board b ) {
    for ( int i = 0; i < b._rows; ++i ) {
        printf( "PID %d:  %s\n", pid, b._grid[i] );
    }
}


/* Finds count of possible moves from current position.
 * @param       b, Board struct to search.
 *              moveTo, Pair struct pointer to store valid next moves.
 * @return      count of possible moves found.
 * @modifies    moveTo
 * @effects     adds Pairs with valid next moves.
 */
int findPossMoves( Board b, Pair * moveTo ) {
    int numPoss = 0;
    for ( int i = 0; i < 8; ++i ) {
        int tmpX = b._curr._x + moves[i]._x, tmpY = b._curr._y + moves[i]._y;
        if ( ( ( 0 <= tmpX ) && ( tmpX < b._cols ) ) &&
                ( ( 0 <= tmpY ) && ( tmpY < b._rows ) ) &&
                ( b._grid[tmpY][tmpX] != 'k' ) ) {
            moveTo[numPoss] = (Pair){ ._x = tmpX, ._y = tmpY };
            ++numPoss;
        }
    }

    return numPoss;
}


/* Touring simulation.
 * @param       b, Board struct pointer.
 * @return      EXIT_SUCCESS or EXIT_FAILURE depending on children processes.
 * @modifies    b
 * @effects     marks spots visited by changing from '.' to 'k'.
 */
int tour( Board * b ) {
#ifdef DEBUG_MODE
    printf( "    %d touring...\n", getpid() );
#endif

    Board tmp = *b;

    /* Count number of valid next moves. */
    Pair * moveTo = calloc( 8, sizeof( Pair ) );
    int poss = findPossMoves( tmp, moveTo );
    moveTo = realloc( moveTo, ( poss * sizeof( Pair ) ) );
#ifdef DEBUG_MODE
    printf( "        poss = %d\n", poss );
    for ( int i = 0; i < poss; ++i ) {
        printf( "            move %d: (%d, %d)\n", ( i + 1 ), moveTo[i]._x,
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
            int * numMoves = calloc( poss, sizeof( int ) );
            for ( int i = 0; i < poss; ++i ) {
                /* Initialize pipe. */
                int p[2], rc;
                if ( ( rc = pipe( p ) ) < 0 ) {
                    fprintf( stderr, "ERROR: pipe() failed.\n" );
                    return EXIT_FAILURE;
                }

                /* Fork to all children processes. */
                if ( ( pids[i] = fork() ) < 0 ) {
                    fprintf( stderr, "ERROR: fork() failed.\n" );
                    return EXIT_FAILURE;
                } else if ( pids[i] == 0 ) {    /* CHILD */
                    tmp._curr = moveTo[i];
                    tmp._grid[tmp._curr._y][tmp._curr._x] = 'k';
                    ++tmp._moves;

                    close( p[0] );              p[0] = -1;
                    int bytesWritten = write( p[1], &tmp._moves,
                            sizeof( int ) );
                    if ( bytesWritten == -1 ) {
                        fprintf( stderr, "ERROR: write() failed\n" );
                        return EXIT_FAILURE;
                    } else {
                        printf( "PID %d: Sending %d on pipe to parent pid %d\n",
                                getpid(), tmp._moves, getppid() );
                    }

                    tour( &tmp );

                    /* Freeing memory that will get lost in children. */
                    free( numMoves );
                    free( moveTo );
                    for ( int j = 0; j < (*b)._rows; ++j ) {
                        free( (*b)._grid[j] );
                    }
                    free( (*b)._grid );
                    exit( EXIT_SUCCESS );
                } else {                        /* PARENT */
                    close( p[1] );              p[1] = -1;
                    int bytesRead = read( p[0], &numMoves[i], sizeof( int ) );
                    if ( bytesRead == -1 ) {
                        fprintf( stderr, "ERROR: read() failed.\n" );
                        return EXIT_FAILURE;
                    } else {
                        printf( "PID %d: Received %d from child pid %d\n",
                                getpid(), numMoves[i], pids[i] );
                    }
                }
            }
            while ( poss > 0 ) {
#ifdef NO_PARALLEL
                wait( NULL );
#endif

                if ( numMoves[poss - 1] > tmp._moves ) {
                    tmp._moves = numMoves[poss - 1];
                }
                --poss;
            }
            free( numMoves );
        } else {
            /* poss <= 1 */
            tmp._curr = moveTo[0];
            tmp._grid[tmp._curr._y][tmp._curr._x] = 'k';
            ++tmp._moves;
            tour( &tmp );
        }
    } else {
        /* poss <= 0 */
        printf( "PID %d: Dead end after move #%d\n", getpid(), tmp._moves );

#ifdef DISPLAY_BOARD
        printBoard( getpid(), tmp );
#endif
    }

    free( moveTo );
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

            int rc = tour( &touring );
            if ( !rc ) {
                /* EXIT_SUCCESS */
                printf( "PID %d: Best solution found visits %d squares (out of %d)\n",
                        getpid(), touring._moves, ( touring._rows * touring._cols ) );
            } else {
                /* EXIT_FAILURE */
                fprintf( stderr, "ERROR: knight's tour failed.\n" );
            }

            /* Freeing allocated memory in board. */
#ifdef DEBUG_MODE
            printf( "    freeing memory...\n" );
#endif

            for ( int i = 0; i < touring._rows; ++i ) {
                free( touring._grid[i] );       touring._grid[i] = NULL;
            }
            free( touring._grid );              touring._grid = NULL;
            return rc;
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
