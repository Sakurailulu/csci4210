/* homework2.c
 * Griffin Melnick, melnig@rpi.edu
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    int _x, _y;
} Coord;

typedef struct {
    int _cols, _rows, _moves;
    Coord _curr;
    char ** _grid;
} Board;

pid_t PAR_PID;

const Coord all[8] = { (Coord){ ._x = +1, ._y = -2 },   /* up, then right */
                       (Coord){ ._x = +2, ._y = -1 },   /* right, then up */
                       (Coord){ ._x = +2, ._y = +1 },   /* right, then down */
                       (Coord){ ._x = +1, ._y = +2 },   /* down, then right */
                       (Coord){ ._x = -1, ._y = +2 },   /* down, the left */
                       (Coord){ ._x = -2, ._y = +1 },   /* left, then down */
                       (Coord){ ._x = -2, ._y = -1 },   /* left, then up */
                       (Coord){ ._x = -1, ._y = -2 } }; /* up, then left */

/* -------------------------------------------------------------------------- */

int max( int n, int nums[] );
void freeBoard( Board * bd );
void printBoard( Board bd, pid_t pid, bool debug );
int findPoss( Board bd, Coord * moves );
void step( Board * bd, Coord to );
void tour( Board bd, int * sol, int from, int to );

/* -------------------------------------------------------------------------- */

/* Helper method to find maximum value in array.
 * @param       n, length of array.
 *              nums, array of values to compare.
 * @return      the maximum value of 'nums' with a lowest possible return value
 *                of 0.
 */
int max( int n, int nums[] ) {
    int max = 0;
    for ( int i = 0; i < n; ++i ) {
        max = ( ( max > nums[i] ) ? max : nums[i] );
    }
    return max;
}


/* Helper method to free allocated memory in Board struct.
 * @param       bd, pointer to Board object.
 * @modifies    bd
 * @effects     frees each row in bd._grid as well as bd._grid.
 */
void freeBoard( Board * bd ) {
    for ( int i = 0; i < (*bd)._rows; ++i ) {
        free( (*bd)._grid[i] );                 (*bd)._grid[i] = NULL;
    }
    free( (*bd)._grid );                        (*bd)._grid = NULL;
}


/* Helper method to print Board struct.
 * @param       bd, Board object to print.
 *              pid, PID of process currently calling the method.
 *              debug, boolean to determine whether of not to print preceding 
 *                "PID ...: "
 */
void printBoard( Board bd, pid_t pid, bool debug ) {
    for ( int i = 0; i < bd._rows; ++i ) {
        if ( !debug ) { printf( "PID %d: ", pid ); }
        printf( "  %s\n", bd._grid[i] );
        fflush( stdout );
    }
}


/* Helper method to find all possible moves from current position in Board.
 * @param       bd, Board object through which to move.
 *              moves, Coord struct array to store valid next moves.
 * @return      count of possible moves.
 * @modifies    moves
 * @effects     store Coord objects from 'all' that are valid next moves.
 */
int findPoss( Board bd, Coord moves[] ) {
    int poss = 0;
    for ( int i = 0; i < 8; ++i ) {
        Coord new = (Coord){ ._x = ( bd._curr._x + all[i]._x ), 
                             ._y = ( bd._curr._y + all[i]._y ) };
        if ( ( ( 0 <= new._x ) && ( new._x < bd._cols ) ) && 
                ( ( 0 <= new._y ) && ( new._y < bd._rows ) ) && 
                ( bd._grid[new._y][new._x] != 'k' ) ) {
            moves[poss] = new;
            ++poss;

#ifdef DEBUG_MODE
            // printf( "  poss. move %d --> (%d, %d)\n", poss, new._x, new._y );
            fflush( stdout );
#endif
        }
    }
    return poss;
}


/* Helper method to take next step in board.
 * @param       bd, pointer to Board object.
 *              to, Coord object to which to move.
 * @modifies    bd
 * @effects     changes '_curr' to 'to', switches "." to "k" in '_grid' at 
 *                '_curr', and increments '_moves'.
 */
void step( Board * bd, Coord to ) {
    (*bd)._curr = to;
    (*bd)._grid[(*bd)._curr._y][(*bd)._curr._x] = 'k';
    ++( (*bd)._moves );
}


/* Touring simulation method.
 * @param       bd, Board to tour.
 *              bestSol, pointer to best solution found.
 * @modifies    bestSol
 * @effects     stores best possible solution.
 */
