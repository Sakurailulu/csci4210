from __future__ import print_function
from collections import defaultdict as ddict
from collections import deque
import os
import sys

DEBUG = False

n = 0
t_cs = 8
t_slice = 80
rr_add = 0

"""
Process helper class to store process details from input file.
"""
class Process:
    """
    Default constructor.
    """
    def __init__( self, pid, arrival, burst, num, io ):
        self._pid = pid
        self._init_arrival = arrival
        self._burst = burst
        self._num = num
        self._io = io
        self._curr_arrival = 0

    # ------------------------------------------------------------------------ #
    # Overidden methods #

    """
    Repr representation.
    :return:    raw representation of all data in Process object.
    """
    def __repr__( self ):
        repr_rep = "_pid = {}; _arrival = {}; ".format( self._pid, self._init_arrival )
        repr_rep += "_burst = {}; _num = {}; _io = {}".format( self._burst, self._num, self._io )
        return repr_rep


    """
    String representation of Process.
    :return:    string detailing Process info.
    """
    def __str__( self ):
        str_rep = "Process {} arrives at {}ms ".format( self._pid, self._init_arrival )
        str_rep += "and performs {} bursts of {}ms with ".format( self._num, self._burst )
        str_rep += "{}ms of IO.".format( self._io )
        return str_rep


"""
CPU helper class to better keep track of processes throughout scheduling.
"""
class CPU:
    """
    Default constructor.
    """
    def __init__( self, procs ):
        self._ticker = 0
        self._curr = None
        self._ready = deque()
        self._finished = []
        self._procs = procs
        self._io = ddict( Process )

        # Simpue output trackers. #
        self._avg_burst = ( sum([ (proc._burst * proc._num) for proc in self._procs ]) \
                / sum([ proc._num for proc in self._procs ]) )
        self._total_wait = 0
        self._avg_wait = 0
        self._total_turnaround = 0
        self._avg_turnaround = 0
        self._context = 0
        self._preempt = 0

    # ------------------------------------------------------------------------ #
    # Accessors #
    
    """
    String representation of CPU ready queue.
    :return:    string in the format of '[Q *contents of _ready*]
    """
    def queue( self ):
        contents = "[Q"
        for proc in self._ready:
            contents += ' ' + proc._pid
        contents += ']'
        if ( contents == "[Q]" ):
            contents = "[Q <empty>]"

        return contents

    # ------------------------------------------------------------------------ #
    # Modifiers #

    """
    """
    def add( self, proc ):
        self._curr = proc
        self._ticker += t_cs // 2


    """
    """
    def remove( self ):
        self._curr = None
        self._ticker += t_cs // 2
        self._context += 1

    # ------------------------------------------------------------------------ #
    # Overridden methods #
    
    """
    Repr representation.
    :return:    raw representation of all data in CPU object.
    """
    def __repr__( self ):
        repr_rep = "_ticker = {}, _curr = {} deque(["
        for proc in self._ready:
            repr_rep += "{}, ".format( proc._pid )

        return repr_rep[ :-2 ] + "])"


    """
    String representation of CPU.
    :return: string with time, current process, and string rep of ready queue.
    """
    def __str__( self ):
        str_rep = "Time {}: current process is {} ".format( self._ticker, self._curr._id )
        str_rep += "with ready queue {}".format( self.queue() )

        return str_rep

# ---------------------------------------------------------------------------- #

"""
Helper method to print to stderr.
:param:     *args, all non-keyworded arguments.
            **kwargs, all keyworded arguments.
"""
def err( *args, **kwargs ):
    print( *args, file=sys.stderr, **kwargs )


"""
Helper method to read in file input.
:param:     f_name, file name to find in directory and read.
:return:    ddict of process id mapped to tuple of details, if successful
            os.EX_IOERR, if file fails, ex.EX_DATAERR, if file is not formatted
"""
def read_file( f_name ):
    pwd = os.path.dirname( __file__ )
    path = os.path.join( pwd, f_name )

    try:
        f = open( path, 'r' )
    except:
        return os.EX_IOERR

    procs = []
    for line in f:
        if ( line[0] != '#' ):
            line = line.strip().split('|')
            try:
                tmp = [ int(line[i]) for i in range(1, 5) ]
                procs.append( Process(line[0], tmp[0], tmp[1], tmp[2], tmp[3]) )
                # procs[ line[0] ] = tuple([ int(line[i]) for i in range(1, 5) ])
            except:
                f.close()
                return os.EX_DATAERR

    f.close()
    return procs


"""
"""
def build_simple( lines, res ):
    lines.append( "-- average CPU burst time: {0:.2f} ms\n".format(res[0]) )
    lines.append( "-- average wait time: {0:.2f} ms\n".format(res[1]) )
    lines.append( "-- average turnaround time: {0:.2f} ms\n".format(res[2]) )
    lines.append( "-- total number of context switches: {}\n".format(res[3]) )
    lines.append( "-- total number of preemptions: {}\n".format(res[4]) )

    return lines


