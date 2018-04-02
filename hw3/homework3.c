/* homework3.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Program info
 *
 *   bash$ a.out <m> <n> [<k>]
 *
 * where: <m> is the width, number of columns, of the board; <n> is the height, 
 * number of rows, of the board; and the optional <k> is the fewest number of 
 * squares allowed on dead end boards printed out.
 */

#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int _x, _y;
} Coord;

#define COORD_SIZE sizeof( Coord )

typedef struct {
    int _cols, _rows, _moves;
    Coord _curr;
    char ** _grid;
} Board;

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
void freeBoard( Board * bd );
void printBoard( Board bd, bool debug );
void step( Board * bd, Coord to );
void tour( Board bd, Board ** bds, int * seen, int * maxTour );

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
                (bd._grid[to._y][to._x] != 'k') ) {
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
 * @param       bd, pointer to Board to free.
 * @modifies    bd
 * @effects     frees all allocated memory in rows and entire board.
 */
void freeBoard( Board * bd ) {
    for ( int i = 0; i < (*bd)._rows; ++i ) {
        free( (*bd)._grid[i] );                 (*bd)._grid[i] = NULL;
    }
    free( (*bd)._grid );                        (*bd)._grid = NULL;
}


/* Board printing helper.
 * @param       bd, Board to print.
 *              debug, whether to include tid or not.
 */
void printBoard( Board bd, bool debug ) {
    for ( int i = 0; i < bd._rows; ++i ) {
        if ( !debug ) { printf( "THREAD %u", (unsigned int)pthread_self() ); }
        if ( i <= 0 ) { printf( " > %s\n", bd._grid[i] ); }
        else {          printf( "   %s\n", bd._grid[i] ); }
        fflush( stdout );
    }
}


/* Helper to move knight in Board.
 * @param       bd, pointer to Board in which to step.
 *              to, Coord of new move.
 * @modifies    bd
 * @effects     bd._moves, increments by 1.
 *              bd._curr, sets to 'to'.
 *              bd._grid, changes "." to "k" at 'to'.
 */
void step( Board * bd, Coord to ) {
    ++(*bd)._moves;
    (*bd)._curr = to;
    (*bd)._grid[ ((*bd)._curr)._y ][ ((*bd)._curr)._x ] = 'k';
}


/* Touring simulation.
 * @param       bd, Board to tour.
 *              bds, pointer to Board pointer to tracker of all Boards made.
 *              seen, int pointer to length tracker for 'bds'.
 *              maxTour, int pointer to maximum found solution.
 * @modifies    seen, bds
 * @effects     increments 'seen' to count of Boards seen.
 *              adds all Boards seen to 'bds'.
 */
void tour( Board bd, Board ** bds, int * seen, int * maxTour ) {
    /* Find possible moves. */
    Coord moves[8];
    int poss = findPoss( bd, moves );

    /* Determine which path to follow ( multiple moves, one move, dead end ). */
    if ( poss > 0 ) {
        if ( poss > 1 ) {
            printf( "THREAD %u: %d moves possible after move #%d; creating \
                    threads\n", (unsigned int)pthread_self(), poss, bd._moves );
        } else {
            /* poss == 1 :: don't create new thread */
            step( &bd, moves[0] );
            tour( bd, bds, seen, maxTour );
        }
    } else {
        /* poss <= 0 :: dead end */
        printf( "THREAD %u: Dead end after move #%d\n", 
                (unsigned int)pthread_self(), bd._moves );
        fflush( stdout );

        /* Add dead end Board to tracker. */
        ++(*seen);
        *bds = realloc( *bds, ( (*seen + 1) * BOARD_SIZE ) );
        *bds[*seen] = bd;
    }
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
            Board bd = (Board){ ._cols = m, ._rows = n, ._moves = 1, 
                                ._curr = (Coord){ ._x = 0, ._y = 0 } };

            /** Dynamic assignments. **/
            bd._grid = calloc( bd._rows, sizeof(char*) );
            if ( !bd._grid ) {
                fprintf( stderr, "ERROR: Could not allocate memory for board\n" );
                return EXIT_FAILURE;
            }

            for ( int i = 0; i < bd._rows; ++i ) {
                bd._grid[i] = calloc( (bd._cols + 1), sizeof(char) );
                if ( !bd._grid[i] ) {
                    fprintf( stderr, ("ERROR: Could not allocate memory for \
                            row %d"), (i + 1) );
                    return EXIT_FAILURE;
                } else {
                    /* bd._grid[i] :: allocation successful, fill with markers */
                    for ( int j = 0; j <= bd._cols; ++j ) {
                        if ( j < bd._cols ) {
                            bd._grid[i][j] = '.';
                        } else {
                            /* j >= bd._cols :: end of string */
                            bd._grid[i][j] = '\0';
                        }
                    }
                }
            }
            bd._grid[ (bd._curr)._y ][ (bd._curr)._x ] = 'k';

#ifdef DEBUG_MODE
            printf( "Board details:\n" );
            printf( "  _cols = %d, _rows = %d, _moves = %d, _curr = (%d, %d)\n", 
                    bd._cols, bd._rows, bd._moves, (bd._curr)._x, (bd._curr)._y );
            fflush( stdout );

            printBoard( bd, 1 );
            printf( '\n' );
            fflush( stdout );
#endif

            printf( "THREAD %u: Solving the knight's tour problem for a %dx%d \
                    board\n", (unsigned int)pthread_self(), bd._rows, bd._cols );
            fflush( stdout );

            /* Initialize touring pointers. */
            int seen = 0, maxTour = 1;
            Board * bds = calloc( 1, BOARD_SIZE );
            bds[seen] = bd;                     ++seen;

            /* Tour. */
            tour( bd, &bds, &seen, &maxTour );

            printf( "THREAD %u: Best solution found visits %d square%s (out \
                    of %d)\n", (unsigned int)pthread_self(), maxTour, 
                    ( (maxTour > 1) ? "s" : ""), (bd._cols * bd._rows) );
            fflush( stdout );

            if ( argc == 4 ) {
                for ( int i = 0; i < seen; ++i ) {
                    if ( bds[i]._moves >= k ) {
                        printBoard( bds[i], 0 );
                    }
                }
            }

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

