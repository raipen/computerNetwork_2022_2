/** 2021114335 박지원
서버는 클라이언트에게 전송 받은 파일이름을 확인해서 파일이 있는 경우, 해당파일을 전송함
파일이 서버에 없는 경우, "File Not Found"를 클라이언트에게 전송하고 소켓 종료
TCP 데이터 송수신 과정(SEQ, ACK 내용)을 반드시 화면 출력해야 됨
서버의 시작 SEQ번호: 1000
파일 입출력 함수는 저수준(read, write)함수 사용
ack는 seq+전송된바이트수+1로 설정
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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
    int seq = SEQ_START;
    int ack = 0;
    int read_cnt;
    FILE *fp;

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
    // 클라이언트로부터 파일이름을 전송받음
    read_cnt = read(clnt_sock, &packet, sizeof(packet));
    if (read_cnt == -1)
        error_handling("read() error");
    else if (read_cnt == 0)
        error_handling("read() error: no file name");

    // 파일이름을 확인해서 파일이 있는 경우, 해당파일을 전송함
    int fin = open(packet.buf, O_RDONLY);
    if (fin != -1)
    {
        printf("[Server] sending %s\n\n", packet.buf);
        // 파일 내용을 전송
        while ((packet.buf_len = read(fin, buf, BUF_SIZE)) > 0)
        {
            // 클라이언트에게 전송할 SEQ, ACK 설정
            packet.seq = seq;
            packet.ack = packet.seq + packet.buf_len + 1;
            // SEQ, ACK 출력
            printf("[Server] Tx: SEQ: %d, %d byte data\n", packet.seq, packet.buf_len);

            // 파일 내용을 전송
            write(clnt_sock, &packet, sizeof(packet));

        }
        fclose(fp);
    }
    // 파일이 서버에 없는 경우, "File Not Found"를 클라이언트에게 전송하고 소켓 종료
    else
    {
        // SEQ, ACK 출력
        printf("%s File Not Found", packet.buf);

        strcpy(packet.buf,"File Not Found");
        packet.buf_len = strlen(packet.buf);
        packet.seq = seq;
        packet.ack = seq + packet.buf_len + 1;
        
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
