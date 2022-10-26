#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
//명령행 인자로 원본 파일과 대상 파일을 입력받아
//저수준 입출력 함수를 사용하여 원본 파일을 대상 파일로 이동시키는 프로그램
void error_handling(char *message);
int main(int argc, char *argv[]){
    int fin, fout;
    int size, totalSize = 0;
    char buf[BUF_SIZE];

    if (argc != 3)
        error_handling("[Error] mymove Usage: ./mymove src_file dest_file");

    fin = open(argv[1], O_RDONLY);
    if (fin == -1)
        error_handling("open(src_file) error!");

    fout = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fout == -1)
        error_handling("open(dest_file) error!");

    while ((size = read(fin, buf, BUF_SIZE)) > 0)
        if (write(fout, buf, size) != size)
            error_handling("write error");
        else
            totalSize += size;

    if (size == -1)
        error_handling("read error");

    printf("move from %s to %s (bytes: %d) finished.\n", argv[1], argv[2], totalSize);
    
    close(fin);
    close(fout);
    if(remove(argv[1])==-1)
        error_handling("remove error");
    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}