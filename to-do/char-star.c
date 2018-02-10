/* char-star.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    int x = 3, y = 4;
    char ** grid = calloc( x, sizeof( char* ) );
    for ( int i = 0; i < x; ++i ) {
        grid[i] = calloc( y, sizeof( char ) );
    }

    int tmp = 0;
    for ( int i = 0; i < x; ++i ) {
        for ( int j = 0; j < y; ++j, ++tmp ) {
            grid[i][j] = tmp;
        }
    }

    for ( int i = 0; i < x; ++i ) {
        for ( int j = 0; j < y; ++j ) {
            printf( "grid[%d][%d] = %d\n", i, j, grid[i][j] );
        }
    }

    return EXIT_SUCCESS;

}

