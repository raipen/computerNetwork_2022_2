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
	int sample;
} PACKET;

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
	
	/* 클라이언트로부터 패킷 받기
	clnt_addr_size=sizeof(clnt_addr);
	recvfrom(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	*/

	/* 클라이언트에 패킷 보내기
	sendto(serv_sock, &packet, sizeof(packet), 0, (struct sockaddr*)&clnt_addr, clnt_addr_size);
	*/

	close(serv_sock);
	printf("Server socket close and exit\n");
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}