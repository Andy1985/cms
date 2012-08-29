#include "Communication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Signal
#define NETPOWER "NetPower"

//Command
#define LIST "List"
#define SEND "Send"
#define File "File"
#define DOWN "Down"

#define TIMEOUT 1200

void init_result( char **result )
{
    *result = NULL;
}

void destroy_result( char **result )
{
    if( NULL != *result )
    {
        free( *result );
        *result = NULL;
    }
}

int send_package( int sockfd, const char *data, int size )
{
    if( 4 != write( sockfd, &size, 4 ) )
    {
        return -1;
    }

    int nSize = 0;

    while( nSize < size )
    {
        if( write( sockfd, data + nSize, 1 ) == 1 )
        {
            nSize++;
        }
    }

    return 0;

}

int recv_package( int sockfd, char **data )
{
    int size = 0;
    if( read( sockfd, &size, 4 ) == -1 )
    {
        return -1;
    }

    size++;

    *data = (char*)realloc( *data, size * sizeof( char ) );

    if( NULL == *data )
    {
        fprintf( stderr, "malloc failed!" );
        return -1;
    }

    memset( *data, '\0', size );

    char buffer[1024] = { '\0' };

    while( read( sockfd, buffer, 1023 ) > 0 )
    {
        strncat( *data, buffer, strlen( buffer ) );
        memset( buffer, '\0', 1024 );
    }

    return 0;
}

int sock_connect( const char *ip, const int port, const int _timeout )
{
    int sockfd;
    struct hostent *host;

    struct timeval timeout;
    memset( &timeout, 0, sizeof( struct timeval ) );
    timeout.tv_sec = _timeout;

    if( ( host = gethostbyname( ip ) ) == NULL )
    {
        fprintf( stderr, "gethostbyname error!\n" );
        return -1;
    }

    if( ( sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
        fprintf( stderr, "socket create error!\n" );
        return -1;
    }

    if( setsockopt( sockfd, SOL_SOCKET, SO_SNDTIMEO, ( char * ) &timeout,
                    sizeof( timeout ) ) == -1 )
    {
        fprintf( stderr, "setsockopt connect timeout error!\n" );
        return -1;
    }

    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons( port );
    serv_addr.sin_addr = *( ( struct in_addr * ) host->h_addr );
    bzero( &( serv_addr.sin_zero ), 8 );

    if( connect( sockfd, ( struct sockaddr * ) &serv_addr,
                 sizeof( struct sockaddr ) ) == -1 )
    {
        if( errno == EINPROGRESS )
        {
            fprintf( stderr, "connect timeout!\n" );
            return -1;
        }

        fprintf( stderr, "connect error!\n" );
        return -1;
    }

    if( setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, ( char * ) &timeout,
                    sizeof( struct timeval ) ) < 0 )
    {
        fprintf( stderr, "setsockopt recv timeout error!\n" );
        sock_close( sockfd );
        return -1;
    }

    struct linger linger;

    linger.l_onoff = 1;
    linger.l_linger = 0;
    if( setsockopt( sockfd, SOL_SOCKET, SO_LINGER, ( char * ) &linger,
                    sizeof( struct linger ) ) < 0 )
    {
        fprintf( stderr, "setsockopt linger error!\n" );
        sock_close( sockfd );
        return -1;
    }

    return sockfd;
}

void sock_close( int sockfd )
{
    close( sockfd );
}

int list_ids( const char *ip, const int port, const char *params,
              char **result )
{
    int sockfd = -1;
    int timeout = TIMEOUT;

    if( ( sockfd = sock_connect( ip, port, timeout ) ) == -1 )
    {
        fprintf( stderr, "cann't connect to cms server(%s:%d)!\n", ip, port );
        return -1;
    }

    if( write( sockfd, NETPOWER, strlen( NETPOWER ) ) == -1 )
    {
        fprintf( stderr, "send signal failed!\n" );
        return -1;
    }

    if( write( sockfd, LIST, strlen( LIST ) ) == -1 )
    {
        fprintf( stderr, "send List command failed\n" );
        return -1;
    }

    if( send_package( sockfd, params, strlen( params ) ) == -1 )
    {
        fprintf( stderr, "send List params(%s) failed!\n", params );
        return -1;
    }

    if( recv_package( sockfd, result ) == -1 )
    {
        fprintf( stderr, "List recv failed!" );
        return -1;
    }

    sock_close( sockfd );

    return 0;
}

