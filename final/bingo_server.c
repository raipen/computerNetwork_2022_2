//2021114335 박지원
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define MAX_CLNT 256

#define ROW 4
#define COL 4

#define FALSE -1
#define TRUE  1

#define EMPTY 0
#define FULL  -1

#define RAND_START 1
#define RAND_END   16

// cmd field value 
#define BINGO_READY		0 	// Server -> Client
#define BINGO_REQUEST 	1	// Client -> Server
#define BINGO_RESPONSE	2	// Server -> Client
#define BINGO_END		3	// Server -> Client 

// result field value
#define FAIL	-1
#define SUCCESS 1

// Server -> Client
typedef struct {
	int cmd;			// cmd field value
	int board[ROW][COL]; // 숫자를 맞춘 클라이언트 id (4: client #1, 5: client #2)
	int result; 		// FAIL, SUCCESS
}RES_PACKET;

// Client -> Server
typedef struct {
	int cmd;
	int bingo_num;
}REQ_PACKET;

int bingo_number_array[ROW][COL];

/*----------------------------------------------------------
 * show_bingo_number()
 *   . bingo_number_array[][] 배열에 저장된 모든 값을 출력 
 *----------------------------------------------------------*/
void show_bingo_number()
{
	int i, j;

	printf("+-------------------+\n");
	for(i=0; i < ROW; i++)
	{
		for(j=0; j < COL; j++)
		{
			printf("| %2d ", bingo_number_array[i][j]);
		}

		printf("|\n");
		printf("+-------------------+\n");
	}

	printf("\n");
}

/*----------------------------------------------------------
 * get_empty_index()
 *   . bingo_number_array[][] 배열에서 비어 있는 배열의 index 값을 리턴함
 *	 . 0: 빈공간
 *  -  reutrn 값 
 *   . index=-1: Full, 그외 숫자: 비어있는 index	
 *----------------------------------------------------------*/
int get_empty_index()
{
	int i, j;
	int index = FULL;

	for(i=0; i < ROW; i++)
	{
		for(j=0; j < COL; j++)
		{
			if(bingo_number_array[i][j] == 0)
			{
				index = (i * ROW) + j;
				return index;
			}
		}
	}

	return FULL;
}

/*----------------------------------------------------------
 * exist_num(int num)
 *   . 입력된 숫자가 board[][]배열 내부에 존재하는지 검사 
 *	- return값 
 *   . index값(0~15): 이미 존재하는 숫자의 index, 
 *   . -1: FALSE(중복된 숫자 없음)
 *----------------------------------------------------------*/
int exist_number(int num)
{
	int i, j;
	int index = -1;

	for(i=0; i < ROW; i++)
	{
		for(j=0; j < COL; j++)
		{
			if(num == bingo_number_array[i][j])
			{
				index = (i * ROW) + j;
				return index; // 배열에 이미 동일한 숫자가 있는 경우, 1 리턴 
			}
		}
	}
	return FALSE;

}

/*----------------------------------------------------------
 * generate_randnum()
 *   . 랜덤 숫자 생성(숫자 범위: 1 ~ RAND_END)
 *   . board[][]배열에 생성된 랜덤숫자 입력 및 중복 검사  
 *	
 *----------------------------------------------------------*/
void generate_random_num()
{
	int rand_num = 0;
	int i, j;
	int found = 0;
	int index = 0;
	int row=0, col=0;

	srand(time(NULL)); // init random seed 

	while(1)
	{
		// board 배열에 빈공간이 없으면 프로그램 종료 

		index = get_empty_index();
		if(index == FULL)
		{
			printf("Bingo numbers are ready.\n");
			break;
		}

		rand_num = (rand() % RAND_END) + 1;
#ifdef DEBUG		
		printf("random num: %d\n", rand_num);
#endif 
		if(exist_number(rand_num) == FALSE)
		{
			row = index / ROW;
			col = index % ROW;
			bingo_number_array[row][col] = rand_num;
#ifdef DEBUG
			printf("row: %d, col: %d, rand_num: %d\n", row, col, rand_num);
#endif
		}

	}
	show_bingo_number();

}

