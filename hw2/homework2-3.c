/* homework2.c
 * Griffin Melnick, melnig@rpi.edu
 *
 * Info
 *
 *   bash$ ./a.out <m> <n>
 *
 * where both 'm' and 'n' are greater than two. Otherwise, an error is shown 
 * and the programs fails.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/* Helper structs */
/** (x, y) pairing to simplify moves. */
typedef struct {
    int _x, _y;
} Pair;

/** Board properties to simplify parameters. */
typedef struct {
    int _rows, _cols, _moves;
    Pair _curr;
    char ** _grid;
} Board;

#define PAIR_SIZE sizeof( Pair )
#define BOARD_SIZE sizeof( Board )


const Pair all[8] = { (Pair){ ._x = +1, ._y = -2 },     /* up, then right */
                      (Pair){ ._x = +2, ._y = -1 },     /* right, then up */
                      (Pair){ ._x = +2, ._y = +1 },     /* right, then down */
                      (Pair){ ._x = +1, ._y = +2 },     /* down, then right */
                      (Pair){ ._x = -1, ._y = +2 },     /* down, then left */
                      (Pair){ ._x = -2, ._y = +1 },     /* left, then down */
                      (Pair){ ._x = -2, ._y = -1 },     /* left, then up */
                      (Pair){ ._x = -1, ._y = -2 } };   /* up, then left */


int max( int l, int r );
void printBoard( pid_t pid, Board b );
int findPoss( Pair * moves, Board b );
void step( Pair move, Board * b );
void tour( Board * b );


/* -------------------------------------------------------------------------- */

/* Maximum helper method.
 * @param       l, int to compare.
 *              r, int to compare.
 * @return      the maximum value chosen from the two passed values.
 */
int max( int l, int r ) {
    return ( ( l > r ) ? l : r );
}


/* Board printing helper method.
 * @param       pid, pid value passed for DISPLAY_BOARD output.
 *              b, Board to print.
 */
void printBoard( pid_t pid, Board b ) {
    for ( int i = 0; i < b._rows; ++i ) {
#ifndef DEBUG_MODE
        printf( "PID %d: ", pid );
#endif
        printf( "  %s\n", b._grid[i] );
    }
}


/* Board parsing helper method.
 * @param       moves, Pair array to store valid moves.
 *              b, Board to parse for moves.
 * @return      count of found valid moves.
 * @modifies    moves
 * @effects     adds valid moves.
 */
int findPoss( Pair * moves, Board b ) {
    int poss = 0;
    for ( int i = 0; i < 8; ++i ) {
        Pair move = (Pair){ ._x = ( b._curr._x + all[i]._x ),
                            ._y = ( b._curr._y + all[i]._y ) };
        if ( ( ( 0 <= move._y ) && ( move._y < b._rows ) ) && 
                ( ( 0 <= move._x ) && ( move._x < b._cols ) ) && 
                ( b._grid[move._y][move._x] != 'k' ) ) {
            moves[poss] = move;
            ++poss;

#ifdef DEBUG_MODE
            printf( "  possible move %d -> (%d, %d)", poss, move._x, move._y );
#endif
        }
    }

    return poss;
}


/* Single step helper method.
 * @param       move, new spot in grid to which to move.
 *              b, Board through which to step.
 * @modifies    b
 * @effects     adds new "k" to position 'move', increments moves taken.
 */
void step( Pair move, Board * b ) {
    (*b)._curr = move;
    (*b)._grid[ (*b)._curr._y ][ (*b)._curr._x ] = 'k';
    ++( (*b)._moves );
}


/* Touring method.
 * @param       b, Board on which to perform knight's tour.
 * @modifies    b
 * @effects     adds effects of tour to board.
 */
void tour( Board * b ) {
    
}