"""
First come, first serve simulation.
:param:     procs, list of Process objects to simulate.
:return:    five-tuple with simple output tracking variables.
"""
def run_fcfs( procs ):
    cpu = CPU( procs )
    print( "time {}ms: Simulator started for FCFS {}".format(cpu._ticker, cpu.queue()) )

    while len(cpu._finished) != n:
        for proc in cpu._procs:
            if ( proc._init_arrival == cpu._ticker ):
                (cpu._ready).append( proc )
                print( "time {}ms: Process {} arrived and added to ready queue {}".format(\
                        cpu._ticker, proc._pid, cpu.queue()) )

        if ( cpu._ticker in (cpu._io).keys() ):
            tmp = cpu._io[cpu._ticker]
            del cpu._io[cpu._ticker]
            (cpu._ready).append( tmp )

            print( "time {}ms: Process {} completed I/O; added to ready queue {}".format(
                        cpu._ticker, tmp._pid, cpu.queue()) )

        if ( cpu._curr != None ):
            # running #
            if ( ( cpu._ticker - (cpu._curr)._curr_arrival ) == (cpu._curr)._burst ):
                (cpu._curr)._num -= 1
                if ( (cpu._curr)._num > 0 ):
                    if ( (cpu._curr)._num == 1 ):
                        print( "time {}ms: Process {} completed a CPU burst; {} burst to go {}".format(\
                                cpu._ticker, (cpu._curr)._pid, (cpu._curr)._num, 
                                cpu.queue() ) )
                    else:
                        print( "time {}ms: Process {} completed a CPU burst; {} bursts to go {}".format(\
                                cpu._ticker, (cpu._curr)._pid, (cpu._curr)._num, 
                                cpu.queue() ) )
                    
                    tmp = ( cpu._ticker + (cpu._curr)._io + (t_cs // 2) )
                    cpu._io[tmp] = cpu._curr
                    print( "time {}ms: Process {} switching out of CPU; will block on I/O until time {}ms {}".format(
                            cpu._ticker, (cpu._curr)._pid, tmp, cpu.queue() ) )
                    
                    cpu.remove()
                    continue

                else:
                    print( "time {}ms: Process {} terminated {}".format(cpu._ticker, 
                            (cpu._curr)._pid, cpu.queue()) )
                    (cpu._finished).append( cpu._curr )
                    cpu.remove()
                    continue   

        else:
            # not running#
            if ( cpu._ready ):
                cpu.add( (cpu._ready).popleft() )
                (cpu._curr)._curr_arrival = cpu._ticker
                print( "time {}ms: Process {} started using the CPU {}".format(
                        cpu._ticker, (cpu._curr)._pid, cpu.queue()) )

        cpu._ticker += 1

    print( "time {}ms: Simulator ended for FCFS\n".format(cpu._ticker) )
    return ( cpu._avg_burst, cpu._avg_turnaround, cpu._avg_wait, \
            cpu._context, cpu._preempt )


"""
Shortest remaining time simulation.
:param:     procs, list of Process objects to simulate.
:return:    five-tuple with simple output tracking variables.
"""
def run_srt( procs ):
    cpu = CPU( procs )
    print( "time {}ms: Simulator started for SRT {}".format(cpu._ticker, cpu.queue()) )
    
    print( "time {}ms: Simulator ended for SRT\n".format(cpu._ticker) )
    return ( cpu._avg_burst, cpu._avg_turnaround, cpu._avg_wait, \
            cpu._context, cpu._preempt )


"""
Round robin simulation.
:param:     procs, list of Process objects to simulate.
:return:    five-tuple with simple output tracking variables.
"""
def run_rr( procs ):
    cpu = CPU( procs )
    print( "time {}ms: Simulator started for RR {}".format(cpu._ticker, cpu.queue()) )
    
    print( "time {}ms: Simulator ended for RR".format(cpu._ticker) )
    return ( cpu._avg_burst, cpu._avg_turnaround, cpu._avg_wait, \
            cpu._context, cpu._preempt )

# ---------------------------------------------------------------------------- #

if ( __name__ == "__main__" ):
    if ( ( len(sys.argv) == 3 ) or ( len(sys.argv) == 4 ) ):
        # Check to make sure that optional argument 3 is valid. #
        if ( len(sys.argv) == 4 ):
            if ( sys.argv[3] == "BEGINNING" ):
                rr_add = 1

            elif ( sys.argv[3] != "END" ):
                err( "ERROR: Invalid arguments" )
                sys.exit( "USAGE: ./a.out <input-file> <stats-output-file> [<rr-add>]" )

        # Open write-to file to make sure file is valid. #
        pwd = os.path.dirname( __file__ )
        path = os.path.join( pwd, sys.argv[2] )
        try:
            f = open( path, 'w' )
        except:
            sys.exit( "ERROR: Invalid output file" )

        procs = read_file( sys.argv[1] )
        if ( isinstance(procs, list) ):
            n = len( procs )
            if DEBUG:
                print( "{} processes...".format(n) )
                for proc in procs:
                    print( "  " + str(proc) )

            fcfs_res = run_fcfs( procs )
            # srt_res = run_srt( procs )
            # rr_res = run_rr( procs )

            simple_out = []

            # Add FCFS results. #
            simple_out.append( "Algorithm FCFS\n" )
            simple_out = build_simple( simple_out, fcfs_res )
            # Add SRT results. #
            simple_out.append( "Algorithm SRT\n" )
            # simple_out = build_simple( simple_out, srt_res )
            # Add RR results. #
            simple_out.append( "Algorithm RR\n" )
            # simple_out = build_simple( simple_out, rr_res )

            f.writelines( simple_out )
            f.close()

            sys.exit()

        else:
            f.close()
            sys.exit( "ERROR: Invalid input file format" )

    else:
        err( "ERROR: Invalid arguments" )
        sys.exit( "USAGE: ./a.out <input-file> <stats-output-file> [<rr-add>]" )
