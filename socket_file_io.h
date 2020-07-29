#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);
int send_file(int socket, const char * filename);
int recv_file(int socket, const char * filename);