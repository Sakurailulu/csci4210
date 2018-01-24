''' caching.py '''

debug = False
from collections import defaultdict

def to_hash( val ):
    new_hash = hash( val ) / 47
    while ( new_hash > 47 ):
        new_hash /= 47
    return new_hash

if ( __name__ == "__main__" ):
    if ( debug ):   print( "started..." )
    print( "Enter values to sum on separate lines.\n" + \
            "When done, hit enter without entering new value." )

    cache = defaultdict( int )
    while True:
        val = input()
        if ( ( val != "" ) and ( val != None ) ):
            try:                val = int( val )
            except ValueError:  raise RuntimeError(
                    "Input is not of type int." )

            div_hash = to_hash( val )
            if ( div_hash in cache.keys() ):
                print( " ==> {}".format( cache[div_hash] ) )
            else:
                val_sum = ( val * ( val + 1 ) ) // 2
                cache[div_hash] = val_sum
                print( " ==> {}".format( cache[div_hash] ) )

        else:
            break

