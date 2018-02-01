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

typedef struct {
    char _word[80];
    int _count;
} Word;

const int SIZE = 32;            /* Starting size and incrementing factor for WORDS. */
Word * WORDS;                   /* Word struct array for storage. */


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/* Prints Word struct using desired output formatting.
 * @param   word, Word struct to be printed in the format
 *                "word._word -- word._count"
 */
void print( Word word ) {
    printf( "%s -- %d\n", word._word, word._count );
}

/* -------------------------------------------------------------------------- */

/* Prints all Word structs in WORDS.
 * @param   count, count of all unique words.
 */
void printAll( int count ) {
    printf( "All words (and corresponding counts) are:\n" );
    for ( int i = 0; i < count; ++i ) {
        Word w = *( WORDS + ( i * sizeof( Word ) ) );
        print( w );
    }
}

/* -------------------------------------------------------------------------- */

/* Adds new Word struct to WORDS.
 * @param       str, char array with word to add.
 *              count, count of all unique words.
 * @return      int with new count of unique
 *              ( same a passed if word already exists, otherwise incremented by 1 )
 * @modifies    WORDS
 * @effects     adds new Word struct of modifies existing when necessary.
 */
int add( char str[80], int count ) {
    bool exists = false;
    for ( int i = 0; i < count; ++i ) {
        Word temp = *( WORDS + ( i * sizeof( Word ) ) );
        if ( strcmp( temp._word, str ) == 0 ) {
            ++( ( WORDS + ( i * sizeof( Word ) ) )->_count );
            exists = true;
            break;
        }
    }

    if ( !exists ) {
        Word temp;
        strcpy( temp._word, str );
        temp._count = 1;

        *( WORDS + ( count * sizeof( Word ) ) ) = temp;
        ++count;
    }

    return count;
}

void checkAlloc( Word ** w, int count ) {
    if ( ( count % 32 == 0 ) && ( count > 0 ) ) {
        int newAlloc = count + 32;
        Word * tmp = realloc( *w, ( newAlloc * sizeof( Word ) ) );
        if ( tmp == NULL ) {
            fprintf( stderr, "ERROR: Memory reallocation failed." );
        } else {
            w = &tmp;
            printf( "Re-allocated parallel arrays to be size %d.\n", newAlloc );
        }
        //free( tmp );
    }
}

/* -------------------------------------------------------------------------- */

/* Parses all files in directory.
 * @param       dir, opened DIR * using directory from command line input.
 *              total, pointer to int with count of all words read.
 *              unique, pointer to int with count of all unique words.
 * @modifies    WORDS
 * @effects     adds and modifies Word structs when necessary.
 */
void parseFiles( DIR * dir, int * total, int * unique ) {
    struct dirent * file;
    while ( ( file = readdir( dir ) ) != NULL ) {
        struct stat info;
        lstat( file->d_name, &info );

        FILE * f = fopen( file->d_name, "r" );
        if ( S_ISREG( info.st_mode ) && ( f != NULL ) ) {
#ifdef DEBUG_MODE
            printf( "working...\n" );
#endif

            char temp[80];
            int i = 0;
            while ( !feof( f ) ) {
                char c = fgetc( f );
                if ( isalnum( c ) ) {
                    temp[i] = c;
                    ++i;
                } else {
                    if ( ( isalnum( temp[0] ) ) && ( isalnum( temp[1] ) ) ) {
                        temp[i] = '\0';
                        checkAlloc( &WORDS, *unique );
                        *unique = add( temp, *unique );
                        ++( *total );

#ifdef DEBUG_MODE
                        /* if ( ( *unique == 12 ) || ( *unique == 37 ) ) {
                            printf( "----------------\n0 > %s   10 > %s\n----------------\n",
                                ( *( WORDS + ( 0 * sizeof( Word ) ) ) )._word,
                                ( *( WORDS + ( 10 * sizeof( Word ) ) ) )._word );
                        } */
                        printf( "str = %s, count = %d\n", temp, *unique );
#endif

                    }
                    i = 0;
                    memset( temp, 0, 80 );
                }
            }
            (void)fclose( f );
        }
    }
    printf( "All done (successfully read %d words; %d unique words).\n",
            *total, *unique );
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
                printf( "good directory, continuing...\n" );
#endif

                // WORDS = malloc( sizeof( Word ) * SIZE );
                WORDS = calloc( SIZE, sizeof( Word ) );
                printf( "Allocated initial parallel arrays of size 32.\n" );

                int * total = calloc( 1, sizeof( int ) ),
                    * unique = calloc( 1, sizeof( int ) );
                parseFiles( dir, total, unique );
                (void)closedir( dir );

#ifdef DEBUG_MODE
                printf( "----------------\n3 > %s   10 > %s\n----------------\n",
                    ( *( WORDS + ( 3 * sizeof( Word ) ) ) )._word,
                    ( *( WORDS + ( 10 * sizeof( Word ) ) ) )._word );
#endif

                printAll( *unique );
                free( unique );
                free( total );
                // free( WORDS );
                return EXIT_SUCCESS;
            } else {
                fprintf( stderr, "ERROR: could not change to directory.\n" );
                return EXIT_FAILURE;
            }
        } else {
            fprintf( stderr, "ERROR: directory does not exist.\n" );
            return EXIT_FAILURE;
        }
    } else {
        fprintf( stderr, "ERROR: Invalid arguments.\n" );
        fprintf( stderr, "USAGE: %s <directory> [<word-count>]\n", argv[0] );
        return EXIT_FAILURE;
    }
}
