/** 2021114335 박지원
 * hw4_server.c
 * 
 * 서버는 클라이언트에게 전송 받은 파일이름을 확인해서 파일이 있는 경우, 해당파일을 전송함
 * 파일이 서버에 없는 경우, buf_len에 -1과 buf에 "File Not Found"를 클라이언트에게 전송하고 소켓 종료
 * TCP 데이터 송수신 과정(SEQ, ACK 내용)을 반드시 화면 출력해야 됨
 * 서버의 시작 SEQ번호: 1000
 * 파일 입출력 함수는 저수준(read, write)함수 사용
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#define BUF_SIZE 100
#define SEQ_START 1000

typedef struct
{
    int seq;            // SEQ number
    int ack;            // ACK number
    int buf_len;        // File read/write bytes
    char buf[BUF_SIZE]; // 파일이름 또는 파일 내용 전송
} Packet;

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    Packet packet;
    int read_cnt;
    char fileName[BUF_SIZE];

    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("-----------------------------------------\n");
	printf(" File Transmission Server \n");
	printf("-----------------------------------------\n");

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1)
        error_handling("accept() error");

    memset(&packet, 0, sizeof(packet));

    if (read(clnt_sock, &packet, sizeof(packet)) == -1)
        error_handling("read() error");

    strcpy(fileName, packet.buf);

    int fin = open(fileName, O_RDONLY);
    if (fin != -1){
        printf("[Server] sending %s\n\n", fileName);
        int totalSize = 0;
        packet.seq = SEQ_START;
        while ((packet.buf_len = read(fin, packet.buf, BUF_SIZE)) >= 0){
            totalSize += packet.buf_len;
            printf("[Server] Tx: SEQ: %d, %d byte data\n", packet.seq, packet.buf_len);
            write(clnt_sock, &packet, sizeof(packet));
            
            if (packet.buf_len < BUF_SIZE)
                break;
            
            if (read(clnt_sock, &packet, sizeof(packet)) == -1)
                error_handling("read() error");
            printf("[Server] Rx: ACK: %d\n\n", packet.ack);
            packet.seq = packet.ack;
        }
        
        printf("%s sent (%d Bytes)\n",fileName,totalSize);
        close(fin);
    }
    // 파일이 서버에 없는 경우, "File Not Found"를 클라이언트에게 전송하고 소켓 종료
    else{
        printf("%s File Not Found\n", packet.buf);

        strcpy(packet.buf,"File Not Found");
        packet.buf_len = -1;
        packet.seq = SEQ_START;
        
        write(clnt_sock, &packet, sizeof(packet));
    }

    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
