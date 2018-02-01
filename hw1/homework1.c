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


void print( struct Word word ) {
    printf( "%s -- %d\n", word._word, word._count );
}

/* ------------------------------------------------------------------------- */

void printAll( struct Word * words, int count ) {
    printf( "All words (and corresponding counts) are:\n" );
    for ( int i = 0; i < count; ++i ) {
        print( *( words + ( i * sizeof( struct Word ) ) ) );
    }
}

/* ------------------------------------------------------------------------- */

int add( char w[80], struct Word * words, int count, int unique ) {
    bool added = false;
    for ( int i = 0; i < unique; ++i ) {
        struct Word * temp = ( words + ( i * sizeof( struct Word ) ) );
#ifdef DEBUG_MODE
        printf( "%s ==> %s\n", temp->_word, w );
#endif
        if ( strcmp( temp->_word, w ) == 0 ) {
            ++temp->_count;
            *( words + ( i * sizeof( struct Word ) ) ) = *temp;
            added = true;
            break;
        }
    }

    if ( !added ) {
        struct Word temp;
        strcpy( temp._word, w );
        temp._count = 1;
        *( words + ( count * sizeof( struct Word ) ) ) = temp;
        ++unique;
    }

    return unique;
}

void myAlloc(struct Word ** w, int count) {
    struct Word *temp = realloc(*w, (count+32)*sizeof(struct Word));
    if (temp) { w=&temp; }
    printf( "Re-allocated parallel arrays to be size %d.\n", (32+count) );
}

/* ------------------------------------------------------------------------- */

int parseRegFiles( DIR * dir, struct Word * words ) {
#ifdef DEBUG_MODE
    printf( "reading...\n" );
#endif

    int count = 0, unique = 0, iter = 1;
    struct dirent * file;
    while ( ( file = readdir( dir ) ) != NULL ) {
        struct stat info;
        lstat( file->d_name, &info );

        FILE * f = fopen( file->d_name, "r" );
        if ( S_ISREG( info.st_mode ) && ( f != NULL ) ) {
#ifdef DEBUG_MODE
            printf( "opening %s...\n", file->d_name );
#endif

            char temp[80];
            int i = 0;
            do {
                char c = fgetc( f );
                if ( isalnum( c ) ) {
                    temp[i] = c;
                    ++i;
                } else {
                    if ( isalnum( temp[1] ) ) {
                        temp[i] = '\0';
                        unique = add( temp, words, count, unique );
                        ++count;
                        if ( unique % 32 == 0 ) {
                            ++iter;
                            myAlloc(&words, unique);
                        }
                    }

                    memset( temp, 0, 80 );
                    i = 0;
                }
            } while ( !feof( f ) );
            ( void )fclose( f );
        }
    }
    printf( "All done (successfully read %d words; %d unique words).\n",
            count, unique );
    return unique;
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
            if ( chdir( argv[1] ) == 0 ) {
#ifdef DEBUG_MODE
                printf( "regular files...\n" );
#endif

                struct Word * words = calloc( 32, sizeof( struct Word ) );
                printf( "Allocated initial parallel arrays of size 32.\n" );
                int uniqueCount = parseRegFiles( dir, words );
                ( void )closedir( dir );

                printAll( words, uniqueCount );
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
