#include "socket_file_io.h"

pid_t 
my_popen(const char *command, int *read_fd)
{
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) < 0)
        return -1;

    pid = fork();

    if (pid < 0)
        error_handling("failed to fork()");
    else if (pid == 0)
    {   
        dup2(pipefd[1], 1);
        close(pipefd[0]);
        close(pipefd[1]);
        execl("/bin/sh", "sh", "-c", command, NULL);
        exit(1);
    }
    *read_fd = pipefd[0];
    return pid;
}


void 
error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}




int 
send_file(int socket, const char * filename) {
    /*
        send할 파일의 size를 먼저 보냄.
    */
    FILE * fp;
    int file_size = 0;
    int buffer_num = 0;
    int total_send_bytes = 0;
    int send_bytes;
    int total_buffer_num;
    
    char buf[BUF_SIZE];
    if((fp = fopen(filename, "rb")) == NULL) 
        error_handling("File not Exist");

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    total_buffer_num = (file_size - 1) / sizeof(buf) + 1;
    fseek(fp, 0, SEEK_SET);
    snprintf(buf, sizeof(buf), "%d", file_size);    

    send_bytes = send(socket, buf, sizeof(buf), 0);  //input file 사이즈 보내기

    buffer_num = 0;
    total_send_bytes = 0;

    while((send_bytes = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
        send(socket, buf, send_bytes, 0);
        buffer_num++;
        total_send_bytes += send_bytes;
        printf("%s progress: %d/%dByte(s) [%d%%] (sending)\n", filename, total_send_bytes, file_size, buffer_num * 100 / total_buffer_num);
    }
    fclose(fp);
    return 0;
}


int 
recv_file(int socket, const char * filename) {
    /*
        receive할 파일의 size를 먼저 보냄.
    */
    FILE * fp;
    fp = fopen(filename, "wb");

    char buf[1024];
    int read_bytes = 0, tmp = 0;
    int file_size;
    
    recv(socket, buf, sizeof(buf), 0);
    file_size = atoi(buf);

    while(read_bytes < file_size) {
        tmp = recv(socket, buf, sizeof(buf), 0);
        read_bytes += tmp;
        fwrite(buf, sizeof(char), tmp, fp);
        printf("%s progress: %d/%dByte(s) [%d%%] (receiving)\n", filename , read_bytes, file_size, read_bytes * 100 / file_size);
    }
    fclose(fp);
    
    snprintf(buf, sizeof(buf), "cp ./%s ./copy.c", filename);
    fp = popen(buf, "r");
    pclose(fp);
    
    return 0;
}

