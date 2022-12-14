#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_CLNT 256

#define ROWS 2
#define COLS 10

typedef enum{
    INQUIRY = 1,
    RESERVATION,
    CANCELLATION,
    QUIT
} COMMAND;
typedef enum{
    SUCCESS = 0, //예약 현황 조회 성공, 예약 성공, 예약 취소 성공
    WRONG_SEATNO, //잘못된 좌석 번호
    ALREADY_RESERVED, //이미 예약된 좌석
    NOT_RESERVED, //예약되지 않은 좌석
    INVALID_BOOKER //예약자가 아님
} RESULT;

typedef struct {
int command;
int seatno;
int seats[ROWS][COLS];
int result;
} RES_PACKET;

typedef struct {
int command;
int seatno;
} REQ_PACKET;

void * handle_clnt(void * arg);
void copySeats(int des[ROWS][COLS]);
void error_handling(char * msg);
void sendCurrentSeats(int clnt_sock,RES_PACKET * res_packet);
RESULT reserveSeat(int seatno,int booker);
RESULT cancelSeat(int seatno,int booker);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
pthread_mutex_t seatMutx;

int seats[ROWS][COLS];

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
	int str_len=0, i;
	RES_PACKET resPacket;
	REQ_PACKET reqPacket;

    int end_flag = 0;
    while(!end_flag){
	    read(clnt_sock, &reqPacket, sizeof(reqPacket));
        memset(&resPacket, 0, sizeof(resPacket));
        resPacket.command = reqPacket.command;
        switch(reqPacket.command){
            case INQUIRY:
                resPacket.result = SUCCESS;
                break;
            case RESERVATION:
                resPacket.result = reserveSeat(reqPacket.seatno,clnt_sock); //returns SUCCESS(0) or WRONG_SEATNO(-1) or ALREADY_RESERVED(-2)
                break;
            case CANCELLATION:
                resPacket.result = cancelSeat(reqPacket.seatno,clnt_sock); //returns SUCCESS(0) or WRONG_SEATNO(-1) or NOT_RESERVED(-3) or INVALID_BOOKER(-4)
                break;
            case QUIT:
            default:
                end_flag = 1;
                break;
        }
        sendCurrentSeats(clnt_sock,&resPacket);
    }
	
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
		    printf("Client removed: clnt_sock=%d, i=%d\n", clnt_sock, i);
			while(i < clnt_cnt)
			{
				clnt_socks[i]=clnt_socks[i+1];
				i++; // 클라이언트 종료시점에 무한루프 발생 문제점 수정
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void copySeats(int des[ROWS][COLS]){
    int i, j;
    pthread_mutex_lock(&seatMutx);
    for(i=0; i<ROWS; i++){
        for(j=0; j<COLS; j++){
            des[i][j] = seats[i][j];
        }
    }
    pthread_mutex_unlock(&seatMutx);
}

RESULT reserveSeat(int seatno,int booker){
    int row = seatno / COLS;
    int col = seatno % COLS;
    if(row < 0 || row >= ROWS || col < 0 || col >= COLS){
        return WRONG_SEATNO;
    }
    pthread_mutex_lock(&seatMutx);
    if(seats[row][col] == 0){
        seats[row][col] = booker;
        pthread_mutex_unlock(&seatMutx);
        return SUCCESS;
    }
    pthread_mutex_unlock(&seatMutx);
    return ALREADY_RESERVED;
}

RESULT cancelSeat(int seatno,int booker){
    int row = seatno / COLS;
    int col = seatno % COLS;
    if(row < 0 || row >= ROWS || col < 0 || col >= COLS){
        return WRONG_SEATNO;
    }
    pthread_mutex_lock(&seatMutx);
    if(seats[row][col] == 0){
        pthread_mutex_unlock(&seatMutx);
        return NOT_RESERVED;
    }
    if(seats[row][col] != booker){
        pthread_mutex_unlock(&seatMutx);
        return INVALID_BOOKER;
    }
    seats[row][col] = 0;
    pthread_mutex_unlock(&seatMutx);
    return SUCCESS;
}

void sendCurrentSeats(int clnt_sock, RES_PACKET * resPacket){
    copySeats(resPacket->seats);
    write(clnt_sock, resPacket, sizeof(RES_PACKET));
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
