#!/usr/bin/python3

"""
Griffin Melnick, melnig@rpi.edu
Peter Straub, straup@rpi.edu

    Program info...

        bash$ python3 project2.py ...

    Options...
"""

from collections import OrderedDict as odict
from itertools import groupby
import copy
import sys

FRAMES = 256
FREE = '.'
PER_LINE = 32
T_MEMMOVE = 1

''' Process helper class. -------------------------------------------------- '''

class Process:

    ''' Constructors. ------------------------------------------------------ '''

    """
    Default contructor.
    :param:     pid, process id (single letter A-Z)
                mem, memory size taken
                times, OrderDict of arrival times mapped to run times
    """
    def __init__( self, pid, mem, times ):
        self._pid = copy.deepcopy( pid )
        self._mem = copy.deepcopy( mem )
        self._times = copy.deepcopy( times )


    ''' Overriden methods. ------------------------------------------------- '''

    """
    Repr representation of Process.
    :return:    string with all variables shown with respective repr outputs
    """
    def __repr__( self ):
        return "_pid = {0}, _mem = {1}, _times = {2}".format( repr(self._pid),
                repr(self._mem), repr(self._times) )


    """
    String representation of Process.
    :return:    string with Process _pid, _mem times
    """
    def __str__( self ):
        return "{0}".format( str(self._pid) * self._mem )


''' Simluator helper class. ------------------------------------------------ '''

class Simulator:

    ''' Constructors. ------------------------------------------------------ '''

    """
    Default constructor.
    :param:     procs, list of Processes pulled from input file.
    """
    def __init__( self, procs ):
        self._procs = copy.deepcopy( procs )
        self._memory = list( str(FREE * FRAMES) )
        self._tick = 0


    ''' Class methods. ----------------------------------------------------- '''

    """
    Helper method for defragmenting.
    :modifies:  self._memory
    :effects:   moves all FREE memory to end of list
    """
    def defrag( self ):
        tmpLst = [ "".join(g) for k, g in groupby( "".join(l for l in
                self._memory) ) ]
        self._memory = list( "".join( g for g in tmpLst if FREE not in g ) +
                "".join( g for g in tmpLst if FREE in g ) )


    ''' Overridden methods. ------------------------------------------------ '''

    """
    Repr representation of Simulator.
    :return:    string with all variables shown with respective repr outputs
    """
    def __repr__( self ):
        return "_procs = {0}, _memory = {1}, _tick = {2}".format(
                repr(self._procs), repr( "".join(l for l in self._memory) ),
                repr(self._tick) )


    """
    String representation of Simulator.
    :return:    string representation of memory space as a frame
    """
    def __str__( self ):
        return "{0}\n{1}\n{2}".format( ('=' * PER_LINE), "\n".join( "".join(l
                for l in self._memory[(PER_LINE * i) : (PER_LINE * (i + 1))])
                for i in range(FRAMES // PER_LINE) ), ('=' * PER_LINE) )


''' Methods. --------------------------------------------------------------- '''

"""
Contiguous, next-fit simulation.
:param:     procs, list of Processes to simulate
"""
def c_next( procs ):
    sim = Simulator( copy.deepcopy(procs) )
    print( "time {}ms: Simulator started (Contiguous -- Next-fit)".format(
            sim._tick) )

    ''' simulation code '''

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
                tmp = Process( l[0], int(l[1]), odict([ ( int(t.split('/')
                        [0]), int(t.split('/')[1]) ) for t in l[2:] ]) )
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

