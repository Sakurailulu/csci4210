/* homework2.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Find the solution to the knight's tour problem for a board with given 
 * dimensions. Take an m x n grid and, starting from the top left corner of the 
 * grid, use forking to generate all possible children processes and pipe the 
 * best solution back to the parent. Ideally, we find the maximum number of 
 * moves that a knight can take in a single board. This is called via:
 *
 *   bash$ ./a.out <m> <n>
 *
 * Where both *m* and *n* are greater than two. Otherwise an error is given and 
 * the program fails.
 *
 * Possible additional compiling flags are:
 *   - DEBUG_MODE, to print debugging output.
 *   - DISPLAY_BOARD, to give a printed representation of the Board.
 *   - NO_PARALLEL, to force all parent processes to wait tostop interleaving.
 *   - TEST, run test method(s).    ** cannot be used with other flags **
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/* Helper structs. */
/** Provides x-y pairing for grid positions. */
typedef struct {
    int _x, _y;
} Pair;

/** Board representation. */
typedef struct {
    int _cols, _rows, _moves;
    Pair _curr;
    char ** _grid;
} Board;

#define PARENT getpid()
#define PAIR_SIZE sizeof( Pair )
#define BOARD_SIZE sizeof( Board )

const Pair all[8] = { (Pair){ ._x = +1, ._y = -2 },   /* up, then right */
                      (Pair){ ._x = +2, ._y = -1 },   /* right, then up */
                      (Pair){ ._x = +2, ._y = +1 },   /* right, then down */
                      (Pair){ ._x = +1, ._y = +2 },   /* down, then right */
                      (Pair){ ._x = -1, ._y = +2 },   /* down, then left */
                      (Pair){ ._x = -2, ._y = +1 },   /* left, then down */
                      (Pair){ ._x = -2, ._y = -1 },   /* left, then up */
                      (Pair){ ._x = -1, ._y = -2 } }; /* up, then left */

void freeBoard( Board * b );
void printBoard( Board b, pid_t pid, bool debug );
int max( int l, int r );
int findPoss( Board b, Pair * moves );
void step( Board * b, Pair move );
int tour( Board * b );

/* -------------------------------------------------------------------------- */

#ifdef TEST

#include <assert.h>

bool testMax() {
    bool passed = false;
    passed = ( 12 != max( 12, 13 ) );
    passed = ( 13 == max( 12, 13 ) );
    passed = ( 12 == max( 12, 12 ) );
    passed = ( 13 != max( 12, 11 ) );
    return passed;
}
    
    
bool testFindPoss() {
    bool passed = false;
    return passed;
}
    
    
bool testStep() {
    bool passed = false;
    return passed;
}
    
    
bool testTour() {
    bool passed = false;
    return passed;
}


void testAll() {
    printf( "testing max()...\n" );
    assert( testMax() );
    // printf( "testing findPoss()...\n" );
    // assert( testFindPoss() );
    // printf( "testing step()...\n" );
    // assert( testStep() );
    // printf( "testing tour()...\n" );
    // assert( testTour() );
}

#endif

/* -------------------------------------------------------------------------- */

/* Board freeing helper method.
 * @param       b, pointer to Board to be freed.
 * @modifies    b
 * @effects     frees all dynamically allocated memory in 'b'.
 */
void freeBoard( Board * b ) {
    for ( int i = 0; i < (*b)._rows; ++i ) {
        free( (*b)._grid[i] );                  (*b)._grid[i] = NULL;
    }
    free( (*b)._grid );
}


/* Board printing helper method.
 * @param       b, Board to print.
 *              pid, PID of current process for formatting.
 *              debug, boolean to remove formatting if printing for debug.
 */
void printBoard( Board b, pid_t pid, bool debug ) {
    for ( int i = 0; i < b._rows; ++i ) {
        if ( !debug ) { printf( "PID %d: ", pid ); }
        printf( "  %s\n", b._grid[i] );
    }
}


/* Maximum helper method.
 * @param       l, int to compare.
 *              r, int to compare.
 * @return      max value between 'l' and 'r'.
 */
int max( int l, int r ) {
    return ( ( l > r ) ? l : r );
}


/* Helper method to find all possible next moves.
 * @param       b, Board pointer in which to find valid moves.
 *              moves, Pair array to store all found valid moves.
 * @return      count of found valid moves.
 * @modifies    moves
 * @effects     stores all Pairs with new position after valid move.
 */
int findPoss( Board b, Pair * moves ) {
    int poss = 0;
    for ( int i = 0; i < 8; ++i ) {
        Pair tmp = (Pair){ ._x = ( b._curr._x + all[i]._x ), 
                           ._y = ( b._curr._y + all[i]._y ) };
        if ( ( ( 0 <= tmp._y ) && ( tmp._y < b._rows ) ) && 
                ( ( 0 <= tmp._x ) && ( tmp._x < b._cols ) ) && 
                ( b._grid[tmp._y][tmp._x] != 'k' ) ) {
            moves[i] = tmp;
            ++poss;
        }
    }
    return poss;
}


