//2021114335 박지원
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/in.h>
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

int generate_random_num(){
    return (rand() % RAND_END) + 1;
}

void printBingoBoard(int player_choice_array[ROW][COL])
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

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);
static sem_t sem_send;

int delay;
int end_flag = 0;

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    sem_init(&sem_send, 0, 0);
    void *thread_return;
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    
    while(delay != 1 && delay != 2){
        printf("Choose a player type (1: Fast player, 2: Slow player): ");
        scanf("%d", &delay);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);

    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sock);
    return 0;
}

void *send_msg(void *arg) // send thread main
{
    int sock = *((int *)arg);
    REQ_PACKET req;
    sem_wait(&sem_send);
    while(!end_flag)
    {
        memset(&req, 0, sizeof(req));
        req.cmd = BINGO_REQUEST;
        req.bingo_num = generate_random_num();
        printf("Random number: %d\n", req.bingo_num);
        write(sock, &req, sizeof(req));
        printf("[Tx] cmd: %d, bingo_num: %d, delay: %d\n", req.cmd, req.bingo_num, delay);
        sleep(delay);
    }

    return NULL;
}

void *recv_msg(void *arg) // read thread main
{
    int sock = *((int *)arg);
    RES_PACKET res;

    read(sock, &res, sizeof(res));
    printf("[Rx] cmd: %d, result: %d\n", res.cmd, res.result);
    if(res.cmd == BINGO_READY){
        printf("BINGO_READY received\n");
        sem_post(&sem_send);
    }

    while(1)
    {
        read(sock, &res, sizeof(res));
        printf("[Rx] cmd: %d, result: %d\n", res.cmd, res.result);
        if(res.cmd == BINGO_RESPONSE || res.cmd == BINGO_END)
            printBingoBoard(res.board);
        if(res.cmd == BINGO_END){
            end_flag = 1;
            printf("BINGO_END. Game is over\n");
            //이긴 사람 출력
            int i, j, count = 0;
            for(i=0; i < ROW; i++)
            {
                for(j=0; j < COL; j++)
                {
                    if(res.board[i][j] == 4){
                        count++;
                    }
                }
            }
            if(count == 8){
                printf("Draw [client1(%d) : client2(%d)]\n", count, 16-count);
                break;
            }
            int winner = count > 8 ? 1 : 2;
            printf("Client%d win [client1(%d) : client2(%d)]\n", winner, count, 16-count);
            break;
        }
    }

    return NULL;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
