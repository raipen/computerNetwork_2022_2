#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#define BASIC_BUF 10
#define STANDARD_BUF 100
#define PREMIUM_BUF 1000
#define MAX_CLNT 256

#define MAX_SIZE PREMIUM_BUF
typedef struct {
    int command;
    int type;
    char buf[MAX_SIZE];
    int len;
} PACKET;
typedef enum{
    FILE_REQ = 0,
    FILE_SENDING,
    FILE_END,
    FILE_END_ACK
} COMMAND;
typedef enum{
    BASIC = 0,
    STANDARD,
    PREMIUM
} TYPE;
int bufSize[3] = {BASIC_BUF, STANDARD_BUF, PREMIUM_BUF};
char* typeString[3] = {"Basic", "Standard", "Premium"};

void * handle_clnt(void * arg);
void error_handling(char * msg);

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

    printf("-----------------------\n");
    printf(" K-OTT Service Server  \n");
    printf("-----------------------\n");
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s , clnt_sock=%d\n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
	}
	close(serv_sock);
	return 0;
}

void * handle_clnt(void * arg)
{
    int clnt_sock=*((int*)arg);
    int str_len=0, i,total=0;
    PACKET packet;
    
    str_len=read(clnt_sock, &packet, sizeof(packet));
    //패킷 내용이 FILE_REQ가 아닌 경우 무시
    if(packet.command != FILE_REQ)
        return NULL;

    //패킷 내용이 FILE_REQ인 경우 파일 전송 시작
    //hw06.mp4 파일을 열고 파일의 내용을 읽어서 클라이언트에게 전송
    int fd = open("hw06.mp4", O_RDONLY);
    if(fd == -1)
    {
        printf("File open error!\n");
        return NULL;
    }

    packet.command = FILE_SENDING; //파일 전송 중에는 FILE_SENDING 패킷을 전송
    while(1)
    {
        packet.len = read(fd, packet.buf, bufSize[packet.type]);
        //가입자 유형에 따라 한번에 전송하는 버퍼 크기가 다름
        if(packet.len < bufSize[packet.type])
            packet.command = FILE_END; //파일 전송 마지막에 FILE_END 패킷을 전송
        DEBUG_PRINT("%d %s %d\n", packet.len, typeString[packet.type], packet.command);
        write(clnt_sock, &packet, sizeof(packet));
        total += packet.len;
        if(packet.command == FILE_END)
            break;
    }
    close(fd);

    
    //전체 파일 전송 바이트 수 출력 및 가입자 유형 출력
    printf("Total Tx Bytes: %d to client %d(%s)\n", total, clnt_sock, typeString[packet.type]);

    read(clnt_sock, &packet, sizeof(packet));
    if(packet.command != FILE_END_ACK)
        return NULL;
    //FILE_END_ACK 패킷을 수신하면 파일 전송을 종료
    printf("[Rx] FILE_END_ACK from client %d => clnt_sock: %d closed.\n", clnt_sock, clnt_sock);

    //파일 전송이 끝나면 클라이언트 소켓을 닫고
    //clnt_socks 배열에서 해당 클라이언트 소켓을 제거
    pthread_mutex_lock(&mutx);
    for(i=0; i<clnt_cnt; i++)   // remove disconnected client
    {
        if(clnt_sock==clnt_socks[i])
        {
            while(i++<clnt_cnt-1)
                clnt_socks[i]=clnt_socks[i+1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}
