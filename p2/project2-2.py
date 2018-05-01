#!/usr/bin/python3

"""
Griffin Melnick, melnig@rpi.edu
Peter Straub, straup@rpi.edu

Memory management simulation using next-fit, best-fit, and worst-fit contiguous
algorithms and a non-contiguous algorithm. Run by calling

    bash$ python3 project2.py <input-file>

                    OR

    bash$ ./project2.py <input-file>    (if Python3 path matches shebang at top)

where <input-file> is a text file with properly formatted process details.
"""

from collections import defaultdict as ddict
from collections import deque
import copy
import itertools
import sys

FRAMES = 256
FREE = '.'
PER_LINE = 32
T_MEMMOVE = 1


''' Wrapper class for process management. ---------------------------------- '''

class Process:

    ''' Constructors. ------------------------------------------------------ '''

    """
    Default constructor.
    :param:     pid, Process ID value
                size, amount of space taken in memory
                time, deque of arrival and run times
    """
    def __init__( self, pid, size, times ):
        self._pid = copy.deepcopy( pid )
        self._size = copy.deepcopy( size )
        self._times = copy.deepcopy( times )


    ''' Accessors. --------------------------------------------------------- '''

    """
    Accessor to find current ticker values.
    :return:    _pid
    """
    def get_pid( self ):
        return self._pid


    """
    Accessor to find current ticker values.
    :return:    _size
    """
    def get_size( self ):
        return self._size


    """
    Accessor to find current ticker values.
    :return:    _times
    """
    def get_tick( self ):
        return self._times


    ''' Overidden methods. ------------------------------------------------- '''

    """
    Repr representation.
    :return:    string with repr representation of all members
    """
    def __repr__( self ):
        return "Process(_pid={0}, _size={1}, _times={2})".format(
                repr(self._pid), repr(self._size), repr(self._times) )


    """
    String representation.
    :return:    string of _pid, _size times
    """
    def __str__( self ):
        return "{0}".format(self._pid * self._size)


''' Wrapper class for simluation. ------------------------------------------ '''

