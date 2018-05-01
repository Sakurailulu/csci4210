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

from collections import defaultdict as ddict, deque, OrderedDict as odict
from copy import deepcopy
from itertools import groupby
from sys import argv, exit

FRAMES = 256
FREE = '.'
PER_LINE = 32
T_MEMMOVE = 1


''' Wrapper class for process management. ---------------------------------- '''

class Process:

    """
    Constructor.
    :param:     pid, Process ID value
                size, amount of space taken in memory
                time, deque of arrival and run times
    """
    def __init__( self, pid, size, times ):
        self._pid = deepcopy( pid )
        self._size = deepcopy( size )
        self._times = deepcopy( times )


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

    """
    Constructor.
    :param:     procs, deep-copied list of Processes in simulator
    """
    def __init__( self, procs ):
        self._procs = deepcopy( procs )

        ''' Non-variable initializing values. '''
        self._last = 0
        self._mem = [ FREE for i in range(FRAMES) ]
        self._NC_memory = FREE*FRAMES
        self._TLB = {}
        self._remain = ddict( int )
        self._tick = 0
        for x in range(256):
            self._TLB[x] = (FREE,0)


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


    ''' Modifiers. --------------------------------------------------------- '''

    """
    Modifier to contiguously add new Process.
    :modifies:  _mem, _remain, _last
    :effects:   (to _mem) adds Process according to simluation
                (to _remain) adds Process mapped to full run time
                (to _last) sets to just past the end of Process
    """
    def c_add( self, proc, next=False, best=False, worst=False ):
        p, rem = proc[0], proc[1]
        if ( next ):
            tmp = [ "".join(g) for k, g in groupby("".join(l for l in
                    self._mem[self._last : FRAMES])) ] + [ "".join(g) for k, g
                    in groupby("".join(l for l in self._mem[0 : self._last])) ]

        else:
            tmp = [ "".join(g) for k, g in groupby( "".join(l for l in
                    self._mem) ) ]

        added = False
        for block in sorted( [ b for b in tmp if ( (FREE in b) and (len(b) >=
                p._size) ) ], key=lambda l : (len(l) if (not next) else (not
                len(l)) ), reverse=worst ):
            tmp[ tmp.index(block) ] = "{}{}".format( str(p), ( FREE * (len(block) - p._size) ) )

            self._mem = ( list( "".join(tmp) ) if (not next) else (list(
                    "".join(tmp) )[(FRAMES - self._last) : FRAMES] + list(
                    "".join(tmp) )[0 : (FRAMES - self._last)]) )
            self._remain[ p ] = rem
            self._last = max([ i for i, l in enumerate(self._mem) if (l ==
                    p._pid) ]) + 1

            print( "time {}ms: Placed process {}:\n{}".format(self._tick,
                    p._pid, self) )

            added = ( (self._mem).count(p._pid) == p._size )
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
        mem = [ "".join(g) for k, g in groupby( "".join(l for l in self._mem),
                key=lambda obj : (obj != FREE) ) ]
        tmp = sum( [len(b) for b in mem if (FREE not in b)] ) if (FREE in
                mem[0]) else sum( [len(b) for b in mem[1:] if (FREE not in b)] )

        ''' Defragment. '''
        self._mem = [ l for l in self._mem if (l != FREE) ] + [ l for l in
                self._mem if (l == FREE) ]
        self._tick += tmp * T_MEMMOVE
        self._last = (self._mem).index( FREE )

        print( ("time {}ms: Defragmentation complete (moved {} frames: "
                "{})\n{}").format( self._tick, tmp, (", ".join( list(
                odict.fromkeys("".join( list( filter(lambda obj : obj != FREE,
                "".join(mem)) ) )) ) )) if (FREE in mem[0]) else (", ".join(
                list( odict.fromkeys("".join( list( filter(lambda obj : obj !=
                FREE, "".join(mem[1:])) ) )) ) )), self ) )

        ''' Increment arrival times. '''
        self._procs = [ Process(p._pid, p._size, (deque([ ((t[0] + (tmp *
                T_MEMMOVE)), t[1]) for t in p._times ]))) for p in self._procs ]


    """
    Modifier to non-contiguously add new Process.
    """
    def non_c_add( self, proc):
        added = False
        if(self._NC_memory.count(FREE)>= proc[0]._size):
            count = proc[0]._size
            for x in range(256):
                if(self._NC_memory[x] == FREE):
                    self._NC_memory = self._NC_memory[:x] + proc[0]._pid + self._NC_memory[x+1:] 
                    #self._NC_memory[x] = proc[0]._pid
                    self._TLB[x] = ('A',proc[0]._size - count)
                    count -=1
                if(count <=0):
                    break
            self._remain[ proc[0] ] = proc[1]
            added = True
            print( "time {}ms: Placed process {}:".format(self._tick,
                    proc[0]._pid) )
            #self.NC_Print
            
            print("================================")
            for x in range(8):
                print(self._NC_memory[32*x:(32*x)+32])
            print("================================")
            #self.print_page_TLB
            print("PAGE TABLE [page,frame]:")
            pids = []
            for proc in [p for p, r in (self._remain).items()]:
                pids.append(proc._pid)
            for pid in sorted(pids):
                occurances = []
                #tmp = proc._pid + ": "
                for x in range(256):
                    if(self._NC_memory[x] == pid):
                        occurances.append((self._TLB[x][1],x))
                        #tmp = tmp + '[' + str(self._TLB[x][1]) + ',' + str(x) + "] "
                count = len(occurances)
                occurances = sorted(occurances, key=lambda tup: tup[1], reverse = True)
                tmp = pid + ": "
                while(count >0):
                    for i in range(10):
                        if(count<=0):
                            break 
                        else:
                            if(i == 0):
                                tmp = tmp + "[" + str(occurances[count-1][0]) + "," + str(occurances[count-1][1]) + "]"
                            else:
                                tmp = tmp + " [" + str(occurances[count-1][0]) + "," + str(occurances[count-1][1]) + "]"
                        count-=1
                    print(tmp)
                    tmp = ""
        return added
     """
    Modifier to non-contiguously remove Process.
    """
    def non_c_remove(self):
        for proc in [ p for p, r in (self._remain).items() if (r == 0) ]:
                for x in range(256):
                    if(self._NC_memory[x] == proc._pid):
                        self._NC_memory = self._NC_memory[:x] + FREE + self._NC_memory[x+1:] 
                        self._TLB[x] = (FREE,0)
                print( "time {}ms: Process {} removed:".format(self._tick,
                    proc._pid) )
                #self.NC_Print
                print("================================")
                for x in range(8):
                    print(self._NC_memory[32*x:(32*x)+32])
                print("================================")
                print("PAGE TABLE [page,frame]:")
                #self.print_page_TLB

                del self._remain[proc]
                pids = []
                for proc in [p for p, r in (self._remain).items()]:
                    pids.append(proc._pid)
                for pid in sorted(pids):
                    occurances = []
                    #tmp = proc._pid + ": "
                    for x in range(256):
                        if(self._NC_memory[x] == pid):
                            occurances.append((self._TLB[x][1],x))
                            #tmp = tmp + '[' + str(self._TLB[x][1]) + ',' + str(x) + "] "
                    count = len(occurances)
                    occurances = sorted(occurances, key=lambda tup: tup[1], reverse = True)
                    tmp = pid + ": "
                    while(count >0):
                        for i in range(10):
                            if(count<=0):
                                break 
                            else:
                                if(i == 0):
                                    tmp = tmp + "[" + str(occurances[count-1][0]) + "," + str(occurances[count-1][1]) + "]"
                                else:
                                    tmp = tmp + " [" + str(occurances[count-1][0]) + "," + str(occurances[count-1][1]) + "]"
                            count-=1
                        print(tmp)
                        tmp = ""

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
            ''' self._last = (self._mem).index( proc._pid ) '''
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
            sim._tick) )

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
                    sim._tick, proc[0]._pid, proc[0]._size) )

            added = sim.c_add( proc, True, False, False )

            if ( (not added) and ( (sim._mem).count(FREE) >= proc[0]._size ) ):
                ''' not added, but space to place after defrag '''
                print( ("time {}ms: Cannot place process {} -- starting "
                        "defragmentation").format(sim._tick,
                        proc[0]._pid) )

                sim.defrag()
                added = sim.c_add( proc, True, False, False )

            if ( not added ):
                ''' not added, skip '''
                print ( "time {}ms: Cannot place process {} -- skipped!".format(
                        sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        sim.tick()

    print( "time {}ms: Simulator ended (Contiguous -- Next-Fit)\n".format(
            sim._tick) )


"""
Contiguous, best-fit simulation.
:param:     procs, list of Processes used in simulation.
"""
def c_best( procs ):
    sim = Simulator( procs )
    print( "time {}ms: Simulator started (Contiguous -- Best-Fit)".format(
            sim._tick) )

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
                    sim._tick, proc[0]._pid, proc[0]._size) )

            added = sim.c_add( proc, False, True, False )

            if ( (not added) and ( (sim._mem).count(FREE) >= proc[0]._size ) ):
                ''' not added, but space to place after defrag '''
                print( ("time {}ms: Cannot place process {} -- starting "
                        "defragmentation").format(sim._tick,
                        proc[0]._pid) )

                sim.defrag()
                added = sim.c_add( proc, False, True, False )

            if ( not added ):
                ''' not added, skip '''
                print ( "time {}ms: Cannot place process {} -- skipped!".format(
                        sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        sim.tick()

    print( "time {}ms: Simulator ended (Contiguous -- Best-Fit)\n".format(
            sim._tick) )


"""
Contiguous, worst-fit simulation.
:param:     procs, list of Processes used in simulation.
"""
def c_worst( procs ):
    sim = Simulator( procs )
    print( "time {}ms: Simulator started (Contiguous -- Worst-Fit)".format(
            sim._tick) )

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
                    sim._tick, proc[0]._pid, proc[0]._size) )

            added = sim.c_add( proc, False, False, True )

            if ( (not added) and ( (sim._mem).count(FREE) >= proc[0]._size ) ):
                ''' not added, but space to place after defrag '''
                print( ("time {}ms: Cannot place process {} -- starting "
                        "defragmentation").format(sim._tick,
                        proc[0]._pid) )

                sim.defrag()
                added = sim.c_add( proc, False, False, True )

            if ( not added ):
                ''' not added, skip '''
                print ( "time {}ms: Cannot place process {} -- skipped!".format(
                        sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        sim.tick()

    print( "time {}ms: Simulator ended (Contiguous -- Worst-Fit)\n".format(
            sim._tick) )


"""
Non-contiguous simulation.
:param:     procs, list of Processes used in simulation.
"""
def non_c( procs ):
    sim = Simulator( procs )
    print( "time {}ms: Simulator started (Non-contiguous)".format(sim._tick) )

    ''' simulation code '''
    while 1:
        ''' Remove done processes. '''
        sim.non_c_remove()

        ''' Check for simulation completion. '''
        arrived = sim.arrived()
        if ( (not arrived) and (not sim._remain) ):
            break

        ''' Added new processes. '''
        for proc in arrived:
            print( "time {}ms: Process {} arrived (requires {} frames)".format(
                    sim._tick, proc[0]._pid, proc[0]._size) )

            added = sim.non_c_add( proc)
            if ( added ):
                continue

            else: 
                ''' not added : skip process '''
                print ( "time {}ms: Cannot place process {} -- "
                    "skipped!".format(sim._tick, proc[0]._pid) )

        ''' Decrement all run times and tick. '''
        sim.tick()
    print( "time {}ms: Simulator ended (Non-contiguous)".format(sim._tick) )


''' Main. ------------------------------------------------------------------ '''

if ( __name__ == "__main__" ):
    if ( len(argv) == 2 ):
        ''' Check for valid input file. '''
        try:
            f = open( argv[1], 'r' )
        except:
            exit( "ERROR: Input file could not be opened" )

        ''' Read in and store as Processes. '''
        procs = []
        for line in [ (l.strip()).split() for l in f.readlines() if ( (l[0] !=
                '#') and (l != '\n') ) ]:
            try:
                tmp = Process( line[0], int(line[1]), deque([ ( int(t.split(
                        '/')[0]), int(t.split('/')[1]) ) for t in line[2:] ]) )
                procs.append( tmp )
            except:
                exit( "ERROR: Invalid input file format" )

        ''' Run simulations. '''
        c_next( procs )
        c_best( procs )
        c_worst( procs )
        non_c( procs )

        exit()

    else:
        ''' too few/many arguments '''
        exit( "ERROR: Invalid argument(s)\nUSAGE: ./a.out <input-file>" )
