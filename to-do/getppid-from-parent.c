/* getppid-from-parent.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    printf( "PID: %d\n", getpid() );
    printf( "Parent: %d\n", getppid() );
    return EXIT_SUCCESS;

}

