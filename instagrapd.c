#include <stdio.h>
#include <stdlib.h> //for atoi()
#include <string.h> // for memset() ��
#include <unistd.h> //sockadd_in, read, write ��
#include <arpa/inet.h> //htonol, htons, INDDR_ANY, sockaddr_in ��
#include <sys/socket.h>
#include "socket_file_io.h"
#define RECEIVED_FILE_NAME "received.c"
int main(int argc, char * argv[]) {
    
    FILE * fp;
    int to_worker_fd;
    struct sockaddr_in worker_addr;
    char buf[BUF_SIZE];

    if(argc != 4) {
        //printf("Usage : %s <ip address> <port> <filename> <testcase directory>\n", argv[0]);
        printf("Usage : %s <Port> <IP>:<WPort> <Dir>\n", argv[0]);
        exit(1);
    }
    
    /************* setting as client (for Worker���� ����) ****************/

    //Worker�� ip�� port �Ľ�.
    char str_wport[10];
    char str_wip[20];
    char * ptr = strtok(argv[2], ":");
    strncpy(str_wip, ptr, sizeof(str_wip));
    ptr = strtok(NULL, ":");
    strncpy(str_wport, ptr, sizeof(str_wport));

    
    
    
    /************* setting as server (for Submitter���� ����) ****************/
    int listen_fd, conn_fd;
    struct sockaddr_in serv_addr;

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error_handling("socket error");
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    if(bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error_handling("bind error");
    if(listen(listen_fd, 50) == -1) 
        error_handling("listen error");
    printf("listening to port %s..\n", argv[1]);

    

    /*
        * ���� iterative server�� ����.
        * ���� Concurrent server�� ������ �� (thread ���.)
    */
    while(1) {
        to_worker_fd = socket(PF_INET, SOCK_STREAM, 0);
        if(to_worker_fd == -1) 
            error_handling("socket error");
        //Worker���� connect ��û
        //���ڷ� ���� �����ּ� ������ ����
        memset(&worker_addr, 0, sizeof(worker_addr));
        worker_addr.sin_family = AF_INET;  //�����ּ�ü��� ipv4.
        worker_addr.sin_addr.s_addr = inet_addr(str_wip); //�����ּ�(ip)����
        worker_addr.sin_port = htons(atoi(str_wport));  //���� ��Ʈ��ȣ ����
        if(connect(to_worker_fd, (struct sockaddr*)&worker_addr, sizeof(worker_addr)) == -1)
            error_handling("connect to Worker error");


        char user_id[20];
        if((conn_fd = accept(listen_fd, (struct sockaddr *) NULL, NULL)) == -1)
            error_handling("accept error");
        printf("Accept a connection request..!\n");
        
        //ping
        send(conn_fd, buf, 1, 0);

        recv(conn_fd, buf, 9, 0);           //�ٲٱ�! 9 --> sizeof(buf)��
        strncpy(user_id, buf, sizeof(user_id));
        printf("user_id : %s\n", user_id);

        //submitter�� ���� FILE �ޱ�
        recv_file(conn_fd, RECEIVED_FILE_NAME);
        
        /************* worker���� �����ְ� ��� �ޱ� ****************/

        //worker���� .c ���� ����
        send_file(to_worker_fd, RECEIVED_FILE_NAME);

        //������ ���� ���� worker���� �޾Ƽ� submitter���� �߼�
        recv(to_worker_fd, buf, 1, 0);
        send(conn_fd, buf, 1, 0);
        if(buf[0] == 0x33) {
            printf("compile error\n"); 
            close(to_worker_fd);   
            close(conn_fd); //conn socket �ݰ�
            continue;       //���� submitter ���� �ޱ�
        }
        
        printf("compile success!\n");
        
        //testcase �ϳ� send, �ϳ� recv         (5���� ����)
        for(int i = 1; i <= 5; i++) {
            snprintf(buf, sizeof(buf), "%s/%d.in", argv[3], i);
            
            send_file(to_worker_fd, buf);

            //timeout ���� Ȯ��
            recv(to_worker_fd, buf, 1, 0);
            if(buf[0] == 0x33) {
                snprintf(buf, sizeof(buf), "Tastcase %d.\tTime Limit Exceeded.", i);
                send(conn_fd, buf, strlen(buf) + 1, 0);
                continue;
            }
            printf("timeout is not occured %X\n", buf[0]);

            //i.out ��� �ް�
            snprintf(buf, sizeof(buf), "%s/%d.recv.out", argv[3], i);
            recv_file(to_worker_fd, buf);

            //���� �´��� diff ��, �� TC�� ���� ��� ���
            snprintf(buf, sizeof(buf), "diff %s/%d.recv.out %s/%d.out", argv[3], i, argv[3], i);
            if((fp = popen(buf, "r")) == NULL)
                error_handling("popen() error");
            memset(buf, 0, sizeof(buf));
            fgets(buf, sizeof(buf), fp);
            if(!strlen(buf)) {
                printf("Tastcase %d.\tCorrect!\n", i);
                snprintf(buf, sizeof(buf), "Tastcase %d.\tCorrect Answer!", i);
                
            }
            else {
                printf("Tastcase %d.\tWrong.\n", i);
                snprintf(buf, sizeof(buf), "Tastcase %d.\tWrong Answer.", i);
            }
            //Submitter���� ���� testcase�� ���� ��� ����.
            send(conn_fd, buf, strlen(buf) + 1, 0);
            pclose(fp);
        }

        //������ ���� ���� (*.recv.out)
        snprintf(buf, sizeof(buf), "rm -rf %s/*.recv.out", argv[3]);
        fp = popen(buf, "r");
        pclose(fp);


        close(conn_fd);
        close(to_worker_fd);
        printf("Done.\n");
    }


    
    puts("FINISHED");
    
    
    return 0;
}