void * handle_clnt(void * arg);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
pthread_mutex_t bingoMutx;

int player_choice_array[ROW][COL] = {0};

int main(int argc, char *argv[])
{

    generate_random_num();
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
		printf("Connected client IP: %s\n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}

void copyBord(int dest[ROW][COL]){
	int i,j;
	for(i=0;i<ROW;i++){
		for(j=0;j<COL;j++){
			dest[i][j] = player_choice_array[i][j];
		}
	}
}


void printBingoBoard()
{
	int i, j;
	printf("+-------------------+\n");
	for(i=0; i < ROW; i++)
	{
		for(j=0; j < COL; j++)
		{
            if(player_choice_array[i][j] == 0)
                printf("|    ");
            else if(player_choice_array[i][j] == 4)
                printf("|  %c ", 'O');
            else if(player_choice_array[i][j] == 5)
                printf("|  %c ", 'X');
		}

		printf("|\n");
		printf("+-------------------+\n");
	}

	printf("\n");
}

int isFull()
{
	int i, j;
	for(i=0; i < ROW; i++)
	{
		for(j=0; j < COL; j++)
		{
			if(player_choice_array[i][j] == 0)
			{
				pthread_mutex_unlock(&bingoMutx);
				return 0;
			}
		}
	}
	return 1;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int read_len=0, i,index;
	RES_PACKET resPacket;
	REQ_PACKET reqPacket;

    //BINGO_READY 메시지 전송
    memset(&resPacket, 0, sizeof(resPacket));
    resPacket.cmd = BINGO_READY;
    resPacket.result = SUCCESS;
    write(clnt_sock, &resPacket, sizeof(resPacket));
    printf("[Tx] cmd: %d, clinet_id: %d, result: %d\n", resPacket.cmd, clnt_sock, resPacket.result);

    while(1)
    {
        read_len=read(clnt_sock, &reqPacket, sizeof(reqPacket));
        if(read_len==-1)
            return (void*)-1;
        if(read_len==0)
            break;
        if(reqPacket.cmd != BINGO_REQUEST)
            continue;

        printf("[Rx] client: %d, cmd: %d, num: %d\n", clnt_sock, reqPacket.cmd, reqPacket.bingo_num);
		memset(&resPacket, 0, sizeof(resPacket));
		if(isFull()){
			resPacket.cmd = BINGO_END;
			resPacket.result = SUCCESS;
			copyBord(resPacket.board);
			for(i=0; i<clnt_cnt; i++){
				write(clnt_socks[i], &resPacket, sizeof(resPacket));
				printf("[Tx] cmd: %d, clinet_id: %d, result: %d\n", resPacket.cmd, clnt_socks[i], resPacket.result);
			}
			show_bingo_number();
			printBingoBoard();
			printf("Bingo is full. Game is over.\n");
			continue;
		}

		resPacket.cmd = BINGO_RESPONSE;
		index = exist_number(reqPacket.bingo_num);
        pthread_mutex_lock(&bingoMutx);
        if(player_choice_array[index / COL][index % COL] == 0)
        {
            printf("Bingo client: %d, [%d][%d]: %d\n", clnt_sock, index / COL,index % COL, reqPacket.bingo_num);
            player_choice_array[index / COL][index % COL] = clnt_sock;
            resPacket.result = SUCCESS;
        }
        else
        {
            printf("Another client already chose(num: %d)\n", reqPacket.bingo_num);
			resPacket.result = FAIL;
        }
        show_bingo_number();
		copyBord(resPacket.board);
		for(i=0; i<clnt_cnt; i++){
			write(clnt_socks[i], &resPacket, sizeof(resPacket));
			printf("[Tx] cmd: %d, clinet_id: %d, result: %d\n", resPacket.cmd, clnt_socks[i], resPacket.result);
		}
		printBingoBoard();
        pthread_mutex_unlock(&bingoMutx);
		
    }

	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
		    printf("Close clnt_sock: %d, clnt_cnt: %d\n", clnt_sock, clnt_cnt);
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

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