void tour( Board bd, int * bestSol, int from, int to ) {
    Coord moves[8];
    int poss = findPoss( bd, moves ), sol = 0;

    int sols[poss];
    int p[2];

    if ( poss >= 1 ) {
        if ( poss > 1 ) {
            printf( "PID %d: %d moves possible after move #%d\n", getpid(), 
                    poss, bd._moves );
            fflush( stdout );

#ifdef DISPLAY_BOARD
            printBoard( bd, getpid(), false );
#endif

            int rc;
            if ( ( rc = pipe( p ) ) < 0 ) {
                fprintf( stderr, "ERROR: pipe() failed\n" );
                freeBoard( &bd );

                *bestSol = EXIT_FAILURE;
                exit( EXIT_FAILURE );
            } else {
                /* rc >= 0 */
                from = p[0];                    to = p[1];
            }

            for ( int i = 0; i < poss; ++i ) {
                pid_t pid;
                if ( ( pid = fork() ) < 0 ) {
                    fprintf( stderr, "ERROR: fork() failed\n" );
                    freeBoard( &bd );

                    *bestSol = EXIT_FAILURE;
                    exit( EXIT_FAILURE );
                } else if ( pid == 0 ) {
                    /* CHILD */
                    step( &bd, moves[i] );
                    tour( bd, bestSol, from, to );
                } else {
                    /* PARENT */
#ifdef NO_PARALLEL
                    wait( NULL );
#endif

#ifndef NO_PARALLEL
                    int status;
                    waitpid( pid, &status, 0 );
                    if ( WIFSIGNALED( status ) ) {
                        freeBoard( &bd );

                        *bestSol = EXIT_FAILURE;
                        exit( EXIT_FAILURE );
                    }
#endif

#ifdef DEBUG_MODE
                    printf( "  reading from %d...\n", from );
                    fflush( stdout );
#endif

                    // close ( to );
                    int in = read( from, &( sols[i] ), sizeof( int ) );
                    if ( in < 0 ) {
                        fprintf( stderr, "ERROR: read() failed\n" );
                        freeBoard( &bd );

                        *bestSol = EXIT_FAILURE;
                        exit( EXIT_FAILURE );
                    }
                }
            }

            for ( int i = 0; i < poss; ++i ) {
                printf( "PID %d: Received %d from child\n", getpid(), sols[i] );
                fflush( stdout );
            }

            sol = max( poss, sols );
            *bestSol = ( *bestSol > sol ) ? *bestSol : sol;
            if ( getpid() != PAR_PID ) {
                int out = write( to, bestSol, sizeof( int ) );
                if ( out < 0 ) {
                    fprintf( stderr, "ERROR: write() failed\n" );
                    freeBoard( &bd );

                    *bestSol = EXIT_FAILURE;
                    exit( EXIT_FAILURE );
                } else {
                    printf( "PID %d: All child processes terminated; sent %d on pipe to parent\n",
                            getpid(), *bestSol );
                    fflush( stdout );
                }
            }
        } else {
            /* poss == 1 :: don't fork */
            step( &bd, moves[0] );
            tour( bd, bestSol, from, to );
        }
    } else {
        /* poss < 1 :: dead end */
        printf( "PID %d: Dead end after move #%d\n", getpid(), bd._moves );
        fflush( stdout );

#ifdef DISPLAY_BOARD
        printBoard( bd, getpid(), false );
#endif

#ifdef DEBUG_MODE
        printf( "  writing to %d...\n", to );
        fflush( stdout );
#endif

        close( from );
        int out = write( to, &( bd._moves ), sizeof( int ) );
        if ( out < 0 ) {
            fprintf( stderr, "ERROR: write() failed\n" );
            freeBoard( &bd );

            *bestSol = EXIT_FAILURE;
            exit( EXIT_FAILURE );
        } else {
            printf( "PID %d: Sent %d on pipe to parent\n", getpid(), bd._moves );
            fflush( stdout );
        }

        freeBoard( &bd );
        exit( EXIT_SUCCESS );
    }
}

/* -------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    if ( argc == 3 ) {
        char * tmp;
        int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 );
        if ( ( m > 2 ) && ( n > 2 ) ) {

            /* Board initialization. */
            /** Static assignments. */
            Board bd = (Board){ ._cols = m, ._rows = n, ._moves = 1,
                                ._curr = (Coord){ ._x = 0, ._y = 0 } };

            /** Memory allocation. */
            bd._grid = calloc( bd._rows, sizeof( char* ) );
            if ( bd._grid == NULL ) {
                fprintf( stderr, "ERROR: full board allocation failed\n" );
                return EXIT_FAILURE;
            }
            for ( int i = 0; i < bd._rows; ++i ) {
                bd._grid[i] = calloc( ( bd._cols + 1 ), sizeof( char ) );
                if ( bd._grid[i] == NULL ) {
                    fprintf( stderr, "ERROR: row allocation failed\n" );
                    return EXIT_FAILURE;
                }
            }

            /** Fill board with markers. */
            for ( int i = 0; i < bd._rows; ++i ) {
                for ( int j = 0; j <= bd._cols; ++j ) {
                    if ( j != bd._cols ) {
                        bd._grid[i][j] = '.';
                    } else {
                        /* j == bd._cols :: last character in string */
                        bd._grid[i][j] = '\0';
                    }
                }
            }
            bd._grid[0][0] = 'k';

#ifdef DEBUG_MODE
            printf( "Initial board details:\n" );
            printf( "_cols = %d, _rows = %d, _moves = %d, _curr = (%d, %d)\n", 
                    bd._cols, bd._rows, bd._moves, bd._curr._x, bd._curr._y );
            fflush( stdout );

            printBoard( bd, 0, true );
#endif

            /* Knight's tour simulation. */
            int sol = 0;
            PAR_PID = getpid();
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board\n",
                    PAR_PID, bd._cols, bd._rows );
            fflush( stdout );

            tour( bd, &sol, 0, 0 );
            if ( getpid() != PAR_PID ) { exit( EXIT_SUCCESS ); }

            /* Print solution, free memory, and exit. */
            if ( sol != EXIT_FAILURE ) {
                printf( "PID %d: Best solution found visits %d squares (out of %d)\n",
                        PAR_PID, sol, ( bd._cols * bd._rows ) );
                fflush( stdout );

                freeBoard( &bd );
                return EXIT_SUCCESS;
            } else {
                /* sol == EXIT_FAILURE */
                fprintf( stderr, "ERROR: knight's tour failed.\n" );
                freeBoard( &bd );
            }
        
        } else {
            /* ( m <= 2 ) || ( n <= 2 ) :: invalid argument values */
            fprintf( stderr, "ERROR: Invalid argument(s)\n" );
            fprintf( stderr, "USAGE: a.out <m> <n>\n" );
        }
    } else {
        /* argc != 3 :: invalid arguments */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <m> <n>\n" );
    }

    return EXIT_FAILURE;
}
