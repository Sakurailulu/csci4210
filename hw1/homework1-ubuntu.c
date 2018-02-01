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

const int SIZE = 32;                /* Incrementing factor for array. */
const int SIZEOF = sizeof( Word );  /* sizeof( Word ) to save function calls. */
// Word * store;                       /* Global pointer to Word struct array. */

/* Prints Word struct using desired output formatting.
 * @param   w, Word struct to be printed in the format
 *                  "w._word -- w._count"
 */
void printWord( Word w ) {
    printf( "%s -- %d\n", w._word, w._count );
}

void checkAlloc( Word ** store, int count ) {
    if ( ( count % 32 == 0 ) && ( count > 0 ) ) {
        int newAlloc = count + 32;
        Word * tmp = realloc( *store, ( newAlloc * SIZEOF ) );
        if ( tmp != NULL ) {
            *store = tmp;
            printf( "Re-allocated parallel arrays to be size %d.\n", newAlloc );
        } else {
            fprintf( stderr, "ERROR: memory reallocation failed.\n" );
        }
        free( tmp );
    }
}

int add( Word ** store, char w[80], int unique ) {
    checkAlloc( store, unique );

    bool exists = false;
    for ( int i = 0; i < unique; ++i ) {
        Word * tmp = ( *store + ( SIZEOF * i ) );
        if ( ( exists = ( strcmp( tmp->_word, w ) == 0 ) ) ) {
            ++( tmp->_count );
            break;
        }
        // free( tmp );
    }

    if ( !exists ) {
        Word tmp;
        strcpy( tmp._word, w );
        tmp._count = 1;

        *( *store + ( SIZEOF * unique ) ) = tmp;
        ++unique;
    }
    return unique;
}

void parseDir( Word ** store, DIR * dir, int * total, int * unique ) {
#ifdef DEBUG_MODE
    printf( "parsing...\n" );
#endif

    struct dirent * file;
    while ( ( file = readdir( dir ) ) != NULL ) {
        struct stat info;
        lstat( file->d_name, &info );

        FILE * f = fopen( file->d_name, "r" );
        if ( ( S_ISREG( info.st_mode ) ) && ( f != NULL ) ) {
#ifdef DEBUG_MODE
            printf( "working on %s...\n", file->d_name );
#endif

            char tmp[80];
            int i = 0;
            char c;
            while ( !feof( f ) ) {
                c = fgetc( f );
                if ( isalnum( c ) ) {
                    tmp[i] = c;
                    ++i;
                } else {
                    if ( ( isalnum( tmp[0] ) ) && ( isalnum( tmp[1] ) ) ) {
                        tmp[i] = '\0';
                        *unique = add( store, tmp, *unique );
                        ++( *total );

#ifdef DEBUG_MODE
                        printf( "%d --> added %s...\n", *total, tmp );
#endif
                    }
                    i = 0;
                    memset( tmp, 0, 80 );
                }
            }
        }
        (void)fclose( f );
    }
}

/* Prints first and last 'x' Word structs in store.
 * @param   num, the number of words to print from start and end.
 *          count, count of all unique words.
 */
void printSome( Word ** store, int num, int count ) {
    Word * words = *store;
    printf( "First %d words (and corresponding counts) are:\n", num );
    for ( int i = 0; i < num; ++i ) {
        printWord( *( words + ( SIZEOF * i ) ) );
    }

    printf( "Last %d words (and corresponding counts) are:\n", num );
    for ( int i = ( count - num ); i < count; ++i ) {
        printWord( *( words + ( SIZEOF * i ) ) );
    }
    // free( words );
}

/* Prints all Word structs in store.
 * @param   count, count of all unique words.
 */
void printAll( Word ** store, int count ) {
    Word * words = *store;
    printf( "All words (and corresponding counts) are:\n" );
    for ( int i = 0; i < count; ++i ) {
        printWord( *( words + ( SIZEOF * i ) ) );
    }
    // free( words );
}

int main( int argc, char * argv[] ) {
    /* Prevent buffering in stdout. */
    setbuf( stdout, NULL );

    if ( ( argc == 2 ) || ( argc == 3 ) ) {
#ifdef DEBUG_MODE
        printf( "started...\n" );
#endif

        DIR * dir = opendir( argv[1] );
        if ( dir != NULL ) {
            if ( chdir( argv[1] ) == 0 ) {
#ifdef DEBUG_MODE
                printf( "working...\n" );
#endif

                /* Initial memory allocation and assignment. */
                Word * store = malloc( SIZEOF * 32 );
                printf( "Allocated initial parallel arrays of size 32.\n" );

                int * total = malloc( sizeof( int ) ),
                    * unique = malloc( sizeof( int ) );

                *total = 0;
                *unique = 0;

                /* Parsing. */
                parseDir( &store, dir, total, unique );
                (void)closedir( dir );

                /* Printing conditionals to provide correct output. */
                if ( argc == 2 ) {
                    printAll( &store, *unique );
                } else {
                    char * tmp;
                    int toPrint = strtol( argv[2], &tmp, 10 );
                    if ( ( toPrint > *unique ) || ( *total < ( 2 * toPrint ) ) ) {
                        printAll( &store, *unique );
                    } else {
                        printSome( &store, toPrint, *unique );
                    }
                }

                /* Free allocated memory and return. */
                free( unique );         unique = NULL;
                free( total );          total = NULL;
                free( store );          store = NULL;
                return EXIT_SUCCESS;
            } else {
                /* chdir( argv[1] ) != 0 */
                fprintf( stderr, "ERROR: could not access directory.\n" );
                return EXIT_FAILURE;
            }
        } else {
            /* dir == NULL */
            fprintf( stderr, "ERROR: could not find directory.\n" );
            return EXIT_FAILURE;
        }
    } else {
        /* ( argc != 2 ) && ( argc != 3 ) */
        fprintf( stderr, "ERROR: invalid arguments.\n" );
        return EXIT_FAILURE;
    }
}
