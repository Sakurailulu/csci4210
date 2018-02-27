/* homework2.c
 * Griffin Melnick, melnig@rpi.edu
 */

#include <limits.h>
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

#define PAR_PID getpid()
#define COORD_SIZE sizeof( Coord )
#define BOARD_SIZE sizeof( Board )

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
int tour( Board bd );

/* -------------------------------------------------------------------------- */

int max( int n, int nums[] ) {
    int max = INT_MIN;
    for ( int i = 0; i < n; ++i ) {
        max = ( ( max > nums[i] ) ? max : nums[i] );
    }
    return max;
}


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
            printf( "  poss. move %d --> (%d, %d)\n", poss, new._x, new._y );
#endif
        }
    }
    return poss;
}


void step( Board * bd, Coord to ) {
    (*bd)._curr = to;
    (*bd)._grid[(*bd)._curr._y][(*bd)._curr._x] = 'k';
    ++( (*bd)._moves );
}


int tour( Board bd ) {
    int sol = 0;

    Coord moves[8];
    int poss = findPoss( bd, moves );

    int p[2];
    if ( poss >= 1 ) {
        if ( poss > 1 ) {
            printf( "PID %d: %d moves possible after move #%d\n", getpid(), 
                    poss, bd._moves );
#ifdef DISPLAY_BOARD
            printBoard( bd, getpid(), false );
#endif

            int sols[poss];
            for ( int i = 0; i < poss; ++i ) {
                int rc = pipe( p );
                if ( rc < 0 ) {
                    fprintf( stderr, "ERROR: pipe() %d failed\n", ( i + 1 ) );
                    exit( EXIT_FAILURE );
                }

                pid_t pid = fork();
                if ( pid < 0 ) {
                    fprintf( stderr, "ERROR: fork() %d failed\n", ( i + 1 ) );
                    exit( EXIT_FAILURE );
                } else if ( pid == 0 ) {        /* CHILD */
                    step( &bd, moves[i] );
                    tour( bd );
                    break;
                } else {                        /* PARENT */
#ifdef NO_PARALLEL
                    wait( NULL );
#endif
#ifndef NO_PARALLEL
                    int status;
                    wait( &status );
                    if ( WIFSIGNALED( status ) ) {
                        if ( getpid() != PAR_PID ) { exit( EXIT_FAILURE ); }
                        else { return EXIT_FAILURE; }
                    } else {
                        /* !WIFSIGNALED( status ) */
                        close( p[1] );          p[1] = -1;
                        int in = read( p[0], &sols[i], sizeof( int ) );
                        if ( in < 0 ) {
                            fprintf( stderr, "ERROR: read() failed\n" );
                            exit( EXIT_FAILURE );
                        } else {
                            printf( "PID %d: Received %d from child\n", getpid(), 
                                    sols[i] );
                        }
                    }
#endif
                }
            }
            sol = max( poss, sols );
        } else {
            /* poss == 1 */
            step( &bd, moves[0] );
            tour( bd );
        }
    } else {
        /* poss < 1 */
        printf( "PID %d: Dead end after move #%d\n", getpid(), bd._moves );
#ifdef DISPLAY_BOARD
        printBoard( bd, getpid(), false );
#endif

        close( p[0] );                          p[0] = -1;
        int out = write( p[1], &bd._moves, sizeof( int ) );
        if ( out < 0 ) {
            fprintf( stderr, "ERROR: write() failed\n" );
            exit( EXIT_FAILURE );
        } else {
            printf( "PID %d: Sent %d on pipe to parent\n", getpid(), bd._moves );
            exit( EXIT_SUCCESS );
        }
    }

    return sol;
}

/* -------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    setbuf( stdout, NULL );                     /* Prevent stdout buffering. */

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
                        /* j == bd._cols */
                        bd._grid[i][j] = '\0';
                    }
                }
            }
            bd._grid[0][0] = 'k';

#ifdef DEBUG_MODE
            printf( "Initial board details:\n" );
            printf( "_cols = %d, _rows = %d, _moves = %d, _curr = (%d, %d)\n", 
                    bd._cols, bd._rows, bd._moves, bd._curr._x, bd._curr._y );
            printBoard( bd, 0, true );
            printf( "\n\n" );
#endif

            /* Knight's tour simulation. */
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board\n",
                    PAR_PID, bd._cols, bd._rows );
            int sol = tour( bd );

            /* Print solution, free memory, and exit. */
            if ( sol != EXIT_FAILURE ) {
                printf( "PID %d: Best solution found visits %d squares (out of %d)\n",
                        PAR_PID, sol, ( bd._cols * bd._rows ) );
                freeBoard( &bd );
                return EXIT_SUCCESS;
            } else {
                /* sol == EXIT_FAILURE */
                fprintf( stderr, "ERROR: knight's tour failed." );
                freeBoard( &bd );
            }
        
        } else {
            /* ( m <= 2 ) || ( n <= 2 ) :: invalid argument values */
            fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        }
    } else {
        /* argc != 3 :: invalid arguments */
        fprintf( stderr, "ERROR: Invalid argument(s)\n" );
        fprintf( stderr, "USAGE: a.out <m> <n>\n" );
    }

    return EXIT_FAILURE;
}
