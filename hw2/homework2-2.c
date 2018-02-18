#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int _x, _y;
} Coord;

typedef struct {
    int _rows, _cols;
    Coord  _pos;
    char ** _grid;
} Board;

#define COORD_SIZE sizeof( Coord )
#define BOARD_SIZE sizeof( Board )

void printBoard( pid_t pid, Board b );
int findPoss( Board b, Coord ** move );
Board step( Board b, Coord c );
int tour( Board * b );

const Coord all[8] = { (Coord){ ._x = +1, ._y = -2 },   /* up, then right */
                       (Coord){ ._x = +2, ._y = -1 },   /* right, then up */
                       (Coord){ ._x = +2, ._y = +1 },   /* right, then down */
                       (Coord){ ._x = +1, ._y = +2 },   /* down, then right */
                       (Coord){ ._x = -1, ._y = +2 },   /* down, then left */
                       (Coord){ ._x = -2, ._y = +1 },   /* left, then down */
                       (Coord){ ._x = -2, ._y = -1 },   /* left, then up */
                       (Coord){ ._x = -1, ._y = -2 } }; /* up, then left */

void printBoard( pid_t pid, Board b ) {
    for ( int i = 0; i < b._rows; ++i ) {
#ifndef DEBUG_MODE
        printf( "PID %d: ", pid );
#endif
        printf( "  %s\n", b._grid[i] );
    }
}

int findPoss( Board b, Coord ** move ) {
    int count = 0;
    for ( int i = 0; i < 8; ++i ) {
        int x = ( b._pos._x + all[i]._x ), y = ( b._pos._y + all[i]._y );
        if ( ( ( 0 <= y ) && ( y < b._rows ) ) && 
             ( ( 0 <= x ) && ( x < b._cols ) ) && ( b._grid[x][y] != 'k' ) ) {
            (*move)[count] = (Coord){ ._x = x, ._y = y };
            ++count;
        }
    }

    Coord * tmp = realloc( *move, ( count * COORD_SIZE ) );
    if ( tmp != NULL ) { move = &tmp; }
    return count;
}

Board step( Board b, Coord c ) {
    b._pos = c;
    b._grid[b._pos._y][b._pos._x] = 'k';
    return b;
}

int tour( Board * b ) {
    int count = 1, num = 0, max = 0, i = 0;
    Board tmp = *b;
    // pid_t par = getpid();

    Coord * move = calloc( 8, COORD_SIZE );
    num = findPoss( tmp, &move );

#ifdef DEBUG_MODE
    printf( "num = %d\n", num );
    for ( int i = 0; i < num; ++i ) {
        printf( "  move opt. %d --> (%d, %d)\n", ( i + 1 ), move[i]._x, 
                move[i]._y );
    }
#endif
    
    /* Prep for pipe(). */
    int p[num][2];

    if ( num >= 1 ) {
        if ( num > 1 ) {
            printf( "PID %d: Multiple moves possible after move #%d\n", 
                    getpid(), count );

#ifdef DISPLAY_BOARD
            printBoard( getpid(), tmp );
#endif

            /* Prep for fork(). */
            pid_t pids[num];
            for ( i = 0; i < num; ++i ) {
                /* Initialize current pipe. */
                int rc = pipe( p[i] );
                if ( rc < 0 ) {
                    fprintf( stderr, "ERROR: pipe() failed.\n" );
                    exit( EXIT_FAILURE );
                }

                /* Fork into children. */
                if ( ( pids[i] = fork() ) < 0 ) {
                    fprintf( stderr, "ERROR: fork() failed.\n" );
                    exit( EXIT_FAILURE );
                } else if ( pids[i] == 0 ) {    /* CHILD */
                    // tmp = step( tmp, move[i] );
                    tmp._pos = move[i];
                    tmp._grid[tmp._pos._y][tmp._pos._x] = 'k';
                    ++count;

                    tour( &tmp );

                    /* Free all memory allocated in children. */
                    free( move );               move = NULL;
                    for ( int j = 0; j < (*b)._rows; ++j ) {
                        free( (*b)._grid[j] );  (*b)._grid[j] = NULL;
                    }
                    free( (*b)._grid );         (*b)._grid = NULL;
                    exit( EXIT_SUCCESS );
                }
            }
        } else {
            /* num == 1 :: one move, don't fork */
            // tmp = step( tmp, move[0] );
            tmp._pos = move[0];
            tmp._grid[tmp._pos._y][tmp._pos._x] = 'k';
            ++count;

            tour( &tmp );
        }
    } else {
        /* num < 1 :: no moves possible */
        // if ( getpid() != par ) {                /* CHILD */
        //     printf( "PID %d: Dead end after move #%d\n", getpid(), count );
        //     exit( EXIT_SUCCESS );
        // }
        printf( "PID %d: Dead end after move #%d\n", getpid(), count );

#ifdef DISPLAY_BOARD
        printBoard( getpid(), tmp );
#endif
        
        close( p[i][0] );           p[i][0] = -1;
        int written = write( p[i][1], &count, sizeof( int ) );
        if ( written == -1 ) {
            fprintf( stderr, "ERROR: write() in child failed.\n" );
            exit( EXIT_FAILURE );
        } else {
            printf( "PID %d: Sending %d on pipe to parent pid %d\n", getpid(), 
                    count, getppid() );
            exit( EXIT_SUCCESS ); 
        }
    }

    free( move );
    b = &tmp;

    for ( int j = 0; j < num; ++j ) { max = ( max < p[j][1] ) ? p[j][1] : max; }
    return max;
}

int main( int argc, char * argv[] ) {
    setbuf( stdout, NULL );                     /* Stop buffering in stdout. */

    if ( argc == 3 ) {
        char * tmp;
        int m = strtol( argv[1], &tmp, 10 ), n = strtol( argv[2], &tmp, 10 );
        if ( ( m > 2 ) && ( n > 2 ) ) {
            /* Initializing Board struct to be used throughout. */
            /** Memory allocation. */
            Board init;
            init._rows = n;                     init._cols = m;
            init._grid = calloc( init._rows, sizeof( char* ) );
            if ( init._grid == NULL ) {
                fprintf( stderr, "ERROR: first calloc() failed.\n" );
                return EXIT_FAILURE;
            }
            for ( int i = 0; i < init._rows; ++i ) {
                init._grid[i] = calloc( init._cols, sizeof( char ) );
                if ( init._grid[i] == NULL ) {
                    fprintf( stderr, "ERROR: second calloc() failed.\n" );
                    return EXIT_FAILURE;
                }
            }

            /** Fill _grid with marker values. */
            init._pos = (Coord){ ._x = 0, ._y = 0 };
            for ( int i = 0; i < init._rows; ++i ) {
                for ( int j = 0; j < init._cols; ++j ) {
                    init._grid[i][j] = '.';
                }
            }
            init._grid[init._pos._y][init._pos._x] = 'k';

            /* Tour simulation. */
            printf( "PID %d: Solving the knight's tour problem for a %dx%d board.\n",
                    getpid(), init._cols, init._rows );

            int max = tour( &init );
            printf( "PID %d: Best solution found visits %d squares (out of %d)\n",
                    getpid(), max, ( init._rows * init._cols ) );

            /* Free allocated memory. */
            for ( int i = 0; i < init._rows; ++i ) {
                free( init._grid[i] );          init._grid[i] = NULL;
            }
            free( init._grid );                 init._grid = NULL;
            return EXIT_SUCCESS;
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
