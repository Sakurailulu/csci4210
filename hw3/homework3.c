/* homework3.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Program info
 *
 *   bash$ a.out <m> <n> [<k>]
 *
 * where: <m> is the width, number of columns, of the board; <n> is the height, 
 * number of rows, of the board; and the optional <k> is the fewest number of 
 * squares allowed on boards printed out.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int _x, _y;
} Coord;

typedef struct {
    int _cols, _rows, _moves;
    Coord _curr;
    char ** _grid;
} Board;

/* Coord array holding all possible steps. */
const Coord all[8] = { (Coord){ ._x = +1, ._y = -2 },   /* up, then right */
                       (Coord){ ._x = +2, ._y = -1 },   /* right, then up */
                       (Coord){ ._x = +2, ._y = +1 },   /* right, then down */
                       (Coord){ ._x = +1, ._y = +2 },   /* down, then right */
                       (Coord){ ._x = -1, ._y = +2 },   /* down, then left */
                       (Coord){ ._x = -2, ._y = +1 },   /* left, then down */
                       (Coord){ ._x = -2, ._y = -1 },   /* left, then up */
                       (Coord){ ._x = -1, ._y = -2 } }; /* up, then left */

/* -------------------------------------------------------------------------- */

void printBoard( Board bd );
int tour( Board bd );

/* -------------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {

}

