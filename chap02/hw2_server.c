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

typedef struct {
	int cmd; // 0: request, 1: response, 2: quit
	char addr[20]; // dotted-decimal address저장(20 bytes)
	struct in_addr iaddr; // inet_aton() result 저장
	int result; // 0: Error, 1: Success
} PACKET;

void error_handling(char *message);

int main(int argc, char* argv[]){
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size;
	PACKET packet;
	
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");
	
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	printf("---------------------------\n");
	printf(" Address Conversion Server \n");
	printf("---------------------------\n");
	
	clnt_addr_size=sizeof(clnt_addr);
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");

	while(1){	
		if(read(clnt_sock, &packet, sizeof(packet))==-1)
			error_handling("read() error!");

		if(packet.cmd==2){
			printf("[Rx] QUIT message received\n");
			break;
		}
		if(packet.cmd==1)
			continue;
		printf("[Rx] Received Dotted-Decimal Address: %s\n", packet.addr);
		
		packet.cmd=1;
		if(inet_aton(packet.addr, &packet.iaddr)==0){
			packet.result=0;
			write(clnt_sock, &packet, sizeof(packet));
			printf("[Tx] Address conversion fail:(%s)\n\n", packet.addr);
			continue;
		}
		packet.result=1;
		printf("inet_aton(%s) -> %#x\n", packet.addr, packet.iaddr.s_addr);
		write(clnt_sock, &packet, sizeof(packet));
		printf("[Tx] cmd: %d, iaddr:(%#x), result: %d\n\n", packet.cmd, packet.iaddr.s_addr, packet.result);
	}

	close(clnt_sock);
	close(serv_sock);
	printf("Server socket close and exit\n");
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}