/* Helper method to step through a single move during a tour.
 * @param       b, pointer to Board in which to step.
 *              move, step to take.
 * @modifies    b
 * @effects     changes '_curr', adds *k* to '_grid', and increments '_moves'.
 */
void step( Board * b, Pair move ) {
    (*b)._curr = move;
    (*b)._grid[ (*b)._curr._y ][ (*b)._curr._x ] = 'k';
    ++( (*b)._moves );
}


/* Touring method.
 * @param       b, Board to tour.
 * @return      best solution if found without error, -1 otherwise.
 * @modifies    b
 * @effects     performs knight's tour.
 */
int tour( Board * b ) {
    int poss = 0, sol = 0, i;
    Board tmp = *b;
    Pair * moves = calloc( 8, PAIR_SIZE );

    poss = findPoss( tmp, moves );
    //int pipes[poss][2];
    if ( poss >= 1 ) {
        if ( poss > 1 ) {
            printf( "PID %d: %d moves possible after move #%d\n", getpid(), 
                    poss, tmp._moves );
#ifdef DISPLAY_BOARD
            printBoard( tmp, getpid(), false );
#endif

            //pid_t pids[poss];
            for ( i = 0; i < poss; ++i ) {
                
#ifdef NO_PARALLEL
                wait( NULL );
#endif
            }
        } else {
            /* poss == 1 :: don't fork */
            step( &tmp, moves[0] );
            tour( &tmp );
        }
    } else {
        /* poss < 1 :: dead end */
        printf( "PID %d: Dead end after move #%d\n", getpid(), tmp._moves );
#ifdef DISPLAY_BOARD
        printBoard( tmp, getpid(), false );
#endif

        //close( pipes[i][0] );                   pipes[i][0] = -1;
        //int written = write( pipes[i][0], &sol, sizeof( int ) );
    }

    free( moves );
    b = &tmp;
    return sol;
}


/* -------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    setbuf( stdout, NULL );

    printf( "%s\n", ( ( PARENT == getpid() ) ? "true" : "false" ) );

#ifndef TEST

    if ( argc == 3 ) {
        char * tmp;
        int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 );
        if ( ( m > 2 ) && ( n > 2 ) ) {
            /* Initital board creation. */
            /** Static assignments. */
            Board toTour = (Board){ ._cols = m, ._rows = n, ._moves = 1,
                                    ._curr = (Pair){ ._x = 0, ._y = 0 } };
            
            /** Memory allocation. */
            toTour._grid = calloc( toTour._rows, sizeof( char* ) );
            if ( toTour._grid == NULL ) {
                fprintf( stderr, "ERROR: first calloc() failed.\n" );
                return EXIT_FAILURE;
            } else {
                int boardWidth = toTour._cols + 1;
                for ( int i = 0; i < toTour._rows; ++i ) {
                    toTour._grid[i] = calloc( boardWidth, sizeof( char ) );
                    if ( toTour._grid[i] == NULL ) {
                        fprintf( stderr, "ERROR: second calloc() failed.\n" );
                        return EXIT_FAILURE;
                    }
                }
            }

            /** Fill grid with marker values. */
            for ( int i = 0; i < toTour._rows; ++i ) {
                for ( int j = 0; j <= toTour._cols; ++j ) {
                    if ( j < toTour._cols ) { toTour._grid[i][j] = '.'; }
                    else { toTour._grid[i][j] = '\0'; }
                }
            }
            toTour._grid[0][0] = 'k';

#ifdef DEBUG_MODE
            printf( "Initial board details:\n" );
            printf( "_cols = %d, _rows = %d, _moves = %d\n", toTour._cols, 
                    toTour._rows, toTour._moves );
            printf( "_curr = (%d, %d)\n", toTour._curr._x, toTour._curr._y );
            printf( "_grid =\n" );
            printBoard( toTour, getpid(), true );
            printf( "\n" );
#endif
        
            /* Touring simulation. */
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board.\n", 
                    getpid(), toTour._cols, toTour._rows );
            
            int max = tour( &toTour );
            printf( "PID %d: Best solution found visits %d squares (out of %d)\n",
                        getpid(), max, ( toTour._cols * toTour._rows ) );

            freeBoard( &toTour );
            return EXIT_SUCCESS;
        } else {
            /* ( m <= 2 ) || ( n <= 2 ) :: invalid argument values */
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

#endif


#ifdef TEST
    /* test methods */
    testAll();
    printf( "All methods work as expected.\n" );
#endif
}

