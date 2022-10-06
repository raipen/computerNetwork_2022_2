#include <stdio.h>
#include <string.h>
#define BUF_SIZE 1024
int main(int argc, const char *argv[])
{
    char msg1[BUF_SIZE];
    char msg2[BUF_SIZE];
    memset(msg1, 0, sizeof(msg1));
    memset(msg2, 0, sizeof(msg2));
    // scanf() 사용
    printf("Input string1 -> ");
    scanf("%s", msg1);
    printf("msg1: %s, len: %d\n", msg1, strlen(msg1));
    getchar();
    // fgets() 사용
    printf("Input string2 -> ");
    fgets(msg2, BUF_SIZE, stdin);
    printf("msg2: %s, len: %d\n", msg2, strlen(msg2));
    return 0;
}