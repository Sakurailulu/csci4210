#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void freeBoard( Board * bd ) {
    for ( int i = 0; i < (*bd)._rows; ++i ) {
        free( (*bd)._grid[i] );                 (*bd)._grid[i] = NULL;
    }
    free( (*bd)._grid );                        (*bd)._grid = NULL;
}

void printBoard( Board bd, pid_t pid, bool debug ) {
    for ( int i = 0; i < bd._rows; ++i ) {
        if ( !debug ) { printf( "PID %d: ", pid ); }
        printf( "  %s\n", bd._grid[i] );
    }
}

int tour( Board bd ) {
    int sol = 0;
    return sol;
}

int main( int argc, char * argv[] ) {
    if ( argc == 3 ) {
        char * tmp;
        int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 );
        if ( ( m > 2 ) && ( n > 2 ) ) {

            pid_t parPid = getpid();

            /* Board initialization. */
            /** Static assignments. */
            Board bd = (Board){ ._cols = m, ._rows = n, ._moves = 1,
                                ._curr = (Coord){ ._x = 0, ._y = 0 } };

            /** Memory allocation. */
            bd._grid = calloc( bd._rows, sizeof( char* ) );
            if ( bd._grid == NULL ) {
                fprintf( stderr, "ERROR: Full board calloc() failed\n" );
                return EXIT_FAILURE;
            }
            for ( int i = 0; i < bd._rows; ++i ) {
                /* bd._grid[i] is a string, so we leave room for '\0' */
                bd._grid[i] = calloc( ( bd._cols + 1 ), sizeof( char ) );
                if ( bd._grid[i] == NULL ) {
                    fprintf( stderr, "ERROR: Row %d calloc() failed\n", ( i + 1 ) );
                }
            }

            /** Fill board with marker values. */
            for ( int i = 0; i < bd._rows; ++i ) {
                for ( int j = 0; j <= bd._cols; ++j ) {
                    bd._grid[i][j] = '.';
                }
            }
            bd._grid[0][0] = 'k';

#ifdef DEBUG_MODE
            printf( "Initial board details:\n" );
            printf( "_cols = %d, _rows = %d, _moves = %d\n", bd._cols, bd._rows,
                    bd._moves );
            printf( "_curr = (%d, %d)\n", bd._curr._x, bd._curr._y );
            printBoard( bd, parPid, true );
            printf( "\n" );
#endif

            /* Knight's tour simulation. */
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board\n",
                    parPid, bd._cols, bd._rows );
            int sol = tour( bd );

            /* Print solution, free memory, and exit. */
            if ( sol != EXIT_FAILURE ) {
                printf( "PID %d: Best solution found visits %d squares (out of %d)\n",
                        parPid, sol, ( bd._cols * bd._rows ) );
                freeBoard( &bd );
                return EXIT_SUCCESS;
            } else {
                fprintf( stderr, "ERROR: knight's tour failed\n" );
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

    return EXIT_FAILURE;                        /* if here, we failed */
}
