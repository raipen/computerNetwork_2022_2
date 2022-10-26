/** 
 * 2021114335 박지원
 * hw2_server.c
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
	int serv_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size;
	PACKET packet;
	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");
	
	//클라이언트로부터 파일 이름 받기
	clnt_addr_size=sizeof(clnt_addr);
	recvfrom(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	printf("[Rx] cmd: %d, file_name: %s\n", packet.cmd, packet.buf);

	//파일 열기
	int fd = open(packet.buf, O_RDONLY);
	int tx_cnt = 0, tx_bytes = 0;
	if(fd == -1){
		memset(&packet, 0, sizeof(packet));
		packet.cmd = FILE_NOT_FOUND;
		sendto(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
		printf("[Tx] cmd: %d, %s: File Not Found\n", packet.cmd, packet.buf);
	}
	else{
		tx_cnt = 0;
		tx_bytes = 0;
		packet.cmd = FILE_RES;
		while(1){
			memset(&packet.buf, 0, sizeof(packet.buf));
			packet.buf_len = read(fd, packet.buf, BUF_SIZE);
			tx_cnt++;
			tx_bytes += packet.buf_len;
			if(packet.buf_len < BUF_SIZE){
				packet.cmd = FILE_END;
				sendto(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
				printf("[Tx] cmd: %d, len: %d, total_tx_cnt: %d, total_tx_bytes: %d\n", packet.cmd, packet.buf_len, tx_cnt, tx_bytes);
				break;
			}
			sendto(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
			printf("[Tx] cmd: %d, len: %d, total_tx_cnt: %d, total_tx_bytes: %d\n", packet.cmd, packet.buf_len, tx_cnt, tx_bytes);
		}
		//파일 전송 완료 후 클라이언트로부터 ACK 받기
		clnt_addr_size=sizeof(clnt_addr);
		recvfrom(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		printf("[Rx] cmd: %d, FILE_END_ACK\n", packet.cmd);
	}

	printf("----------------------------------------\n");
	printf("Total Tx count: %d, bytes: %d\n", tx_cnt, tx_bytes);
	close(serv_sock);
	printf("UDP Server Socket Close!\n");
	printf("----------------------------------------\n");

	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}