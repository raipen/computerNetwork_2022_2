#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>

#define ROWS 2
#define COLS 10

typedef enum
{
    INQUIRY = 1,
    RESERVATION,
    CANCELLATION,
    QUIT
} COMMAND;
typedef enum
{
    SUCCESS = 0,      // 예약 현황 조회 성공, 예약 성공, 예약 취소 성공
    WRONG_SEATNO,     // 잘못된 좌석 번호
    ALREADY_RESERVED, // 이미 예약된 좌석
    NOT_RESERVED,     // 예약되지 않은 좌석
    INVALID_BOOKER    // 예약자가 아님
} RESULT;
char *resultStr[] = {"Operation success.",
                     "Wrong seat number.",
                     "Reservation failed. (The seat was already reserved.)",
                     "Cancellation failed. (The seat was not reserved.)",
                     "Cancellation failed. (The seat was reserved by another person.)"};

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

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);
void printSeats(int seats[ROWS][COLS]);
static sem_t sem_send;
static sem_t sem_recv;

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    sem_init(&sem_send, 0, 1);
    sem_init(&sem_recv, 0, 0);
    void *thread_return;
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
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
    int end_flag = 0;
    while(!end_flag)
    {
        sem_wait(&sem_send);
        memset(&req, 0, sizeof(req));
        printf("1: inquiry, 2: reservation, 3: cancellation, 4: quit: ");
        scanf("%d", &req.command);
        switch(req.command)
        {
            case INQUIRY:
                break;
            case RESERVATION:
                printf("Input seat number: ");
                scanf("%d", &req.seatno);
                req.seatno--;
                break;
            case CANCELLATION:
                printf("Input seat number for cancellation: ");
                scanf("%d", &req.seatno);
                req.seatno--;
                break;
            case QUIT:
                printf("Quit.\n");
                end_flag = 1;
                break;
        }
        write(sock, &req, sizeof(req));
        sem_post(&sem_recv);
    }

    return NULL;
}

void *recv_msg(void *arg) // read thread main
{
    int sock = *((int *)arg);
    RES_PACKET res;
    int end_flag = 0;
    while(!end_flag)
    {
        sem_wait(&sem_recv);
        memset(&res, 0, sizeof(res));
        read(sock, &res, sizeof(res));
        if(res.command == QUIT){
            sem_post(&sem_send);
            break;
        }
        printSeats(res.seats);
        printf("%s\n", resultStr[res.result]);
        sem_post(&sem_send);
    }

    return NULL;
}

void printSeats(int seats[ROWS][COLS])
{
    int i, j;
    printf("---------------------------------------------------\n");
    printf("            Bus Reservation Status\n");
    printf("---------------------------------------------------\n");
    for(i = 0; i < ROWS; i++)
    {   
        printf("|");
        for(j = 0; j < COLS; j++)
        {
            printf(" %2d |", i * COLS + j + 1);
        }
        printf("\n---------------------------------------------------\n");
        printf("|");
        for(j = 0; j < COLS; j++)
        {
            printf(" %2d |", seats[i][j]);
        }
        printf("\n---------------------------------------------------\n");
    }
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
