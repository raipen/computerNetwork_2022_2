#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 2048

typedef enum{SENDER = 1, RECEIVER} role_t;
char* role_str[] = {"", "Sender", "Receiver"};

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int fd1, fd2;
	char message[BUF_SIZE];
	int str_len, fd_max, fd_num;
	struct sockaddr_in serv_adr;
	fd_set reads, cpy_reads;
	struct timeval timeout;

	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	printf("--------------------------------\n");
	printf("Choose function\n");
	printf("%d. Sender,\t%d: Receiver\n", SENDER, RECEIVER);
	printf("--------------------------------\n");
	printf("=> ");
	int type;
	scanf("%d", &type);
	if(type != SENDER && type != RECEIVER)
	{
		printf("Invalid input\n");
		exit(1);
	}
	printf("File %s Start!\n", role_str[type]);

	
	FD_ZERO(&reads);

	if(type==SENDER){
		fd1 = open("rfc1180.txt", O_RDONLY);
		if(fd1 == -1)
			error_handling("open() error");
		FD_SET(fd1, &reads);
	}

	fd2 = socket(PF_INET, SOCK_STREAM, 0);
	if (fd2 == -1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	if (connect(fd2, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");
	else
		puts("Connected...........");

	write(fd2, role_str[type], strlen(role_str[type]));

	FD_SET(fd2, &reads);
	fd_max = fd2;

	if(type==SENDER)
		printf("fd1: %d, fd2: %d\n", fd1, fd2);
	else
		printf("fd2: %d\n", fd2);

	printf("fd_max: %d\n", fd_max);
	while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
            break;
        if (fd_num == 0)
            continue;

		if(type==SENDER){
			if(FD_ISSET(fd2, &cpy_reads)){
				str_len = read(fd2, message, BUF_SIZE);
				write(1, message, str_len);
				// if(str_len <BUF_SIZE) //자동종료인데 교수님이 ^C로 종료하셔서 주석처리
				// 	break;
			}
			if(FD_ISSET(fd1, &cpy_reads)){
				sleep(1); //이거 write 뒤에 넣으면 서버한테 아까 "Sender"보낸거랑 이어서 빠르게 보내서 서버측에서 뒤에꺼랑 합쳐서 받아버림;;
				str_len = read(fd1, message, BUF_SIZE);
				if(str_len == 0){
					FD_CLR(fd1, &reads);
					close(fd1);
				}
				write(fd2, message, str_len);
			}
		}
		else{
			if(FD_ISSET(fd2, &cpy_reads)){
				str_len = read(fd2, message, BUF_SIZE);
				write(1, message, str_len);
				write(fd2, message, str_len);
				// if(str_len < BUF_SIZE) //자동종료인데 교수님이 ^C로 종료하셔서 주석처리
				// 	break;
			}
		}
	}
	FD_CLR(fd2, &reads);
	close(fd2);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}