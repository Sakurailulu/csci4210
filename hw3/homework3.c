/* homework3.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * This program simulates a solution to the knight's tour problem using 
 * multithreading via the 'pthread' library. The program is called using
 *
 *   bash$ a.out <m> <n> [<k>]
 *
 * where <m> is the width, number of columns, of the board; <n> is the height,
 * number of rows, of the board; and the optional <k> is the fewest number of
 * squares allowed on dead end boards printed out.
 */

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VISITED 'k'
#define UNVISITED '.'

typedef struct {
    int _x, _y;
} Coord;

typedef struct {
    int _cols, _rows, _moves, _k;
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

/* Variables shared by all threads. */
static Board * endBds;
int maxTour = 0, ended = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* -------------------------------------------------------------------------- */

int arrayMax( int n, int nums[] );
int findPoss( Board bd, Coord moves[] );
void freeBoard( Board * bdPtr );
void printBoard( Board bd, bool debug );
Board step( Board bd, Coord to );
void * tour( void * ptr );

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
            printf( " > poss. move at (%d, %d)\n", to._x, to._y );
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
 *              debug, whether to include tid or not.
 */
void printBoard( Board bd, bool debug ) {
    for ( int i = 0; i < bd._rows; ++i ) {
        if ( !debug ) { printf( "THREAD %u:", (unsigned int)pthread_self() ); }
        if ( i <= 0 ) { printf( " > %s\n", bd._grid[i] ); }
        else {          printf( "   %s\n", bd._grid[i] ); }
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
 * @param       ptr, pointer to Board to tour.
 */
void * tour( void * ptr ) {
    Board bd = *(Board*)ptr;
    free( ptr );

    /* Find possible moves. */
    Coord moves[8];
    int poss = findPoss( bd, moves );

    /* Determine which path to follow ( multiple moves, one move, dead end ). */
    if ( poss > 0 ) {
        if ( poss > 1 ) {
            printf( "THREAD %u: %d moves possible after move #%d; creating "
                    "threads\n", (unsigned int)pthread_self(), poss, bd._moves );

            pthread_t tid[poss];
            int i = 0, rc = 0;

            /* Create child threads. */
            for ( i = 0; i < poss; ++i ) {
                Board * bdPtr = malloc( BOARD_SIZE );
                (*bdPtr) = (Board){ ._cols = bd._cols, ._rows = bd._rows, 
                                    ._moves = bd._moves, ._k = bd._k, 
                                    ._curr = bd._curr };
                (*bdPtr)._grid = calloc( (*bdPtr)._rows, sizeof( char* ) );
                for ( int j = 0; j < (*bdPtr)._rows; ++j ) {
                    (*bdPtr)._grid[j] = strdup( bd._grid[j] );
                }

                // pthread_mutex_lock( &mutex );
                    *bdPtr = step( *bdPtr, moves[i] );
                // pthread_mutex_unlock( &mutex );

                rc = pthread_create( &tid[i], NULL, tour, bdPtr );

                if ( rc != 0 ) {
                    fprintf( stderr, "ERROR: Could not create thread (%d)\n", rc );
                }
            }

            /* Wait for child threads to complete. */
            for ( i = 0; i < poss; ++i ) {
                unsigned int * u_intPtr;
                rc = pthread_join( tid[i], (void **)&u_intPtr );

                if ( rc != 0 ) {
                    fprintf( stderr, "ERROR: Could not join thread (%d)\n", rc );
                }
                free( u_intPtr );
            }
        } else {
            /* poss == 1 :: don't create new thread */
            Board * bdPtr = malloc( BOARD_SIZE );
            *bdPtr = step( bd, moves[0] );
            tour( bdPtr );
        }
    } else {
        /* poss <= 0 :: dead end */
        printf( "THREAD %u: Dead end after move #%d\n",
                (unsigned int)pthread_self(), bd._moves );
        fflush( stdout );

        pthread_mutex_lock( &mutex );
            /* Add dead end Board to tracker. */
            ++ended;
            endBds = realloc( endBds, (ended * BOARD_SIZE) );
            endBds[ (ended - 1) ] = bd;

            /* Update maxTour to current max. */
            maxTour = ( (bd._moves > maxTour) ? bd._moves : maxTour );
        pthread_mutex_unlock( &mutex );

        /* Exit thread. */
        unsigned int * u_intPtr = malloc( sizeof(unsigned int) );
        *u_intPtr = (unsigned int)pthread_self();
        pthread_exit( u_intPtr );
    }

    return NULL;
}

/* -------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    if ( (argc == 3) || (argc == 4) ) {
        /* Check that 'm' and 'n' are greater than two. */
        char * tmp;
        int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 ),
                k = -1;
        if ( argc == 4 ) {
            k = strtol( argv[3], &tmp, 10 );
            if ( (k <= 0) || ( k > (m * n) ) ) {
                fprintf( stderr, "ERROR: Invalid argument(s)\n" );
                fprintf( stderr, "USAGE: a.out <m> <n> [<k>]\n" );
                return EXIT_FAILURE;
            }
        }

        if ( (m > 2) && (n > 2) ) {
            /* Board initialization. */
            /** Static assignments. **/
            Board tourBd = (Board){ ._cols = m, ._rows = n, ._moves = 1, ._k = k,
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
                    fprintf( stderr, ("ERROR: Could not allocate memory for \
                            row %d"), (i + 1) );
                    return EXIT_FAILURE;
                } else {
                    /* bd._grid[i] :: allocation successful, fill markers */
                    for ( int j = 0; j <= tourBd._cols; ++j ) {
                        if ( j < tourBd._cols ) {
                            tourBd._grid[i][j] = UNVISITED;
                        } else {
                            /* j >= bd._cols :: end of string */
                            tourBd._grid[i][j] = '\0';
                        }
                    }
                }
            }
            tourBd._grid[ (tourBd._curr)._y ][ (tourBd._curr)._x ] = VISITED;
            maxTour = 1;

#ifdef DEBUG_MODE
            printf( "Board details:\n" );
            printf( " > _cols = %d, _rows = %d, _moves = %d, _curr = (%d, %d)\n",
                    tourBd._cols, tourBd._rows, tourBd._moves, (tourBd._curr)._x,
                    (tourBd._curr)._y );
            fflush( stdout );

            printBoard( tourBd, 1 );
            printf( "\n" );
            fflush( stdout );
#endif

            printf( "THREAD %u: Solving the knight's tour problem for a %dx%d "
                    "board\n", (unsigned int)pthread_self(), tourBd._cols,
                    tourBd._rows );
            fflush( stdout );

            /* Tour. */
            Board * bdPtr = malloc( BOARD_SIZE );
            *bdPtr = tourBd;
            tour( bdPtr );

            printf( "THREAD %u: Best solution found visits %d square%s (out "
                    "of %d)\n", (unsigned int)pthread_self(), maxTour,
                    ( (maxTour != 1) ? "s" : "" ), (tourBd._cols * tourBd._rows) );
            fflush( stdout );

            if ( argc == 4 ) {
                for ( int i = 0; i < ended; ++i ) {
                    if ( endBds[i]._moves >= k ) {
                        printBoard( endBds[i], 0 );
                    }
                }
            } else {
                /* argc != 4 :: print all dead end boards */
                for ( int i = 0; i < ended; ++i ) {
                    printBoard( endBds[i], 0 );
                }
            }

            free( endBds );
            freeBoard( &tourBd );

            return EXIT_SUCCESS;
        } else {
            /* (m <= 2) || (n <= 2) :: invalid arguments */
            fprintf( stderr, "ERROR: Invalid argument(s)\n" );
            fprintf( stderr, "USAGE: a.out <m> <n> [<k>]\n" );
            return EXIT_FAILURE;
        }
    } else {
        /* (argc != 3) && (argc != 4) :: incorrect number of arguments */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <m> <n> [<k>]\n" );
        return EXIT_FAILURE;
    }
}
