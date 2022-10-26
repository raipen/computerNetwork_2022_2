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
	int sample;
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

	/* 서버에 패킷 보내기
	if(write(sock, &packet, sizeof(packet))==-1)
		error_handling("write() error!");
	*/

	/* 서버로부터 패킷 받기
	if(read(sock, &packet, sizeof(packet))==-1)
		error_handling("read() error!");
	*/

	close(sock);
	printf("Client socket close and exit\n");
	return 0;
}

void error_handling(char *message){
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
