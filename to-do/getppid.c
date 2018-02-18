/* getppid.c */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {

    pid_t pid = fork();

    if ( pid < 0 ) {
        fprintf( stderr, "ERROR: fork() failed.\n" );
    } else if ( pid == 0 ) {
        printf( "I am PID %d, and my parent is PID %d... I'm the child.\n",
                getpid(), getppid() );
        exit( 0 );
    } else {
        int status;
        wait( &status );

        printf( "I am PID %d, and my parent is PID %d... I'm the parent.\n",
                getpid(), getppid() );
    }

    return EXIT_SUCCESS;

}
