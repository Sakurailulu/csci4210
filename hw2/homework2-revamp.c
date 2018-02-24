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

const Coord changes[8] = { (Coord){ ._x = +1, ._y = -2 },   /* up, then right */ 
                           (Coord){ ._x = +2, ._y = -1 },   /* right, then up */
                           (Coord){ ._x = +2, ._y = +1 },   /* right, then down */
                           (Coord){ ._x = +1, ._y = +2 },   /* down, then right */
                           (Coord){ ._x = -1, ._y = +2 },   /* down, then left */
                           (Coord){ ._x = -2, ._y = +1 },   /* left, then down */
                           (Coord){ ._x = -2, ._y = -1 },   /* left, then up */
                           (Coord){ ._x = -1, ._y = -2 } }; /* up, then left */

int max( int nums[], int count ) {
    int max = 0;
    for ( int i = 0; i < count; ++i ) {
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

int findChanges( Board bd, Coord changed[] ) {
    int poss = 0;
    for ( int i = 0; i < 8; ++i ) {
        Coord new = (Coord){ ._x = ( bd._curr._x + changes[i]._x ),
                             ._y = ( bd._curr._y + changes[i]._y ) };

        if ( ( ( 0 <= new._y ) && ( new._y < bd._rows ) ) && 
                ( ( 0 <= new._x ) && ( new._x < bd._cols ) ) && 
                ( bd._grid[new._y][new._x] != 'k' ) ) {
#ifdef DEBUG_MODE
            printf( "  Valid move %d: (%d, %d)\n", ( poss + 1 ), new._x, new._y );
#endif

            changed[poss] = new;
            ++poss;
        }
    }
    return poss;
}

void step( Board * bd, Coord change ) {
    (*bd)._curr = change;
    (*bd)._grid[(*bd)._curr._y][(*bd)._curr._x] = 'k';
    ++( (*bd)._moves );
}

int tour( Board bd ) {
    int poss = 0, sol = 0, j = 0;

    /* Find and count next valid moves. */
    Coord changed[8];
    poss = findChanges( bd, changed );

    /* Initialize pipe. */
    int pipes[poss][2];
    for ( int i = 0; i < poss; ++i ) {
        int rc = pipe( pipes[i] );
        if ( rc < 0 ) {
            fprintf( stderr, "ERROR: pipe() %d failed\n", ( i + 1 ) );

            freeBoard( &bd );
            exit( EXIT_FAILURE );
        }
    }

    pid_t pids[poss];
    if ( poss >= 1 ) {
        if ( poss > 1 ) {
            printf( "PID %d: %d moves possible after move #%d\n", getpid(), 
                    poss, bd._moves );

#ifdef DISPLAY_BOARD
            printBoard( bd, getpid(), false );
#endif

            for ( j = 0; j < poss; ++j ) {
                if ( ( pids[j] = fork() ) < 0 ) {
                    fprintf( stderr, "ERROR: fork() failed\n" );

                    freeBoard( &bd );
                    exit( EXIT_FAILURE );
                } else if ( pids[j] == 0 ) {
                    step( &bd, changed[j] );
                    tour( bd );
                    break;
                }
#ifdef NO_PARALLEL
                wait( NULL );
#endif
            }

            if ( pids[j] > 0 ) {
                int moves[poss], i = poss;
                pid_t pid;
                while ( i > 0 ) {
                    wait( &pid );
                    if ( WIFEXITED( pid ) ) {
                        --i;
                        close( pipes[j][1] );       pipes[j][1] = -1;
                        int in = read( pipes[j][0], &( moves[poss] ), 
                                sizeof( int ) );
                        if ( in < 0 ) {
                            fprintf( stderr, "ERROR: read() failed\n" );

                            freeBoard( &bd );
                            exit( EXIT_FAILURE );
                        }
                        printf( "PID %d: Received %d from child\n", getpid(), 
                                moves[poss] );
                    } else {
                        freeBoard( &bd );
                        exit( EXIT_FAILURE );
                    }
                }

                sol = max( moves, poss );
            } 
        } else {
            /* poss == 1 :: don't fork */
            step( &bd, changed[0] );
            tour( bd );
        }
    } else {
        /* poss < 1 :: dead end */
        printf( "PID %d: Dead after move #%d\n", getpid(), bd._moves );

#ifdef DISPLAY_BOARD
        printBoard( bd, getpid(), false );
#endif

        close( pipes[j][0] );                   pipes[j][0] = -1;
        printf( "  %d\n", sol );
        int out = write( pipes[j][1], &( bd._moves ), sizeof( int ) );
        if ( out < 0 ) {
            fprintf( stderr, "ERROR: write() failed\n" );

            freeBoard( &bd );
            exit( EXIT_FAILURE );
        }
        printf( "PID %d: Sent %d on pipe to parent\n", getpid(), bd._moves );
    }

    return sol;
}

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
            printf( "_cols = %d, _rows = %d, _moves = %d\n", bd._cols, bd._rows,
                    bd._moves );
            printf( "_curr = (%d, %d)\n", bd._curr._x, bd._curr._y );
            printBoard( bd, 0, true );
            printf( "\n" );
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
