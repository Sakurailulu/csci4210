/* homework1.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Emulating a file directory and text manipulation program, we parse 
 * through all usable files in a given directory to find the words versus
 * unique words using one of the following command line calls
 *
 *   bash$ *executable* *directory*
 *
 *   OR
 *
 *   bash$ *executable* *directory* *number*
 *
 * The first call parses through the directory and gives all unique 
 * words with a count of ocurrences, whereas the second call only gives
 * the first and last *number* unique words with a count of ocurrences.
 */

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct Word {
    char * _word;
    int _count;
};

bool findFiles( char * dirName, char * files[] ) {
    DIR * dir = opendir( dirName );
    if ( dir != NULL ) {

#ifdef DEBUG_MODE
        printf( "good directory\n" );
        fflush( stdout );
#endif

        struct dirent * file;
        struct lstat info;
        while ( ( file = readdir( dir ) ) != NULL ) {
            lstat( file->d_name, &info );
            printf( "File: %s.", file->d_name );
            printf( "File mode: %d.\n", info->st_mode );
            fflush( stdout );
        }
        free( file );
        return true;

    } else {

#ifdef DEBUG_MODE
        printf( "bad directory\n" );
        fflush( stdout );
#endif
        perror( "ERROR" );
        return false;

    }
}

int main( int argc, char * argv[] ) {
    if ( ( argc == 2 ) || ( argc == 3 ) ) {

#ifdef DEBUG_MODE
        printf( "started...\n" );
        fflush( stdout );
#endif

        char ** files = calloc( 0, sizeof( char* ) );
        bool hasFiles = findFiles( argv[1], files );

        if ( hasFiles ) {
            return EXIT_SUCCESS;
        } else {
            return EXIT_FAILURE;
        }

    } else {

#ifdef DEBUG_MODE
        printf( "failed.\n" );
        fflush( stdout );
#endif

        return EXIT_FAILURE;

    }
}

