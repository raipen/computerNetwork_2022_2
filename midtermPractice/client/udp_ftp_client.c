/** 
 * 2021114335 박지원
 * hw2_client.c
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
#define FILE_REQ 0
#define FILE_RES 1
#define FILE_END 2
#define FILE_END_ACK 3
#define FILE_NOT_FOUND 4

typedef struct {
	int cmd;
	int buf_len; // 실제 전송되는 파일의 크기 저장
	char buf[BUF_SIZE];
}PACKET;

void error_handling(char *message);

int main(int argc, char* argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	socklen_t serv_addr_size;
	PACKET packet;
	int str_len;
	char filename[BUF_SIZE];
	
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

	packet.cmd = FILE_REQ;
	write(sock, &packet, sizeof(packet));
	printf("[Tx] cmd: %d, file_name: %s\n", packet.cmd, packet.buf);

	int isFirst = 1;
	int fd;
	int rx_cnt = 0, rx_bytes = 0;
	while(1){
		memset(&packet, 0, sizeof(packet));
		str_len = read(sock, &packet, sizeof(packet));
		if(str_len == -1)
			error_handling("read() error");

		if(isFirst){
			if(packet.cmd == FILE_NOT_FOUND){
				printf("[Rx] cmd: %d, %s: File Not Found\n", packet.cmd, filename);
				break;
			}
			fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if(fd == -1)
				error_handling("open() error");
			isFirst = 0;
		}
		rx_cnt++;
		rx_bytes += packet.buf_len;
		printf("[Rx] cmd: %d, len: %d, total_rx_cnt: %d, total_rx_bytes: %d\n", packet.cmd, packet.buf_len, rx_cnt, rx_bytes);
		write(fd, packet.buf, packet.buf_len);
		if(packet.cmd == FILE_END){
			memset(&packet, 0, sizeof(packet));
			packet.cmd = FILE_END_ACK;
			write(sock, &packet, sizeof(packet));
			printf("[Tx] cmd: %d, FILE_END_ACK\n", packet.cmd);
			break;
		}
	}
	printf("----------------------------------------\n");
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
