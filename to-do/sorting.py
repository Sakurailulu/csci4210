''' sorting.py '''

debug = False
import sys

def read_file( name ):
    try:    f = open( name, 'r' )
    except: raise RuntimeError( "File could not be opened." )

    to_sort = [ line.strip() for line in f.readlines() ]
    return to_sort

if ( __name__ == "__main__" ):
    if ( debug ):   print( "started..." )

    if ( len(sys.argv) == 2 ):
        if ( debug ):   print( "file" )
        to_sort = read_file( sys.argv[1] )

    else:
        if ( debug ):   print( "no file" )
        print( "Enter values to sort on separate lines.\n" + \
                "When done, hit enter without entering new value." )

        to_sort = []
        while True:
            val = input()
            if ( ( val != "" ) and ( val != None ) ):
                to_sort.append( val )
            else:
                break

    print( "\n{:^43}\n".format( "SORTED LIST" ) + ( "-" * 43 ) )
    for i, val in enumerate( sorted(to_sort) ):
        print( "Ind. {:<3} | {:<12}  ==>  {:<12}".format(
            i, to_sort[i], val ) )
    print()

