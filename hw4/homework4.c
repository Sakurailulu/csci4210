/* homework4.c
 * Griffin Melnick, melnig@rpi.edu
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* Macro defintions. */
#define BUFFER_MAX 994
#define ID_MIN 3
#define ID_MAX 20
#define TCP "tcp"
#define UDP "udp"
#define USER_INC 32

typedef struct {
    char * _id, _connection;
} User;

#define USER_SIZE sizeof( User )

unsigned int numUsers = 0;
User * users;
int tcpSd = -1, udpSd = -1, tcpServerLen = -1, udpServerLen = -1;
struct sockaddr_in tcpServer, udpServer;

/* Method declarations. ----------------------------------------------------- */



/* Method definitions. ------------------------------------------------------ */



/* Main. -------------------------------------------------------------------- */

int main( int argc, char * argv[] ) {
    if ( argc == 3 ) {
        char * tmp;
        const unsigned short tcpPort = strtol( argv[1], &tmp, 10 ),
                udpPort = strtol( argv[2], &tmp, 10 );

#ifdef DEBUG_MODE
        printf( "tcp %u --- udp %u\n", tcpPort, udpPort );
#endif

        /* Initialize TCP socket. */
        if ( ( tcpSd = socket(PF_INET, SOCK_STREAM, 0) ) < 0 ) {
            fprintf( stderr, "MAIN: ERROR TCP socket() failed\n" );
            return EXIT_FAILURE;
        }

        tcpServer.sin_family = PF_INET;
        tcpServer.sin_port = htons( tcpPort );
        (tcpServer.sin_addr).s_addr = htonl( INADDR_ANY );
        tcpServerLen = sizeof( tcpServer );

        if ( bind( tcpSd, (struct sockaddr *)&tcpServer, tcpServerLen ) < 0 ) {
            fprintf( stderr, "MAIN: ERROR TCP bind() failed\n" );
            return EXIT_FAILURE;
        }

        if ( listen( tcpSd, 5 ) < 0 ) {
            fprintf( stderr, "MAIN: ERROR TCP listen() failed\n" );
            return EXIT_FAILURE;
        }

        /* Initialize UDP socket. */
        if ( ( udpSd = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 ) {
            fprintf( stderr, "MAIN: ERROR UDP socket() failed\n" );
            return EXIT_FAILURE;
        }

        udpServer.sin_family = AF_INET;
        udpServer.sin_port = htons( udpPort );
        (udpServer.sin_addr).s_addr = htonl( INADDR_ANY );
        udpServerLen = sizeof( udpServer );

        if ( bind( udpSd, (struct sockaddr *)&udpServer, udpServerLen ) < 0 ) {
            fprintf( stderr, "MAIN: ERROR UDP bind() failed\n" );
            return EXIT_FAILURE;
        }

        if ( getsockname( udpSd, (struct sockaddr *)&udpServer, (socklen_t
                *)&udpServerLen ) < 0 ) {
            fprintf( stderr, "MAIN: ERROR UDP getsockname() failed\n" );
            return EXIT_FAILURE;
        }

        printf( "MAIN: Started server\n" );
        printf( "MAIN: Listening for TCP connections on port: %d\n",
                ntohs(tcpServer.sin_port) );
        printf( "MAIN: Listening for UDP datagrams on port: %d\n",
                ntohs(udpServer.sin_port) );
        fflush( stdout );

        users = calloc( USER_INC, USER_SIZE );

        char tcpBuffer[ BUFFER_MAX ], udpBuffer[ BUFFER_MAX ];
        struct sockaddr_in tcpClient, udpClient;
        int tcpClientLen = sizeof( tcpClient ),
                udpClientLen = sizeof( udpClient );
        /* to clear temporary warning */
        strcat( tcpBuffer, "\0" );
        tcpClientLen = 0;
        if ( tcpPort != udpPort ) {
            int tcpIn = -1, udpIn = -1;
            while ( 1 ) {
                if ( ( tcpIn =  ) )
                if ( ( udpIn = recvfrom(udpSd, udpBuffer, BUFFER_MAX, 0, (struct
                        sockaddr *)&udpClient, (socklen_t *)&udpClientLen) ) <
                        0 ) {
                    fprintf( stderr, "MAIN: ERROR UDP recvfrom() failed\n" );
                } else {
                    printf( "MAIN: Rcvd incoming UDP datagram from: %s\n",
                            inet_ntoa(udpClient.sin_addr) );
                    udpBuffer[ udpIn ] = '\0';

                    char * udpRead = strstr( udpBuffer, "LOGIN" );
                    if ( udpRead == udpBuffer ) {

                    }

                    udpRead = strstr( udpBuffer, "WHO" );
                    if ( udpRead == udpBuffer ) {
                        int tmp = 3 + ( ID_MAX * (numUsers + 1) );
                        char whoBuffer[ tmp ];
                        strcat( whoBuffer, "OK\n" );
                        for ( int i = 0; i < numUsers; ++i ) {
                            strcat( whoBuffer, strcat( users[i]._id, "\n" ) );
                        }
                        strcat( whoBuffer, "\0" );

                        if ( ( sendto(udpSd, whoBuffer, tmp, 0, (struct sockaddr
                                *)&udpClient, udpClientLen) ) < 0 ) {
                            fprintf( stderr, "MAIN: ERROR UDP sendto() failed"
                                    "\n" );
                        }
                    }

                    udpRead = strstr( udpBuffer, "LOGOUT" );
                    if ( udpRead == udpBuffer ) {

                    }

                    udpRead = strstr( udpBuffer, "SEND" );
                    if ( udpRead == udpBuffer ) {

                    }

                    udpRead = strstr( udpBuffer, "BROADCAST" );
                    if ( udpRead == udpBuffer ) {

                    }

                    udpRead = strstr( udpBuffer, "SHARE" );
                    if ( udpRead == udpBuffer ) {

                    }
                }
            }
        } else {
            /* need to use select() calls */
            /* int in = -1; */
            while ( 1 ) {
                /* ... */
            }
        }

        return EXIT_SUCCESS;
    } else {
        /* too few/many arguments */
        fprintf( stderr, "MAIN: ERROR Invalid argument(s)\n" );
        fprintf( stderr, "MAIN: USAGE a.out <tcp-port> <udp-port>\n" );
    }
    return EXIT_FAILURE;
}
