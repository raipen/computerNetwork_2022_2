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

typedef struct {
	int cmd; // 0: request, 1: response, 2: quit
	char addr[20]; // dotted-decimal address저장(20 bytes)
	struct in_addr iaddr; // inet_aton() result 저장
	int result; // 0: Error, 1: Success
} PACKET;

void error_handling(char *message);

int main(int argc, char* argv[]){
	int sock;
	struct sockaddr_in serv_addr;
	PACKET packet;
	int str_len;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
		
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
		error_handling("connect() error!");

	while(1){
		printf("Input dotted-decimal address: ");
		scanf("%s", packet.addr);
		if(strcmp(packet.addr, "quit") == 0){
			packet.cmd = 2;
			write(sock, &packet, sizeof(packet));
			printf("[Tx] cmd: 2(QUIT)\n");
			break;
		}
		packet.cmd = 0;
		write(sock, &packet, sizeof(packet));
		printf("[Tx] cmd: %d, arrd: %s\n", packet.cmd, packet.addr);
		
		str_len=read(sock, &packet, sizeof(packet));
		if(str_len==-1)
			error_handling("read() error!");
		
		if(packet.result == 1)
			printf("[Rx] cmd: %d, Address conversion: %#x (result: %d)\n\n", packet.cmd, packet.iaddr.s_addr, packet.result);
		else
			printf("[Rx] cmd: %d, Address conversion fail! (result: %d)\n\n", packet.cmd, packet.result);
	}
	close(sock);
	printf("Client socket close and exit\n");
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
