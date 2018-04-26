#!/usr/bin/python3

"""
Griffin Melnick, melnig@rpi.edu
Peter Straub, straup@rpi.edu

    Program info...

        bash$ python3 project2.py ...

    Options...
"""

from collections import defaultdict as ddict
from collections import deque
from itertools import groupby
import copy
import sys

FRAMES = 256
FREE = '.'
PER_LINE = 32
T_MEMMOVE = 1

''' Block helper class. ---------------------------------------------------- '''

class Block:

    ''' Constructors. ------------------------------------------------------ '''

    """
    Default constructor.
    :param:     tag, identifier (True or False) for if segment contains process
                mem, process in Block or FREE
    """
    def __init__( self, tag=False, mem=(FREE*FRAMES) ):
        self._tag = tag
        self._mem = mem


    ''' Overidden methods. ------------------------------------------------- '''

    """
    Repr representation of Block.
    :return:    string with all variable shown as respective repr outputs
    """
    def __repr__( self ):
        return "Block(_tag = {0}, _mem = {1})".format( repr(self._tag),
                repr(self._mem) )


    """
    String representation of Block.
    :return:    _mem
    """
    def __str__( self ):
        return self._mem


''' Process helper class. -------------------------------------------------- '''

class Process:

    ''' Constructors. ------------------------------------------------------ '''

    """
    Default contructor.
    :param:     pid, process id (single letter A-Z)
                size, memory space taken
                times, OrderDict of arrival times mapped to run times
    """
    def __init__( self, pid, size, times ):
        self._pid = copy.deepcopy( pid )
        self._size = copy.deepcopy( size )
        self._times = copy.deepcopy( times )


    ''' Overriden methods. ------------------------------------------------- '''

    """
    Repr representation of Process.
    :return:    string with all variables shown as respective repr outputs
    """
    def __repr__( self ):
        return "Process(_pid = {0}, _size = {1}, _times = {2})".format(
                repr(self._pid), repr(self._size), repr(self._times) )


    """
    String representation of Process.
    :return:    string with Process _pid, _size times
    """
    def __str__( self ):
        return "{0}".format( str(self._pid) * self._size )


''' Simluator helper class. ------------------------------------------------ '''

