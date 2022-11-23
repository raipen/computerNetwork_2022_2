#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define TTL 64
#define BUF_SIZE 120
#define LINE_SIZE 100
void error_handling(char *message);

int main(int argc, char *argv[])
{
    if(argc != 4) {
        printf("Usage : %s <GroupIP> <PORT> <name>\n", argv[0]);
    }

    int send_sock;
    int recv_sock;
    struct sockaddr_in mul_adr;
    struct sockaddr_in adr;
    struct ip_mreq join_adr;
    int time_live = TTL;
    int str_len;
    char buf[BUF_SIZE];
    char line[LINE_SIZE];

    send_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&mul_adr, 0, sizeof(mul_adr));
    mul_adr.sin_family = AF_INET;
    mul_adr.sin_addr.s_addr = inet_addr(argv[1]);
    mul_adr.sin_port=htons(atoi(argv[2]));

    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));


    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    int opt = 1;
    setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[2]));

    if(bind(recv_sock, (struct sockaddr*)&adr, sizeof(adr)) == -1)
        error_handling("bind() error");

    join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);    // 가입할 멀티캐스트 그룹 주소 
    join_adr.imr_interface.s_addr = htonl(INADDR_ANY);      // 멀티캐스트 그룹에 가입할 자신의 IP 주소
    // 멀티캐스트 그룹 가입 
    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));

    pid_t pid = fork();
    switch(pid){
        case -1:
            error_handling("fork() error");
            break;
        case 0:
            while(1)
            {
                str_len = recvfrom(recv_sock, buf, BUF_SIZE-1, 0, NULL, 0);
                if(str_len < 0)
                    break;
                buf[str_len] = 0;
                printf("Received Message: %s", buf);
            }
            break;
        default:
            while(1)
            {
                fgets(line, LINE_SIZE, stdin);
                if(!strcmp(line, "q\n") || !strcmp(line, "Q\n"))
                    break;
                sprintf(buf, "[%s]: %s", argv[3], line);
                sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
            }
            setsockopt(recv_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
            printf("Leave multicast group\n");
            kill(pid, SIGKILL);
            break;
    }
    close(recv_sock);
    close(send_sock);
    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
