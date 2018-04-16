/* pipe.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define MSG_SIZE 16

int main() {
    int p[2];
    pipe( p );

    pid_t pid = fork();
    if ( pid == 0 ) {
        close( p[0] );                          p[0] = -1;
        for ( int i = 0; i < 5; ++i ) {
            int written = write( p[1], &i, 4 );
            if ( written > 0 ) {
                printf( "CHILD: Wrote %d to pipe\n", i );
                fflush( stdout );
            }
        }
    } else {
        wait( NULL );
        close( p[1] );                          p[1] = -1;
        char buf[80];
        int readIn = read( p[0], buf, 5 );
        if ( readIn > 0 ) {
            buf[5] = '\0';
            printf( "PARENT: Read %s from pipe\n", buf );
            fflush( stdout );
        }
    }

    return EXIT_SUCCESS;
}