class Simulator:

    ''' Constructors. ------------------------------------------------------ '''

    """
    Default constructor.
    :param:     procs, list of Processes pulled from input file.
    """
    def __init__( self, procs ):
        self._procs = copy.deepcopy( procs )
        self._memory = [ Block() ]
        self._running = ddict( int )
        self._tick = 0


    ''' Class methods. ----------------------------------------------------- '''

    """
    Helper method to added new Processes for contiguous simulations.
    :param:     proc, Process to add
                next, boolean if running c_next()
                best, boolean if running c_best()
                worst, boolean if running c_worst()
    :return:    True if added, False otherwise
    """
    def add( self, proc, next=False, best=False, worst=False ):
        if next:
            added = False
            for block in [ b for b in self._memory if ( (not b._tag) and
                    (len(b._mem) >= proc[0]._size) ) ]:
                i = (self._memory).index( block )
                tmp = len(block._mem) - proc[0]._size

                ''' Changed stored memory. '''
                self._memory[i] = Block( True, (proc[0]._pid * proc[0]._size) )
                (self._memory).insert( (i + 1), Block( False, (FREE * tmp) ) )

                ''' Manage in _running. '''
                self._running[ proc[0] ] = proc[1][1]

                added = True
                print( "time {}ms: Placed process {}:\n{}".format(self._tick,
                        proc[0]._pid, self) )
                break

            return added


    """
    Helper method to check new Processes for arrival.
    :return:    sorted list with tuples of new Processes and respective times
    """
    def arrived( self ):
        tmp = []
        for proc in [ p for p in self._procs if ( (p._times) and (p._times[0][0]
                == self._tick) ) ]:
            tmp.append( ( proc, (proc._times).popleft() ) )

        return sorted( tmp, key = lambda obj : obj[0]._pid )


    """
    Helper method for defragmenting.
    :modifies:  _memory
    :effects:   moves all FREE memory to end of list
    """
    def defrag( self ):
        self._memory = list( [ b for b in self._memory if (b._tag) ] + [ b for
                b in self._memory if (not b._tag) ] )

        ''' maybe add check for process arrival '''
        tmp = sum( len( b._mem for b in self._memory if (b._tag) ) )
        self._tick += tmp * T_MEMMOVE
        print( "time {}ms: Defragmentation complete (moved {} frames: "
                "{})".format( self._tick, tmp, "".join(b._mem[0] for b in
                self._memory if (b._tag)) ) )


    """
    Helper method to remove finished processes.
    :modifies:  _memory, _running
    :effects:   (to _memory) removes respective Block
                (to _running) deletes Process
    """
    def remove( self ):
        for proc in [ p for p, r in (self._running).items() if (r == 0) ]:
            ''' Delete process block from memory. '''
            for block in [ b for b in self._memory if (b._mem == str(proc)) ]:
                self._memory[ (self._memory).index(block) ] = Block( False,
                        (FREE * proc._size) )

            print( "time {}ms: Process {} removed:\n{}".format(self._tick,
                    proc._pid, self) )

            ''' Remove from running. '''
            del self._running[proc]


    ''' Overridden methods. ------------------------------------------------ '''

    """
    Repr representation of Simulator.
    :return:    string with all variables shown with respective repr outputs
    """
    def __repr__( self ):
        return "Simulator(_procs = {0}, _memory = {1}, _tick = {2})".format(
                repr(self._procs), repr(self._memory), repr(self._tick) )


    """
    String representation of Simulator.
    :return:    string representation of memory space as a frame
    """
    def __str__( self ):
        return "{0}\n{1}\n{0}".format( ('=' * PER_LINE), "\n".join( "".join( l
                for l in ("".join( b._mem for b in self._memory ))[(PER_LINE *
                i) : PER_LINE * (i+1)] ) for i in range(FRAMES // PER_LINE) ) )


''' Methods. --------------------------------------------------------------- '''

"""
Contiguous, next-fit simulation.
:param:     procs, list of Processes to simulate
"""
def c_next( procs ):
    sim = Simulator( copy.deepcopy(procs) )
    print( "time {}ms: Simulator started (Contiguous -- Next-fit)".format(
            sim._tick) )

    while 1:
        ''' Remove done processes. '''
        sim.remove()

        ''' Check for simulation completion. '''
        arrived = sim.arrived()
        if ( (not arrived) and (not sim._running) ):
            break

        ''' Added new processes. '''
        for proc in arrived:
            print( "time {}ms: Process {} arrived (requires {} frames)".format(
                    sim._tick, proc[0]._pid, proc[0]._size) )

            added = sim.add( proc, True, False, False )
            if ( added ):
                continue

            elif ( (not added) and ( sum( len(b._mem) for b in sim._memory if
                    (not b._tag) ) >= proc[0]._size ) ):
                sim.defrag()
                added = sim.add( proc, True, False, False )

                if ( added ):
                    continue

                else:
                    ''' not added : skip process '''
                    print ( "time {}ms: Cannot place process {} -- "
                            "skipped!".format(sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        for proc, rem in (sim._running).items():
            rem -= 1

        sim._tick += 1

    print( "time {}ms: Simulator ended (Contiguous -- Next-fit)\n".format(
            sim._tick) )


"""
Contiguous, best-fit simulation.
:param:     procs, list of Processes to simulate
"""
def c_best( procs ):
    sim = Simulator( copy.deepcopy(procs) )
    print( "time {}ms: Simulator started (Contiguous -- Best-fit)".format(
            sim._tick) )

    ''' simulation code '''

    print( "time {}ms: Simulator ended (Contiguous -- Best-fit)\n".format(
            sim._tick) )


"""
Contiguous, worst-fit simulation.
:param:     procs, list of Processes to simulate
"""
def c_worst( procs ):
    sim = Simulator( copy.deepcopy(procs) )
    print( "time {}ms: Simulator started (Contiguous -- Worst-fit)".format(
            sim._tick) )

    ''' simulation code '''

    print( "time {}ms: Simulator ended (Contiguous -- Worst-fit)\n".format(
            sim._tick) )


"""
Non-contiguous simulation.
:param:     procs, list of Processes to simulate
"""
def non_c( procs ):
    sim = Simulator( copy.deepcopy(procs) )
    print( "time {}ms: Simulator started (Non-contiguous)".format(sim._tick) )

    ''' simulation code '''

    print( "time {}ms: Simulator ended (Non-contiguous)".format(sim._tick) )


''' Main. ------------------------------------------------------------------ '''

if ( __name__ == "__main__" ):
    if ( len(sys.argv) == 2 ):
        ''' Check for valid input file. '''
        try:
            f = open( sys.argv[1], 'r' )
        except:
            sys.exit( "ERROR: Could not open input file" )

        ''' Read in all processes. '''
        procs = []
        for l in [m for m in f.readlines() if ((m[0] != '#') and (m != '\n'))]:
            l = (l.strip()).split()
            try:
                tmp = Process( l[0], int(l[1]), deque([ ( int(t.split('/')[0]),
                        int(t.split('/')[1]) ) for t in l[2:] ]) )
                procs.append( tmp )
            except:
                sys.exit( "ERROR: Invalid input file format" )

        f.close()

        ''' Run individual simluations. '''
        c_next( procs )
        c_best( procs )
        c_worst( procs )
        non_c( procs )

    else:
        ''' len(sys.argv) != 2 '''
        sys.exit( "ERROR: Invalid argument(s)\nUSAGE: ./a.out <input-file>" )

