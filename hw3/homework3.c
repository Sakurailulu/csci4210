/* homework3.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Program info
 *
 *   bash$ a.out <m> <n> [<k>]
 *
 * where: <m> is the width, number of columns, of the board; <n> is the height, 
 * number of rows, of the board; and the optional <k> is the fewest number of 
 * squares allowed on boards printed out.
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
int tour( Board bd, int * seen, Board * bds );

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
 * @param
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


/* Touring simulation. */
int tour( Board bd, int * seen, Board * bds ) {
    int maxTour = 1;
    return maxTour;
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
                    fprintf( stderr, ("ERROR: Could not allocate memory for row %d"), 
                            (i + 1) );
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

            printf( "THREAD %u: Solving the knight's tour problem for a %dx%d board\n",
                    (unsigned int)pthread_self(), bd._rows, bd._cols );
            fflush( stdout );

            int seen = 0;
            Board * bds = calloc( 0, BOARD_SIZE );
            int maxTour = tour( bd, &seen, bds );

            printf( "THREAD %u: Best solution found visits %d squares (out of %d)\n",
                    (unsigned int)pthread_self(), maxTour, (bd._cols * bd._rows) );
            fflush( stdout );
            for ( int i = 0; i < seen; ++i ) {
               printBoard( bds[i], 0 );
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