int send_ids( const char *ip, const int port, const char *id,
              const char *cmd, char **result )
{
    int sockfd = -1;
    int timeout = TIMEOUT;

    if( ( sockfd = sock_connect( ip, port, timeout ) ) == -1 )
    {
        fprintf( stderr, "cann't connect to cms server(%s:%d)!\n", ip, port );
        return -1;
    }

    if( write( sockfd, NETPOWER, strlen( NETPOWER ) ) == -1 )
    {
        fprintf( stderr, "send signal failed!\n" );
        return -1;
    }

    if( write( sockfd, SEND, strlen( SEND ) ) == -1 )
    {
        fprintf( stderr, "send Send command failed!\n" );
        return -1;
    }

    if( send_package( sockfd, id, strlen( id ) ) == -1 )
    {
        fprintf( stderr, "send ids sid(%s) failed!\n", id );
        return -1;
    }

    if( send_package( sockfd, cmd, strlen( cmd ) ) == -1 )
    {
        fprintf( stderr, "send cmd(%s) failed!\n", cmd );
        return -1;
    }

    if( recv_package( sockfd, result ) == -1 )
    {
        fprintf( stderr, "Send cmd(%s) recv failed!\n", cmd );
        return -1;
    }

    sock_close( sockfd );
    return 0;
}

int send_file( const char *ip, const int port, const char *id,
               const char *content, const char *filename, char **result )
{
    int sockfd = -1;
    int timeout = TIMEOUT;

    if( ( sockfd = sock_connect( ip, port, timeout ) ) == -1 )
    {
        fprintf( stderr, "cann't connect to cms server(%s:%d)!\n", ip, port );
        return -1;
    }

    if( write( sockfd, NETPOWER, strlen( NETPOWER ) ) == -1 )
    {
        fprintf( stderr, "send signal failed!\n" );
        return -1;
    }

    if( write( sockfd, File, strlen( File ) ) == -1 )
    {
        fprintf( stderr, "send File command failed!\n" );
        return -1;
    }

    if( send_package( sockfd, id, strlen( id ) ) == -1 )
    {
        fprintf( stderr, "send ids File(%s) failed!\n", id );
        return -1;
    }

    if( send_package( sockfd, content, strlen( content ) ) == -1 )
    {
        fprintf( stderr, "send content(File:%s) failed!\n", content );
        return -1;
    }

    if( send_package( sockfd, filename, strlen( filename ) ) == -1 )
    {
        fprintf( stderr, "send filename(File:%s) failed!\n", filename );
        return -1;
    }

    if( recv_package( sockfd, result ) == -1 )
    {
        fprintf( stderr, "Send File recv failed!\n" );
        return -1;
    }

    sock_close( sockfd );
    return 0;
}

int down_file( const char *ip, const int port, const char *id,
               const char *filename, char **result )
{
    int sockfd = -1;
    int timeout = TIMEOUT;

    if( ( sockfd = sock_connect( ip, port, timeout ) ) == -1 )
    {
        fprintf( stderr, "cann't connect to cms server(%s:%d)!\n", ip, port );
        return -1;
    }

    if( write( sockfd, NETPOWER, strlen( NETPOWER ) ) == -1 )
    {
        fprintf( stderr, "send signal failed!\n" );
        return -1;
    }

    if( write( sockfd, DOWN, strlen( DOWN ) ) == -1 )
    {
        fprintf( stderr, "send Down command failed!\n" );
        return -1;
    }

    if( send_package( sockfd, id, strlen( id ) ) == -1 )
    {
        fprintf( stderr, "send ids Down(%s) failed!\n", id );
        return -1;
    }

    if( send_package( sockfd, filename, strlen( filename ) ) == -1 )
    {
        fprintf( stderr, "send filename(Down:%s) failed!\n", filename );
        return -1;
    }

    if( recv_package( sockfd, result ) == -1 )
    {
        fprintf( stderr, "Send Down recv failed!\n" );
        return -1;
    }

    sock_close( sockfd );
    return 0;
}
