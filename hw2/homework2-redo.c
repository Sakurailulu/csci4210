/* homework2-redo.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * This program simulates a solution to the knight's tour problem using multiple
 * processes. The program is called using
 *
 *   bash$ a.out <m> <n>
 *
 * where <m> is the width, number of columns, of the board; and <n> is the 
 * height, number of rows, of the board.
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define VISITED 'k'
#define UNVISITED '.'

typedef struct {
    int _x, _y;
} Coord;

typedef struct {
    int _cols, _rows, _moves;
    Coord _curr;
    char ** _grid;
} Board;

#define COORD_SIZE sizeof( Coord )
#define BOARD_SIZE sizeof( Board )

/* Coord array holding all possible steps. */
const Coord all[8] = { (Coord){ ._x = +1, ._y = -2 },   /* up, then right */
                       (Coord){ ._x = +2, ._y = -1 },   /* right, then up */
                       (Coord){ ._x = +2, ._y = +1 },   /* right, then down */
                       (Coord){ ._x = +1, ._y = +2 },   /* down, then right */
                       (Coord){ ._x = -1, ._y = +2 },   /* down, then left */
                       (Coord){ ._x = -2, ._y = +1 },   /* left, then down */
                       (Coord){ ._x = -2, ._y = -1 },   /* left, then up */
                       (Coord){ ._x = -1, ._y = -2 } }; /* up, then left */

/* -------------------------------------------------------------------------- */

int arrayMax( int n, int nums[] );
int findPoss( Board bd, Coord moves[] );
void freeBoard( Board * bdPtr );
void printBoard( Board bd, int debug );
Board step( Board bd, Coord to );
void tour ( Board bd, int * maxTourPtr, int r, int w );

/* -------------------------------------------------------------------------- */

/* Helper to find max value in array.
 * @param       n, length of array.
 *              nums, array of which to find max.
 * @return      max value of array.
 */
int arrayMax( int n, int nums[] ) {
    int max = INT_MIN;
    for ( int i = 0; i < n; ++i ) {
        max = ( (nums[i] > max) ? nums[i] : max );
    }
    return max;
}


/* Helper to find all possible moves from current position.
 * @param       bd, Board in which to find possible moves.
 *              moves, array in which to store possible moves.
 * @return      count of valid moves found.
 */
int findPoss( Board bd, Coord moves[] ) {
    int poss = 0;
    for ( int i = 0; i < 8; ++i ) {
        Coord to = (Coord){ ._x = ( (bd._curr)._x + all[i]._x ),
                            ._y = ( (bd._curr)._y + all[i]._y ) };
        if ( ( (0 <= to._x) && (to._x < bd._cols) ) &&
                ( (0 <= to._y) && (to._y < bd._rows) ) &&
                (bd._grid[to._y][to._x] != VISITED) ) {
            moves[poss] = to;
            ++poss;

#ifdef DEBUG_MODE
            printf( "  poss. move at (%d, %d)\n", to._x, to._y );
            fflush( stdout );
#endif
        }
    }
    return poss;
}


/* Board freeing helper.
 * @param       bdPtr, pointer to Board to free.
 * @modifies    bdPtr
 * @effects     frees all allocated memory in rows and entire board.
 */
void freeBoard( Board * bdPtr ) {
    for ( int i = 0; i < (*bdPtr)._rows; ++i ) {
        free( (*bdPtr)._grid[i] );              (*bdPtr)._grid[i] = NULL;
    }
    free( (*bdPtr)._grid );                     (*bdPtr)._grid = NULL;
}


/* Board printing helper.
 * @param       bd, Board to print.
 *              debug, whether to include pid or not.
 */
void printBoard( Board bd, int debug ) {
    for ( int i = 0; i < bd._rows; ++i ) {
        if ( !debug ) { printf( "PID %d:", getpid() ); }
        printf( "  %s\n", bd._grid[i] );
        fflush( stdout );
    }
}


/* Helper to move knight in Board.
 * @param       bd, Board in which to step.
 *              to, Coord of new move.
 * @return      Board with step taken.
 */
Board step( Board bd, Coord to ) {
    ++bd._moves;
    bd._curr = to;
    bd._grid[ (bd._curr)._y ][ (bd._curr)._x ] = VISITED;
    return bd;
}


/* Touring simulation.
 * @param       bd, Board to tour.
 *              maxTourPtr, pointer to value of longest tour.
 */
