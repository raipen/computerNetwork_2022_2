#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048
void error_handling(char *buf);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock, sender_sock, receiver_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;

    socklen_t adr_sz;
    int fd_max, str_len, fd_num;
    char buf[BUF_SIZE];
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    while (1)
    {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;
        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
            break;
        if (fd_num == 0)
            continue;

        if (FD_ISSET(serv_sock, &cpy_reads))
        {
            adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
            FD_SET(clnt_sock, &reads);
            if (fd_max < clnt_sock)
                fd_max = clnt_sock;
            printf("connected client: %d\n", clnt_sock);
            str_len = read(clnt_sock, buf, BUF_SIZE);
            buf[str_len] = 0;
            if (strcmp(buf, "Sender") == 0)
                sender_sock = clnt_sock;
            else if (strcmp(buf, "Receiver") == 0)
                receiver_sock = clnt_sock;
            else
            {
                printf("wrong client: %s\n", buf);
                close(clnt_sock);
                FD_CLR(clnt_sock, &reads);
            }
        }
        else if (FD_ISSET(sender_sock, &cpy_reads))
        {
            str_len = read(sender_sock, buf, BUF_SIZE);
            if (str_len == 0)
            {
                FD_CLR(sender_sock, &reads);
                close(sender_sock);
                printf("closed client: %d\n", sender_sock);
            }
            else
            {
                printf("Forward  [%d] ---> [%d]\n", sender_sock, receiver_sock);
                write(receiver_sock, buf, str_len);
            }
        }
        else if (FD_ISSET(receiver_sock, &cpy_reads))
        {
            str_len = read(receiver_sock, buf, BUF_SIZE);
            if (str_len == 0)
            {
                FD_CLR(receiver_sock, &reads);
                close(receiver_sock);
                printf("closed client: %d\n", receiver_sock);
            }
            else
            {
                printf("Backward [%d] <--- [%d]\n", receiver_sock, sender_sock);
                write(sender_sock, buf, str_len);
            }
        }
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *buf)
{
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}
