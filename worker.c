#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "socket_file_io.h"
#define BUF_SIZE 1024

int main(int argc, char * argv[]) {
    
    char buf[BUF_SIZE];
    int serv_sock;
    int clnt_sock;

    //sockaddr_in�� ���� �ּ��� Ʋ�� �������ִ� ����ü�� AF_INET�� ��� ���
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;   //accept�Լ����� ����.
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    if(argc != 3) {
        printf("Usage : %s <Port> <Working Dir>\n", argv[0]);
        exit(1);
    }

    //TCPd�����������̰� ipv4 �������� ���� ������ ����
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket error");
    
    //�ּҸ� �ʱ�ȭ�� �� IP�ּҿ� ��Ʈ ����
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;                   //Ÿ��: ipv4
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);    //ip�ּ�
    serv_addr.sin_port=htons(atoi(argv[1]));         //port

    //���ϰ� ���� �ּҸ� ���ε�
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind error");

    //���� ��⿭ 50�� ����
    if(listen(serv_sock, 50)==-1)
        error_handling("listen error");
    printf("listening to port %s\n", argv[1]);    

    
    while(1) {
         FILE * fp;

         
        //Ŭ���̾�Ʈ�κ��� ��û�� ���� ���� ����
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);   //���ŷ!
        if(clnt_sock==-1)
                error_handling("accept error");
        printf("Connection Request from Client [IP:%s, Port:%d] has been Accepted\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

        snprintf(buf, sizeof(buf), "%s/new.c", argv[2]);
        //������ ���������� �Ǿ����� file �ޱ�
        recv_file(clnt_sock, buf);

        snprintf(buf, sizeof(buf), "cp ./%s/new.c ./copy.c", argv[2]);
        fp = popen(buf, "r");
        pclose(fp);
        
       

        //���� (.c)���� ������ �� �����ϱ�
        snprintf(buf, sizeof(buf), "gcc -w %s/new.c 2> /dev/stdout", argv[2]);
        
        if((fp = popen(buf, "r")) == NULL)
            error_handling("popen() error (1)");
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), fp);
        
        //Compile error
        if(strlen(buf) != 0) {
            buf[0] = 0x33;
            send(clnt_sock, buf, 1, 0);    //compile error ���� �˷��ֱ� (Compile error�� buf[0] != 0x0)
            printf("********************\n");
            printf("Compile error.\n");
            printf("********************\n");
            while(fgets(buf, sizeof(buf), fp) != NULL)
                printf("%s\n", buf);
            printf("----------------------\n");

            

            //�����ϱ�.
            snprintf(buf, sizeof(buf), "rm -rf ./%s/* ./a.out", argv[2]);
            fp = popen(buf, "r");
            pclose(fp);
            continue;
        }
        buf[0] = 0xff;
        send(clnt_sock, buf, 1, 0);    //compile error ���� �˷��ֱ� (Compile error�� buf[0] != 0x0)
        pclose(fp);

        for(int i = 1; i <= 5; i++) {
            int read_fd;
            pid_t child;
            printf("%d !\n", i);
            //input file �ް�
            snprintf(buf, sizeof(buf), "./%s/%d.in", argv[2], i);
            recv_file(clnt_sock, buf);

            //��������
            snprintf(buf, sizeof(buf), "./a.out < ./%s/%d.in > ./%s/%d.out && echo done", argv[2], i, argv[2], i);
            if((child = my_popen(buf, &read_fd)) == -1) 
                error_handling("my_popen() error (2)");

            //�ִ� 3secs ��ٸ���
            int result;
            fd_set fd_status;
            struct timeval timeout;
            
            FD_ZERO(&fd_status);
            FD_SET(read_fd, &fd_status);
            timeout.tv_sec = 3; //3�� Ÿ�Ӿƿ�
            timeout.tv_usec = 0;
            result = select(read_fd + 1, &fd_status, 0, 0, &timeout);
            printf("readfd : %d\n", read_fd);
            if(result == -1) {
                error_handling("unintended disconnect!");
            }
            else if(result == 0) {
                //timeout�� �Ͼ�ٴ� ����� �˸�
                buf[0] = 0x33;
                send(clnt_sock, buf, 1, 0);
                printf("send Timeout for testcase %d\n", i);
                kill(child, SIGINT);
            }
            else {
                //timeout�� �� �Ͼ�ٴ� ����� �˸�
                buf[0] = 0x00;
                send(clnt_sock, buf, 1, 0);

                //output file ����.
                snprintf(buf, sizeof(buf), "./%s/%d.out", argv[2], i);
                send_file(clnt_sock, buf);
                
            }

            close(read_fd);
        }

        //������ ���� ����.(worker space�� ���ϵ� ����)
        snprintf(buf, sizeof(buf), "rm -rf ./%s/* ./a.out", argv[2]);
        fp = popen(buf, "r");
        pclose(fp);
        
        close(clnt_sock);
    }

    //���� �ݱ�
    
    close(serv_sock);
    puts("FINISHED");
    return 0;
}