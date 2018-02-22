'''
    project1.py
    Griffin Melnick, melnig@rpi.edu
    Peter Straub, straup@rpi.edu

    CPU process scheduling simulation built with three scheduling algorithms:
    (1) first come, first served [FCFS]; (2) shortest remaining time [SRT]; (3)
    round robin [RR]. An input file with process ids, arrival times, burst
    times, burst counts, and I/O times is parsed and simulated using each
    algorithm. A detailed watch of the CPU is printed followed by a closing
    summary. The program is called by:

        bash$ a.out <input-file> <stats-output-file> [<rr-add>]

    where <input-file> is the name of the formatted file to be parsed,
    <stats-output-file> is the name of the file to which to print results, and
    the optional <rr-add> to determine whether processes should be added to the
    BEGINNING or END of the ready queue in the round robin simulation.
'''

from __future__ import print_function
from collections import defaultdict as d_dict
import os
import sys

# RR_ADD is False is END, True if BEGINNING. #
RR_ADD = False

# Used to print debugging output. #
DEBUG = False
# DEBUG = True


# ---------------------------------------------------------------------------- #

'''
Helper method to print to stderr.
:param:     *args, all non-keyworded arguments.
            **kwargs, all keyworded arguments.
'''
def err( *args, **kwargs ):
    print( *args, file=sys.stderr, **kwargs )


'''
Helper method to read in file input.
:param:     f_name, file name to find in directory and read.
:return:    d_dict of process id mapped to tuple of details, if successful
            os.EX_IOERR, if file fails, ex.EX_DATAERR, is file is not formatted
'''
def read_file( f_name ):
    pwd = os.path.dirname( __file__ )
    path = os.path.join( pwd, f_name )

    try:
        f = open( path, 'r' )
    except:
        err( "ERROR: Could not open file" )
        return os.EX_IOERR

    procs = d_dict( tuple )
    for line in f:
        if ( line[0] != '#' ):
            line = line.strip().split('|')
            try:
                procs[ line[0] ] = tuple([ int(line[i]) for i in range(1, 5) ])
            except:
                err( "ERROR: Invalid input file format" )
                return os.EX_DATAERR

    f.close()
    return procs


'''
Simulator for FCFS algorithm.
:param:     procs, d_dict of processes to simulate.
:return:    5-tuple of simulation results.
'''
def fcfs( procs ):
    res = tuple()
    return res


'''
Simulator for SRT algorithm.
:param:     procs, d_dict of processes to simulate.
:return:    5-tuple of simulation results.
'''
def srt( procs ):
    res = tuple()
    return res


'''
Simulator for RR algorithm.
:param:     procs, d_dict of processes to simulate.
            add, RR_ADD global to determine queue addtion.
:return:    5-tuple of simulation results.
'''
def rr( procs, add=False ):
    res = tuple()
    return res


# ---------------------------------------------------------------------------- #

if ( __name__ == "__main__" ):
    if ( ( len(sys.argv) == 3 ) or ( len(sys.argv) == 4 ) ):

        # check to make sure that arg 3 is BEGINNING or END #
        if ( len(sys.argv) == 4 ):
            if ( sys.argv[3] == "BEGINNING" ):
                RR_ADD = True
            elif ( sys.argv[3] != "END" ):
                err( "ERROR: Invalid arguments" )
                err( "USAGE: ./a.out <input-file> <stats-output-file> [<rr-add>]" )

        procs = read_file( sys.argv[1] )

        # make sure that read_file() didn't return an error #
        if ( isinstance(procs, d_dict) ):
            if ( DEBUG ):
                print( "procs = " )
                for key, val in procs.items():
                    print( "  {} -> {}".format(key, val) )

            fcfs_res = fcfs( procs )
            srt_res = srt( procs )
            rr_res = rr( procs, RR_ADD )

    else:
        # ( len( sys.argv ) != 3 ) and ( len( sys.argv ) != 4 ) #
        err( "ERROR: Invalid arguments" )
        err( "USAGE: ./a.out <input-file> <stats-output-file> [<rr-add>]" )

