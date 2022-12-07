#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#define MAX_SIZE 1000
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

char* typeString[3] = {"Basic", "Standard", "Premium"};
int type;

void* getFile(void* arg);

int main(int argc, char *argv[])
{
   int sock;
   struct sockaddr_in serv_addr;
   pthread_t thread;
   void * thread_return;
   if(argc!=3) {
      printf("Usage : %s <IP> <port>\n", argv[0]);
      exit(1);
   }

    while(1){
        printf("-----------------------------------------------\n");
        printf("              K-OTT Service\n");
        printf("-----------------------------------------------\n");
        printf(" Choose a subscribe type\n");
        printf("-----------------------------------------------\n");
        printf("1: Basic, 2: Standard, 3: Premium, 4: quit: ");
        scanf("%d", &type);
        if(type == 4){
            printf("Exit program\n");
            exit(1);
        }
        type -= 1;
        int menu;
        printf("-----------------------------------------------\n");
        printf("1. Download, 2: Back to Main menu: ");
        scanf("%d", &menu);
        if(menu == 1)
            break;
    }
   
   sock=socket(PF_INET, SOCK_STREAM, 0);
   
   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
   serv_addr.sin_port=htons(atoi(argv[2]));
     
   if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
        printf("connect() error");
        exit(1);
    }

    pthread_create(&thread, NULL, getFile, (void*)&sock);
    pthread_join(thread, &thread_return);

   close(sock);  
   return 0;
}

void* getFile(void* arg){
    int sock = *((int*)arg);
    PACKET packet;
    packet.command = FILE_REQ;
    packet.type = type;
    packet.len = 0;
    write(sock, &packet, sizeof(packet));
    
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    int total = 0;
    while(1){
        read(sock, &packet, sizeof(packet));
        printf(".");
        total += packet.len;
        DEBUG_PRINT("%d %s %d\n", packet.len, typeString[packet.type], packet.command);
        if(packet.command == FILE_END)
            break;
    }
    
    clock_gettime(CLOCK_REALTIME, &end);
    packet.command = FILE_END_ACK;
    write(sock, &packet, sizeof(packet));
    printf("\n");
    printf("File Transmission Finished\n");
    printf("Total received bytes: %d\n", total);
    unsigned long time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
    printf("Downloading time: %lu msec\n", time);
    printf("Client closed\n");
    return NULL;
}