class Simulator:

    ''' Constructors. ------------------------------------------------------ '''

    """
    Default constructor.
    :param:     procs, deep-copied list of Processes in simulator
    """
    def __init__( self, procs ):
        self._procs = copy.deepcopy( procs )

        ''' Non-variable initializing values. '''
        self._last = 0
        self._mem = [ FREE for i in range(FRAMES) ]
        self._remain = ddict( int )
        self._tick = 0


    ''' Accessors. --------------------------------------------------------- '''

    """
    Accessor to find arriving Processes.
    :return:    list of arrived Processes sorted by _pid
    """
    def arrived( self ):
        tmp = []
        for proc in [ p for p in self._procs if ( (p._times) and (p._times[0][0]
                == self._tick) ) ]:
            tmp.append( ( proc, (proc._times).popleft()[1] ) )

        return sorted( tmp, key = lambda obj : obj[0]._pid )


    """
    Accessor to find current ticker values.
    :return:    _procs
    """
    def get_procs( self ):
        return self._procs


    """
    Accessor to find current ticker values.
    :return:    _last
    """
    def get_last( self ):
        return self._last


    """
    Accessor to find current ticker values.
    :return:    _mem
    """
    def get_mem( self ):
        return self._mem


    """
    Accessor to find current ticker values.
    :return:    _remain
    """
    def get_remain( self ):
        return self._remain


    """
    Accessor to find current ticker values.
    :return:    _tick
    """
    def get_tick( self ):
        return self._tick


    ''' Modifiers. --------------------------------------------------------- '''

    """
    Modifier to contiguously add new Process.
    :modifies:  _mem, _remain, _last
    :effects:   (to _mem) adds Process according to simluation
                (to _remain) adds Process mapped to full run time
                (to _last) sets to just past the end of Process
    """
    def c_add( self, proc, next=False, best=False, worst=False ):
        if ( next ):
            tmp = [ "".join(g) for k, g in itertools.groupby( "".join(l for l in
                    self._mem[self._last : FRAMES]) ) ] + [ "".join(g) for k, g
                    in itertools.groupby( "".join(l for l in self._mem[0 :
                    self._last]) ) ]

        else:
            tmp = [ "".join(g) for k, g in itertools.groupby( "".join(l for l in
                    self._mem) ) ]

        added = False
        for block in sorted( [ list(b) for b in tmp if ( (FREE in b) and (len(b) >=
                proc[0]._size) ) ], key=lambda l : (len(l) if (not next)
                else (not len(l)) ), reverse=worst ):
            i = tmp.index( "".join(l for l in block) )
            block[0 : proc[0]._size] = list( str(proc[0]) )
            tmp[i] = "".join(l for l in block)

            self._mem = []
            for b in tmp:
                for l in b:
                    (self._mem).append( l )
            if ( next ):
                self._mem = self._mem[(FRAMES - self._last) : FRAMES] + \
                        self._mem[0 : (FRAMES - self._last)]

            self._remain[ proc[0] ] = proc[1]
            self._last = max([ i for i, l in enumerate(self._mem) if (l ==
                    proc[0]._pid) ]) + 1

            print( "time {}ms: Placed process {}:\n{}".format(self._tick,
                    proc[0]._pid, self) )

            added = ( (self._mem).count(proc[0]._pid) == proc[0]._size )
            break

        return added


    """
    Modifier to defragment memory.
    :modifies:  _mem, _tick, _last
    :effects:   (to _mem) moves all Processes to start
                (to _tick) increases by frames times T_MEMMOVE
                (to _last) sets to first occurence of FREE
    """
    def defrag( self ):
        tmp_mem = [ "".join(g) for k, g in itertools.groupby( "".join(l for l in
                self._mem), key=lambda obj : (obj != FREE) ) ]
        tmp_moved = 0

        ''' Defragment. '''
        if ( FREE in tmp_mem[0] ):
            tmp_moved = sum([ len(b) for b in tmp_mem if (FREE not in b) ])
            '''self._mem = [ list(l) for l in ( b for b in tmp_mem if (FREE
                    not in b) ) ] + [ list(l) for l in ( b for b in tmp_mem
                    if (FREE in b) ) ]'''
            self._mem = [ list("".join(b)) for b in tmp_mem if (FREE not in tmp_mem) ] \
                    + [ list("".join(b)) for b in tmp_mem if (FREE in tmp_mem) ]
        else:
            tmp_moved = sum([ len(b) for b in tmp_mem[1 :] if (FREE not in b) ])
            self._mem = [ l for l in tmp_mem[0] ] + [ l for l in (s for s in (b
                    for b in tmp_mem[1 :] if (FREE not in b))) ] + [ l for l in
                    (s for s in (b for b in tmp_mem[1 :] if (FREE not in b))) ]

        print(self._mem)
        self._tick += tmp_moved * T_MEMMOVE
        self._last = (self._mem).index( FREE )

        print( ("time {}ms: Defragmentation complete (moved {} frames: "
                "{})\n{}").format( self._tick, tmp_moved, ", ".join(
                p for p in sorted( set( l for l in (b for b in tmp_mem) ) ) ),
                self ) )


        """tmp = len([ l for l in self._mem if (l != FREE) ]) * T_MEMMOVE

        ''' Defragment. '''
        self._mem = [ l for l in self._mem if (l != FREE) ] + [ l for l in
                self._mem if (l == FREE) ]
        self._tick += tmp
        self._last = (self._mem).index( FREE )"""

        ''' Increment arrival times. '''
        for i in range( len(self._procs) ):
            for j in range( len((self._procs[i])._times) ):
                (self._procs[i])._times[j] = ((self._procs[i])._times[j][0] +
                (tmp_moved * T_MEMMOVE), (self._procs[i])._times[j][1])


    """
    Modifier to non-contiguously add new Process.
    """
    def n_add( self, proc ):
        return 0


    """
    Modifier to remove process from running in memory.
    :modifies:  _last, _mem, _remain
    :effects:   (to _last) sets to start of last-removed Process
                (to _mem) removes all Process strings
                (to _remain) removes all Process trackers
    """
    def remove( self ):
        for proc in [ p for p, r in (self._remain).items() if (r == 0) ]:
            ''' Remove from memory and running; change last-accesed. '''
            self._mem = [ (self._mem[i] if self._mem[i] != proc._pid else FREE)
                    for i in range(FRAMES) ]
            del self._remain[proc]

            ''' Notify. '''
            print( "time {}ms: Process {} removed:\n{}".format(self._tick,
                    proc._pid, self) )


    """
    Helper method to increment time.
    :modifies:  _remain, _tick
    :effects:   (to _remain) decrements all remaining time (values)
                (to _tick) increments
    """
    def tick( self ):
        for proc, rem in (self._remain).items():
            self._remain[proc] -= 1

        self._tick += 1


    ''' Overidden methods. ------------------------------------------------- '''

    """
    Repr representation.
    :return:    string with repr representation of all members
    """
    def __repr__( self ):
        return ("Simulator(_procs={0}, _last={1}, _mem={2}, _remain={3}, "
                "_tick={4})").format( repr(self._procs), repr(self._last),
                repr(self._mem), repr(self._remain), repr(self._tick) )


    """
    String representation.
    :return:    framed output of simulator memory
    """
    def __str__( self ):
        return "{0}\n{1}\n{0}".format( ('=' * PER_LINE), "\n".join( "".join(
                line for line in ("".join( let for let in self._mem ))[(PER_LINE
                * i) : PER_LINE * (i+1)]) for i in range(FRAMES // PER_LINE) ) )


''' Simulation methods. ---------------------------------------------------- '''

"""
Contiguous, next-fit simulation.
:param:     procs, list of Processes used in simulation.
"""
def c_next( procs ):
    sim = Simulator( procs )
    print( "time {}ms: Simulator started (Contiguous -- Next-Fit)".format(
            sim.get_tick()) )

    while 1:
        ''' Remove completed Processes. '''
        sim.remove()

        ''' Check for simulation completion. '''
        arrived = sim.arrived()
        if ( (not arrived) and (not sim._remain) ):
            break

        ''' Add new Processes. '''
        for proc in arrived:
            print( "time {}ms: Process {} arrived (requires {} frames)".format(
                    sim.get_tick(), proc[0].get_pid(), proc[0].get_size()) )

            added = sim.c_add( proc, True, False, False )
            if ( added ):
                continue

            elif ( (not added) and ( (sim.get_mem()).count(FREE) >=
                    proc[0].get_size() ) ):
                ''' not added, but space to place after defrag '''
                print( ("time {}ms: Cannot place process {} -- starting "
                        "defragmentation").format(sim.get_tick(),
                        proc[0].get_pid()) )
                sim.defrag()
                """tmp = [ l for l in sim.get_mem() if (l != FREE) ]
                print( ("time {}ms: Defragmentation complete (moved {} frames: "
                        "{})\n{}").format( sim.get_tick(), len(tmp), ", ".join(
                        p for p in sorted( set(tmp) ) ), sim ) )"""

                added = sim.c_add( proc, True, False, False )
                if ( added ):
                    continue

            ''' not added, skip '''
            print ( "time {}ms: Cannot place process {} -- "
                    "skipped!".format(sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        sim.tick()

    print( "time {}ms: Simulator ended (Contiguous -- Next-Fit)\n".format(
            sim.get_tick()) )


"""
Contiguous, best-fit simulation.
:param:     procs, list of Processes used in simulation.
"""
def c_best( procs ):
    sim = Simulator( procs )
    print( "time {}ms: Simulator started (Contiguous -- Best-Fit)".format(
            sim.get_tick()) )

    while 1:
        ''' Remove completed Processes. '''
        sim.remove()

        ''' Check for simulation completion. '''
        arrived = sim.arrived()
        if ( (not arrived) and (not sim._remain) ):
            break

        ''' Add new Processes. '''
        for proc in arrived:
            print( "time {}ms: Process {} arrived (requires {} frames)".format(
                    sim.get_tick(), proc[0].get_pid(), proc[0].get_size()) )

            added = sim.c_add( proc, False, True, False )
            if ( added ):
                continue

            elif ( (not added) and ( (sim.get_mem()).count(FREE) >=
                    proc[0].get_size() ) ):
                ''' not added, but space to place after defrag '''
                print( ("time {}ms: Cannot place process {} -- starting "
                        "defragmentation").format(sim.get_tick(),
                        proc[0].get_pid()) )
                sim.defrag()
                """tmp = [ l for l in sim.get_mem() if (l != FREE) ]
                print( ("time {}ms: Defragmentation complete (moved {} frames: "
                        "{})\n{}").format( sim.get_tick(), len(tmp), ", ".join(
                        p for p in sorted( set(tmp) ) ), sim ) )"""

                added = sim.c_add( proc, False, True, False )
                if ( added ):
                    continue

            ''' not added, skip '''
            print ( "time {}ms: Cannot place process {} -- "
                    "skipped!".format(sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        sim.tick()

    print( "time {}ms: Simulator ended (Contiguous -- Best-Fit)\n".format(
            sim.get_tick()) )


"""
Contiguous, worst-fit simulation.
:param:     procs, list of Processes used in simulation.
"""
def c_worst( procs ):
    sim = Simulator( procs )
    print( "time {}ms: Simulator started (Contiguous -- Worst-Fit)".format(
            sim.get_tick()) )

    while 1:
        ''' Remove completed Processes. '''
        sim.remove()

        ''' Check for simulation completion. '''
        arrived = sim.arrived()
        if ( (not arrived) and (not sim._remain) ):
            break

        ''' Add new Processes. '''
        for proc in arrived:
            print( "time {}ms: Process {} arrived (requires {} frames)".format(
                    sim.get_tick(), proc[0].get_pid(), proc[0].get_size()) )

            added = sim.c_add( proc, False, False, True )
            if ( added ):
                continue

            elif ( (not added) and ( (sim.get_mem()).count(FREE) >=
                    proc[0].get_size() ) ):
                ''' not added, but space to place after defrag '''
                print( ("time {}ms: Cannot place process {} -- starting "
                        "defragmentation").format(sim.get_tick(),
                        proc[0].get_pid()) )
                sim.defrag()
                """tmp = [ l for l in sim.get_mem() if (l != FREE) ]
                print( ("time {}ms: Defragmentation complete (moved {} frames: "
                        "{})\n{}").format( sim.get_tick(), len(tmp), ", ".join(
                        p for p in sorted( set(tmp) ) ), sim ) )"""

                added = sim.c_add( proc, False, False, True )
                if ( added ):
                    continue

            ''' not added, skip '''
            print ( "time {}ms: Cannot place process {} -- "
                    "skipped!".format(sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        sim.tick()

    print( "time {}ms: Simulator ended (Contiguous -- Worst-Fit)\n".format(
            sim.get_tick()) )


"""
Non-contiguous simulation.
:param:     procs, list of Processes used in simulation.
"""
def non_c( procs ):
    sim = Simulator( procs )
    print( "time {}ms: Simulator started (Non-contiguous)".format(sim.get_tick()) )

    ''' simulation code '''

    print( "time {}ms: Simulator ended (Non-contiguous)".format(sim.get_tick()) )


''' Main. ------------------------------------------------------------------ '''

if ( __name__ == "__main__" ):
    if ( len(sys.argv) == 2 ):
        ''' Check for valid input file. '''
        try:
            f = open( sys.argv[1], 'r' )
        except:
            sys.exit( "ERROR: Input file could not be opened" )

        ''' Read in and store as Processes. '''
        procs = []
        for line in [ (l.strip()).split() for l in f.readlines() if ( (l[0] !=
                '#') and (l != '\n') ) ]:
            try:
                tmp = Process( line[0], int(line[1]), deque([ ( int(t.split(
                        '/')[0]), int(t.split('/')[1]) ) for t in line[2:] ]) )
                procs.append( tmp )
            except:
                sys.exit( "ERROR: Invalid input file format" )

        ''' Run simulations. '''
        c_next( procs )
        ''' c_best( procs ) '''
        ''' c_worst( procs ) '''
        ''' non_c( procs ) '''

        sys.exit()

    else:
        ''' too few/many arguments '''
        sys.exit( "ERROR: Invalid argument(s)\nUSAGE: ./a.out <input-file>" )
