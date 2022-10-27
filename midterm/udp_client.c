/** 
 * 2021114335 박지원
 * udp_client.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#define BUF_SIZE 1024
// cmd type
#define FILE_NAME_REQ 0
#define FILE_NAME_RES 1
#define FILE_REQ 2
#define FILE_SEND 3
#define FILE_END 4
#define FILE_END_ACK 5
// result values
#define SUCCESS 0
#define FAIL -1

typedef struct {
	int cmd;
	int buf_len; // 실제 전송되는 파일의 크기 저장
	char buf[BUF_SIZE];
	int result; // SUCCESS, FAIL(FILE NOT FOUND 인 경우:-1)
}PACKET;

void error_handling(char *message);

int main(int argc, char* argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	socklen_t serv_addr_size;
	PACKET packet;
	int str_len;
	char filename[BUF_SIZE];	
	int rx_cnt = 0, rx_bytes = 0;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	
	memset(&packet, 0, sizeof(packet));
	printf("Input file name: ");
	scanf("%s", packet.buf);
	strcpy(filename, packet.buf);

	packet.cmd = FILE_NAME_REQ;
	write(sock, &packet, sizeof(packet));
	printf("[Tx] FILE_NAME_REQ(cmd: %d) file_name: %s\n", packet.cmd, packet.buf);

	str_len = read(sock, &packet, sizeof(packet));
	if(str_len == -1)
		error_handling("read() error");
	if(packet.result == FAIL){
		printf("[Rx] FILE_NAME_RES(cmd: %d) %s: File Not Found\n", packet.cmd, filename);
	}else{
		printf("\n");
		while(1){
			memset(&packet, 0, sizeof(packet));
			packet.cmd = FILE_REQ;
			write(sock, &packet, sizeof(packet));

			str_len = read(sock, &packet, sizeof(packet));
			if(str_len == -1)
				error_handling("read() error");
			rx_cnt++;
			rx_bytes += packet.buf_len;
			write(1, packet.buf, packet.buf_len); //stdout

			if(packet.cmd == FILE_END){
				memset(&packet, 0, sizeof(packet));
				packet.cmd = FILE_END_ACK;
				write(sock, &packet, sizeof(packet));
				break;
			}
		}
	}
	printf("\n----------------------------------------\n");
	printf("Total Rx count: %d, bytes: %d\n", rx_cnt, rx_bytes);
	close(sock);
	printf("UDP Client Socket Close!\n");
	printf("----------------------------------------\n");
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
