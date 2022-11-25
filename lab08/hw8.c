#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#define BUF_SIZE 1024

void error_handling(char *m);

int main(int argc, char *argv[]){
    int fd1, fd2;
    FILE *fp1, *fp2;
    char buf1[BUF_SIZE], buf2[BUF_SIZE];

    if ((fd1 = open("data1.txt", O_RDONLY)) < 0)
        error_handling("open");

    if ((fd2 = dup(fd1)) < 0)
        error_handling("dup");

    if ((fp1 = fdopen(fd1, "r")) == NULL)
        error_handling("fdopen");

    if ((fp2 = fdopen(fd2, "r")) == NULL)
        error_handling("fdopen");

    while (fgets(buf1, BUF_SIZE, fp1) != NULL) {
        fputs(buf1, stdout);
        fflush(stdout);
        if(fgets(buf2, BUF_SIZE, fp2) != NULL) {
            fputs(buf2, stdout);
            fflush(stdout);
        }
    }

    fclose(fp1);
    fclose(fp2);

    return 0;
}

void error_handling(char *m){
    perror(m);
    exit(1);
}