/* homework2.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Info
 *
 *   bash$ ./a.out <m> <n>
 *
 * where both 'm' and 'n' are greater than two. Otherwise, an error is shown
 * and the programs fails.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


/* Helper structs */
/** (x, y) pairing to simplify moves. */
typedef struct {
    int _x, _y;
} Pair;

/** Board properties to simplify parameters. */
typedef struct {
    int _rows, _cols, _moves;
    Pair _curr;
    char ** _grid;
} Board;

#define PAIR_SIZE sizeof( Pair )
#define BOARD_SIZE sizeof( Board )


const Pair all[8] = { (Pair){ ._x = +1, ._y = -2 },     /* up, then right */
                      (Pair){ ._x = +2, ._y = -1 },     /* right, then up */
                      (Pair){ ._x = +2, ._y = +1 },     /* right, then down */
                      (Pair){ ._x = +1, ._y = +2 },     /* down, then right */
                      (Pair){ ._x = -1, ._y = +2 },     /* down, then left */
                      (Pair){ ._x = -2, ._y = +1 },     /* left, then down */
                      (Pair){ ._x = -2, ._y = -1 },     /* left, then up */
                      (Pair){ ._x = -1, ._y = -2 } };   /* up, then left */


int max( int l, int r );
void printBoard( pid_t pid, Board b );
int findPoss( Pair * moves, Board b );
void step( Pair move, Board * b );
int tour( Board * b );


/* -------------------------------------------------------------------------- */

/* Maximum helper method.
 * @param       l, int to compare.
 *              r, int to compare.
 * @return      the maximum value chosen from the two passed values.
 */
int max( int l, int r ) {
    return ( ( l > r ) ? l : r );
}


/* Board printing helper method.
 * @param       pid, pid value passed for DISPLAY_BOARD output.
 *              b, Board to print.
 */
void printBoard( pid_t pid, Board b ) {
    for ( int i = 0; i < b._rows; ++i ) {
#ifndef DEBUG_MODE
        printf( "PID %d: ", pid );
#endif
        printf( "  %s\n", b._grid[i] );
    }
}


/* Board parsing helper method.
 * @param       moves, Pair array to store valid moves.
 *              b, Board to parse for moves.
 * @return      count of found valid moves.
 * @modifies    moves
 * @effects     adds valid moves.
 */
int findPoss( Pair * moves, Board b ) {
    int poss = 0;
    for ( int i = 0; i < 8; ++i ) {
        Pair move = (Pair){ ._x = ( b._curr._x + all[i]._x ),
                            ._y = ( b._curr._y + all[i]._y ) };
        if ( ( ( 0 <= move._y ) && ( move._y < b._rows ) ) &&
                ( ( 0 <= move._x ) && ( move._x < b._cols ) ) &&
                ( b._grid[move._y][move._x] != 'k' ) ) {
            moves[poss] = move;
            ++poss;

#ifdef DEBUG_MODE
            printf( "  poss. move %d -> (%d, %d)\n", poss, move._x, move._y );
#endif
        }
    }

    return poss;
}


/* Single step helper method.
 * @param       move, new spot in grid to which to move.
 *              b, Board through which to step.
 * @modifies    b
 * @effects     adds new "k" to position 'move', increments moves taken.
 */
void step( Pair move, Board * b ) {
    (*b)._curr = move;
    (*b)._grid[ (*b)._curr._y ][ (*b)._curr._x ] = 'k';
    ++( (*b)._moves );
}


/* Touring method.
 * @param       b, Board on which to perform knight's tour.
 * @modifies    b
 * @effects     adds effects of tour to board.
 */
