/* homework1.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Emulating a file directory and text manipulation program, we parse 
 * through all regular files in a given directory to find the words 
 * versus unique words using one of the following command line calls
 *
 *   bash$ ./a.out <directory>
 *
 *   OR
 *
 *   bash$ ./a.out <directory> <word-count>
 *
 * The first call parses through the directory and gives all unique 
 * words with a count of ocurrences, whereas the second call only gives
 * the first and last *number* unique words with a count of ocurrences.
 */

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct Word {
    char _word[80];
    int _count;
};


/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */


/* bool hasRegFiles( DIR * dir ) {
#ifdef DEBUG_MODE
    printf( "checking for regular files...\n" );
#endif

    bool hasReg = false;
    struct dirent * file;
    while ( ( file = readdir( dir ) ) != NULL ) {
        struct stat info;
        lstat( file->d_name, &info );

        if ( info.st_mode == 33188 ) {
            hasReg = true;
        }
    }

    free( file );
    return hasReg;
} */

void parseRegFiles( DIR * dir, struct Word * words ) {
#ifdef DEBUG_MODE
    printf( "reading...\n" );
#endif

    int wordCount = 0, uniqueCount = 0;
    struct dirent * file;
    int curr = 1, iter = 1;
    while ( ( file = readdir( dir ) ) != NULL ) {
        struct stat info;
        lstat( file->d_name, &info );

        if ( S_ISREG( info.st_mode ) ) {
            FILE * f = fopen( file->d_name, "r" );
            if ( f != NULL ) {
#ifdef DEBUG_MODE
                printf( "opening %s...\n", file->d_name );
#endif

                char temp[80];
                int i = 0;
                do {
                    char c = fgetc( f );
                    if ( feof( f ) ) {
#ifdef DEBUG_MODE
                        printf( "here, now\n" );
#endif
                        break;
                    } else {
                        if ( isalnum( c ) ) {
                            temp[i] = c;
                            ++i;
                        } else if ( isalnum( temp[1] ) ) {
                            temp[i] = '\0';
                            printf( "%s\n", temp );
                            memset( temp, 0, 80 );
                            i = 0;
                        }
                        // printf( "%c", c );
                    }
                } while ( true );

                ( void )fclose( f );
            }
        }

        if ( curr % 32 == 0 ) {
            ++iter;
            words = realloc( words, ( 32 * iter ) * sizeof( struct Word ) );
            printf( "Re-allocated parallel arrays to be size %d.\n", (32 * iter) );
        }

        ++curr;
    }
    printf( "All done (successfully read %d words; %d unique words).\n", 
            wordCount, uniqueCount );
}

void printAll( struct Word * words ) {
    printf( "All words (and corresponding counts) are:\n" );
}

void print( struct Word word ) {
    printf( "%s -- %d\n", word._word, word._count );
}


/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */


int main( int argc, char * argv[] ) {
    setbuf( stdout, NULL );     /* Prevent stdout buffering. */
    if ( ( argc == 2 ) || ( argc == 3 ) ) {
#ifdef DEBUG_MODE
        printf( "started...\n" );
#endif

        DIR * dir = opendir( argv[1] );
        if ( dir != NULL ) {
            // int changeDir = chdir( argv[1] );
            // if ( chdir( argv[1] ) == 0 ) {
            if ( true ) {
#ifdef DEBUG_MODE
                printf( "regular files...\n" );
#endif

                struct Word * words = calloc( 32, sizeof( struct Word ) );
                printf( "Allocated initial parallel arrays of size 32.\n" );
                parseRegFiles( dir, words );
                ( void )closedir( dir );
            } else {
                fprintf( stderr, "ERROR: could not change to directory.\n" );
            }
        } else {
            fprintf( stderr, "ERROR: directory does not exist.\n" );
        }
    } else {
        fprintf( stderr, "ERROR: Invalid arguments.\n" );
        fprintf( stderr, "USAGE: %s <directory> [<word-count>]\n", argv[0] );
        return EXIT_FAILURE;
    }
}