void tour( Board bd, int * maxTourPtr, int r, int w ) {
    /* Find possible moves. */
    Coord moves[8];
    int poss = findPoss( bd, moves );

    int p[2], sols[poss];

    /* Determine which path to follow ( multiple moves, one move, dead end ). */
    if ( poss > 0 ) {
        if ( poss > 1 ) {
            printf( "PID %d: %d moves possible after move #%d\n", getpid(),
                    poss, bd._moves );
            fflush( stdout );
#ifdef DISPLAY_BOARD
            printBoard( bd, 0 );
#endif

            pid_t pids[poss];
            int i = 0, rc = 0, reading = 0, writing = 0;

            /* Create child processes. */
            for ( i = 0; i < poss; ++i ) {
                fflush( stdout );               /* Flush for safety in fork. */

                rc = pipe( p );
                if ( rc < 0 ) {
                    fprintf( stderr, "ERROR: pipe() caused tour to fail\n" );
                } else {
                    /* rc >= 0 :: pipe succeeded */
                    reading = p[0];             writing = p[1];
                }

                pids[i] = fork();
                if ( pids[i] < 0 ) {
                    fprintf( stderr, "ERROR: fork() caused tour to fail\n" );
                } else if ( pids[i] == 0 ) {
                    /* Child process. */
                    bd = step( bd, moves[i] );
                    tour( bd, maxTourPtr, reading, writing );
                } else {
                    /* Parent process. */
                    wait( NULL );
                }
            }

            for ( i = 0; i < poss; ++i ) {
                close( w );                     w = -1;
                int in = read( r, &sols[i], sizeof(int) );
                if ( in < 0 ) {
                    fprintf( stderr, "ERROR: read() caused tour to fail\n" );
                } else {
                    /* in >= 0 :: read succeeded */
                    printf( "PID %d: Received %d from child\n", getpid(),
                            sols[i] );
                    fflush( stdout );
                }
            }

            *maxTourPtr = ( ( *maxTourPtr > arrayMax(poss, sols) ) ? 
                    *maxTourPtr : arrayMax( poss, sols ) );
        } else {
            /* poss == 1 :: don't fork */
            bd = step( bd, moves[0] );
            tour( bd, maxTourPtr, r, w );
        }
    } else {
        /* poss <= 0 :: dead end */
        printf( "PID %d: Dead end after move #%d\n", getpid(), bd._moves );
        fflush( stdout );

#ifdef DISPLAY_BOARD
        printBoard( bd, 0 );
#endif

        close( r );                             r = -1;
        int out = write( w, &bd._moves, sizeof(int) );
        if ( out < 0 ) {
            fprintf( stderr, "ERROR: write() caused tour to fail\n" );
        } else {
            /* out >= 0 :: write succeeded */
            printf( "PID %d: Sent %d on pipe to parent\n", getpid(), bd._moves );
            fflush( stdout );
        }

        freeBoard( &bd );
        exit( EXIT_SUCCESS );
    }
}

/* -------------------------------------------------------------------------- */

int main ( int argc, char * argv[] ) {
    if ( argc == 3 ) {
        char * tmp;
        const int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 );
        if ( (m > 2) && (n > 2) ) {
            /* Board initialization. */
            /** Static assignments. **/
            Board tourBd = (Board){ ._cols = n, ._rows = m, ._moves = 1,
                                    ._curr = (Coord){ ._x = 0, ._y = 0 } };

            /** Dynamic assignments. **/
            tourBd._grid = calloc( tourBd._rows, sizeof(char*) );
            if ( !tourBd._grid ) {
                fprintf( stderr, "ERROR: Could not allocate memory for board\n" );
                return EXIT_FAILURE;
            }

            for ( int i = 0; i < tourBd._rows; ++i ) {
                tourBd._grid[i] = calloc( (tourBd._cols + 1), sizeof(char) );
                if ( !tourBd._grid[i] ) {
                    fprintf( stderr, "ERROR: Could not allocate memory for "
                            "row %d\n", (i + 1) );
                    return EXIT_FAILURE;
                } else {
                    /* tourBd._grid[i] :: allocation successful, fill markers */
                    for ( int j = 0; j <= tourBd._cols; ++j ) {
                        if ( j < tourBd._cols ) {
                            tourBd._grid[i][j] = UNVISITED;
                        } else {
                            /* j == tourBd._cols :: end of string */
                            tourBd._grid[i][j] = '\0';
                        }
                    }
                }
            }

            tourBd._grid[ (tourBd._curr)._y ][ (tourBd._curr)._x ] = VISITED;
#ifdef DEBUG_MODE
            printf( "Board details:\n" );
            printf( "  _cols = %d, _rows = %d, _moves = %d, _curr = (%d, %d)\n",
                    tourBd._cols, tourBd._rows, tourBd._moves, (tourBd._curr)._x,
                    (tourBd._curr)._y );
            fflush( stdout );

            printBoard( tourBd, 1 );
            printf( "\n" );
            fflush( stdout );
#endif

            int maxTour = -1;
            printf( "PID %d: Solving the knight's tour problem for a %dx%d "
                    "board\n", getpid(), tourBd._rows, tourBd._cols );
            fflush( stdout );

            /* Tour. */
            tour( tourBd, &maxTour, 0, 0 );
            if ( maxTour != EXIT_FAILURE ) {
                printf( "PID %d: Best solution found visits %d square%s (out "
                        "of %d)\n", getpid(), maxTour, ( (maxTour != 1) ? "s" : 
                        "" ), (tourBd._rows * tourBd._cols) );
                fflush( stdout );

                freeBoard( &tourBd );
                return EXIT_SUCCESS;
            } else {
                /* maxTour == EXIT_FAILURE */
                fprintf( stderr, "ERROR: Knight's tour failed\n" );
                freeBoard( &tourBd );
            }
        } else {
            /* (m <= 2) || (n <= 2) :: invalid arguments */
            fprintf( stderr, "ERROR: Invalid argument(s)\n" );
            fprintf( stderr, "USAGE: a.out <m> <n>\n" );
        }
    } else {
        /* argc != 3 :: too few/many arguments */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <m> <n>\n" );
    }

    return EXIT_FAILURE;
}
