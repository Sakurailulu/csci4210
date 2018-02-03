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

const int SCALE = 32;               /* Starting size and incrementing factor for STORE. */
const int SIZEOF = sizeof( Word );  /* Size of struct Word to save function calls. */
Word * STORE;                       /* Word struct array for storage. */


/* -------------------------------------------------------------------------- */
/* Printing functions. */

/* Prints Word struct using desired output formatting.
 * @param       word, Word struct to be printed in the format
 *                      "word._word -- word._count"
 */
void printWord( Word word ) {
    printf( "%s -- %d\n", word._word, word._count );
}


/* Prints passed number of first and last words.
 * @param       num, number of first and last to print.
 *              count, count of all unique words.
 */
void printSome( int num, int size ) {
    printf( "First %d words (and corresponding counts) are:\n", num );
    for ( int i = 0; i < num; ++i ) {
        printWord( STORE[i] );
    }

    printf( "Last %d words (and corresponding counts) are:\n", num );
    for ( int i = ( size - num ); i < size; ++i ) {
        printWord( STORE[i] );
    }
}


/* Prints all Word structs in STORE.
 * @param       size, count of all unique words.
 */
void printAll( int size ) {
    printf( "All words (and corresponding counts) are:\n" );
    for ( int i = 0; i < size; ++i ) {
        printWord( STORE[i] );
    }
}


/* -------------------------------------------------------------------------- */
/* Struct array modifiers. */

/* Reallocation method.
 * @param       w, struct array to allocate.
 *              size, count of all unique words.
 * @return      Word struct array equal to original or post-reallocation.
 */
Word * checkAlloc( Word * w, int size ) {
    if ( ( size % 32 == 0 ) && ( size > 0 ) ) {
        int newAlloc = size + 32;
        Word * tmp = realloc( w, ( newAlloc * SIZEOF ) );
        if ( tmp == NULL ) {
            fprintf( stderr, "ERROR: Memory reallocation failed." );
        } else {
            for ( int i = size; i < newAlloc; ++i ) {
                memset( ( tmp[i] )._word, 0, 80 );
                ( tmp[i] )._count = 0;
            }
            printf( "Re-allocated parallel arrays to be size %d.\n", newAlloc );
        }
        return tmp;
    } else {
        return STORE;
    }
}


/* Adds new Word struct to STORE.
 * @param       str, char array with word to add.
 *              size, count of all unique words.
 * @return      int with new count of unique
 *              ( same a passed if word already exists, otherwise incremented by 1 )
 * @modifies    STORE
 * @effects     adds new Word struct of modifies existing when necessary.
 */
int add( char str[80], int size ) {
    bool exists = false;
    for ( int i = 0; i < size; ++i ) {
        if ( ( exists = ( strcmp( ( STORE[i]._word ), str ) == 0 ) ) ) {
            ++( ( STORE[i] )._count );
            break;
        }
    }

    if ( !exists ) {
        STORE = checkAlloc( STORE, size );
        strcpy( ( STORE[size] )._word, str );
        ( STORE[size] )._count = 1;

        ++size;
    }

    return size;
}


/* -------------------------------------------------------------------------- */
/* File parsing. */

/* Parses all files in directory.
 * @param       dir, opened DIR * using directory from command line input.
 *              total, pointer to int with count of all words read.
 *              unique, pointer to int with count of all unique words.
 * @modifies    STORE
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

            char tmp[80];
            int i = 0, c = 0;
            while ( ( c = fgetc( f ) ) != EOF ) {
                if ( isalnum( c ) ) {
                    tmp[i] = c;
                    ++i;
                } else {
                    if ( ( isalnum( tmp[0] ) ) && ( isalnum( tmp[1] ) ) ) {
                        tmp[i] = '\0';
                        *unique = add( tmp, *unique );
                        ++( *total );

#ifdef DEBUG_MODE
                        printf( "str = %s, count = %d\n", tmp, *unique );
#endif

                    }
                    i = 0;
                    memset( tmp, 0, 80 );
                }
            }
        }
        (void)fclose( f );
    }
    printf( "All done (successfully read %d words; %d unique words).\n",
            *total, *unique );
}


/* ------------------------------------------------------------------------- */
/* Main */

int main( int argc, char * argv[] ) {
    if ( ( argc == 2 ) || ( argc == 3 ) ) {
        setbuf( stdout, NULL );     /* Prevent stdout buffering. */

#ifdef DEBUG_MODE
        printf( "started...\n" );
#endif

        DIR * dir = opendir( argv[1] );
        if ( dir != NULL ) {
            if ( chdir( argv[1] ) == 0 ) {
#ifdef DEBUG_MODE
                printf( "good directory, continuing...\n" );
#endif

                /* Initial memory allocation. */
                // STORE = malloc( SIZEOF * SCALE );
                STORE = calloc( SCALE, SIZEOF );
                printf( "Allocated initial parallel arrays of size 32.\n" );

                int * total = malloc( sizeof( int ) ),
                    * unique = malloc( sizeof( int ) );
                *total = 0;
                *unique = 0;

                /* Parsing. */
                parseFiles( dir, total, unique );
                (void)closedir( dir );

#ifdef DEBUG_MODE
                printf( "----------------\n3 > %s   10 > %s\n----------------\n",
                    ( *( STORE + ( 3 * SIZEOF ) ) )._word,
                    ( *( STORE + ( 10 * SIZEOF ) ) )._word );
#endif

                    /* Printing conditionals to provide correct output. */
                if ( argc == 2 ) {
                    printAll( *unique );
                } else {
                    char * tmp;
                    int toPrint = strtol( argv[2], &tmp, 10 );
                    if ( ( toPrint >= *unique ) || ( *total < ( 2 * toPrint ) ) ) {
                        printAll( *unique );
                    } else {
                        printSome( toPrint, *unique );
                    }
                }


                /* Free memory and return. */
                free( unique );     unique = NULL;
                free( total );      total = NULL;
                free( STORE );      STORE = NULL;
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

    /* We should never be able to ge here... */
    return EXIT_FAILURE;
}
