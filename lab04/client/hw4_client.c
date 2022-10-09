/** 2021114335 박지원
 * hw4_client.c
 *
 * 사용자로부터 파일 이름을 입력받아 서버에 전송
 * 서버는 클라이언트에게 전송 받은 파일이름을 확인해서 파일이 있는 경우, 해당파일을 전송함
 * 파일이 서버에 없는 경우, buf_len에 -1과 buf에 "File Not Found"를 클라이언트에게 전송하고 소켓 종료
 * TCP 데이터 송수신 과정(SEQ, ACK 내용)을 반드시 화면 출력해야 됨
 * 파일 입출력 함수는 저수준(read, write)함수 사용
 * ack는 (seq + 전송 받은 바이트수 + 1)로 설정
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

typedef struct{
    int seq;            // SEQ number
    int ack;            // ACK number
    int buf_len;        // File read/write bytes
    char buf[BUF_SIZE]; // 파일이름 또는 파일 내용 전송
} Packet;

void error_handling(char *message);

int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in serv_addr;
    Packet packet;
    int read_cnt;
    char fileName[BUF_SIZE];
    int fd;

    if(argc!=3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock=socket(PF_INET, SOCK_STREAM, 0);
    if(sock==-1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling("connect() error!");

    // 파일 이름 입력
    printf("Input file name: ");
    scanf("%s", packet.buf);
    strcpy(fileName, packet.buf);
    packet.buf_len = strlen(packet.buf);

    // 파일 이름 전송
    write(sock, &packet, sizeof(packet));
    printf("[Client] request %s\n\n", packet.buf);

    // 파일 생성
    fd = open(packet.buf, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    //buf_len이 -1이면 파일이 없는 경우
    //buf_len이 0 이상, BUF_SIZE 미만이면 파일의 끝에 도달한 경우
    int totalSize = 0;
    while(1){
        read_cnt = read(sock, &packet, sizeof(packet));
        if(packet.buf_len <0){
            printf("%s\n", packet.buf);
            break;
        }
        printf("[Client] RX SEQ: %d, len: %d bytes\n", packet.seq, packet.buf_len);
        write(fd, packet.buf, packet.buf_len);
        totalSize += packet.buf_len;

        if(packet.buf_len < BUF_SIZE){
            printf("%s received (%d Bytes)\n", fileName, totalSize);
            break;
        }

        packet.ack = packet.seq + packet.buf_len + 1;
        write(sock, &packet, sizeof(packet));
        printf(" [Client] TX ACK: %d\n\n", packet.ack);
    }
    close(sock);
    close(fd);
    if(packet.buf_len < 0)
        remove(fileName);
    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}