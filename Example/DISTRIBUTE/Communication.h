#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

void init_result( char **result );
void destroy_result( char **result );
int sock_connect( const char *ip, const int port, const int timeout );
void sock_close( int sockfd );
int send_package( int sockfd, const char *data, int size );
int recv_package( int sockfd, char **result );
int list_ids( const char *ip, const int port, const char *params,
              char **result );
int send_ids( const char *ip, const int port, const char *id, const char *cmd,
              char **result );
int send_file( const char *ip, const int port, const char *id,
               const char *content, const char *filename, char **result );
int down_file( const char *ip, const int port, const char *id,
               const char *filename, char **result );

#endif                          /*COMMUNICATION_H */