int tour( Board * b ) {
    Board tmp = *b;

    /* Find valid moves from _curr. */
    Pair * moves = calloc( 8, PAIR_SIZE );
    int poss = findPoss( moves, tmp );
    int p[poss][2];

    int i = 0, most = 0;
    if ( poss >= 1 ) {
        if ( poss > 1 ) {
            for ( int j = 0; j < poss; ++j ) {
                int rc = pipe( p[j] );
                if ( rc < 0 ) {
                    fprintf( stderr, "ERROR: pipe() %d failed.\n", ( j + 1 ) );
                    exit( EXIT_FAILURE );
                }
            }

            printf( "PID %d: Multiple moves possible after move #%d\n",
                    getpid(), tmp._moves );

#ifdef DISPLAY_BOARD
                    printBoard( getpid(), tmp );
#endif

            pid_t pids[poss];
            for ( i = 0; i < poss; ++i ) {

                pids[i] = fork();
                if ( pids[i] < 0 ) {
                    fprintf( stderr, "ERROR: fork() %d failed.\n", ( i + 1 ) );
                    exit( EXIT_FAILURE );
                } else if ( pids[i] == 0 ) {
                    step( moves[i], &tmp );
                    tour( &tmp );

                    free( moves );              moves = NULL;
                    for ( int j = 0; j < (*b)._rows; ++j ) {
                        free( (*b)._grid[j] );  (*b)._grid[j] = NULL;
                    }
                    free( (*b)._grid );         (*b)._grid = NULL;
                    exit( EXIT_SUCCESS );
                }
            }

            pid_t wPid;
            i = 0;
            while ( ( wPid = wait( &wPid ) ) > 0 ) {
                int tmp = 0;

                close( p[i][1] );                          p[i][1] = -1;
                int bytesRead = read( p[i][0], &tmp, sizeof( int ) );
                if ( bytesRead < 0 ) {
                    fprintf( stderr, "ERROR: read() %d failed.\n", ( i + 1 ) );
                    exit( EXIT_FAILURE );
                }
                most = max( most, (int)tmp );
                ++i;
            }
        } else {
            /* poss == 1 :: don't fork */
            step( moves[i], &tmp );
            tour( &tmp );
        }
    } else {
        /* poss < 1 :: no valid moves */
        printf( "PID %d: Dead end after move #%d\n", getpid(), tmp._moves );

#ifdef DISPLAY_BOARD
        printBoard( getpid(), tmp );
#endif

        close( p[i][0] );                          p[i][0] = -1;
        int bytesWrite = write( p[i][1], &tmp._moves, sizeof( int ) );
        if ( bytesWrite < 0 ) {
            fprintf( stderr, "ERROR: write() %d failed.\n", ( i + 1 ) );
            exit( EXIT_FAILURE );
        }
        printf( "PID %d: Sending %d on pipe to parent pid %d\n", getpid(),
                tmp._moves, getppid() );
    }

    b = &tmp;
    return most;
}


/* -------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    setbuf( stdout, NULL );                     /* Don't buffer stdout. */

    if ( argc == 3 ) {
        char * tmp;
        int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 );
        if ( ( m > 2 ) && ( n > 2 ) ) {
#ifdef DEBUG_MODE
            printf( "  starting...\n" );
#endif

            /* Initializing Board struct to be used throughout. */
            /** Memory allocation. */
            Board init;
            init._cols = m;                     init._rows = n;
            init._grid = calloc( init._rows, sizeof( char* ) );
            if ( init._grid == NULL ) {
                fprintf( stderr, "ERROR: first calloc() failed.\n" );
                return EXIT_FAILURE;
            } else {
                for ( int i = 0; i < init._rows; ++i ) {
                    init._grid[i] = calloc( init._cols, sizeof( char ) );
                    if ( init._grid[i] == NULL ) {
                        fprintf( stderr, "ERROR: second calloc() failed.\n" );
                        return EXIT_FAILURE;
                    }
                }
            }

            /** Fill grid with marker values. */
            for ( int i = 0; i < init._rows; ++i ) {
                for ( int j = 0; j < init._cols; ++j ) {
                    init._grid[i][j] = '.';
                }
            }
            init._grid[0][0] = 'k';             init._moves = 1;
            init._curr = (Pair){ ._x = 0, ._y = 0 };

#ifdef DEBUG_MODE
            printf( "    Board details:\n" );
            printf( "    _x = %d, _y = %d, _moves = %d\n", init._cols,
                        init._rows, init._moves );
            printf( "    _curr = (%d, %d)\n", init._curr._x, init._curr._y );
#endif

            /* Touring simulation. */
            pid_t pid = getpid();
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board.\n",
                    pid, init._cols, init._rows );

            int max = tour( &init );
            if ( max >= 0 ) {
                printf( "PID %d: Best solution found visits %d squares (out of %d)\n",
                        getpid(), max, ( init._rows * init._cols ) );

                for ( int i = 0; i < init._rows; ++i ) {
                    free( init._grid[i] );          init._grid[i] = NULL;
                }
                free( init._grid );              init._grid = NULL;
                return EXIT_SUCCESS;
            } else {
                /* max < 0 :: tour() returned an error */
                fprintf( stderr, "ERROR: knight's tour failed\n" );

                for ( int i = 0; i < init._rows; ++i ) {
                    free( init._grid[i] );          init._grid[i] = NULL;
                }
                free( init._grid );              init._grid = NULL;
                return EXIT_FAILURE;
            }
        } else {
            /* ( m <= 2 ) || ( n <= 2 ) :: invalid values */
            fprintf( stderr, "ERROR: Invalid argument(s)\n" );
            fprintf( stderr, "USAGE: a.out <m> <n>\n" );
            return EXIT_FAILURE;
        }
    } else {
        /* argc != 3 :: invalid arguments */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <m> <n>\n" );
        return EXIT_FAILURE;
    }
}
