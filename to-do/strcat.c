/* strcat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    char word[10];
    char lets[9] = "abcdefghi";

    strcat( word, lets );
    printf( "%s", word );

    
    return EXIT_SUCCESS;
}
