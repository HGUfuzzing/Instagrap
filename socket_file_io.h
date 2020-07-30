#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#define BUF_SIZE 1024

void error_handling(char *message);
pid_t my_popen(const char *command, int *read_fd);
int send_file(int socket, const char * filename);
int recv_file(int socket, const char * filename);