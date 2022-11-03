#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void timeout2(int sig){
   if(sig==SIGALRM)
      puts("parent: Timeout!");
   alarm(2);
}

void timeout5(int sig){
   if(sig==SIGALRM)
      puts("child: Timeout!");
   alarm(5);
}

void (*setTime(int time))(int){
   switch(time){
   case 2:
      return timeout2;
   case 5:
      return timeout5;
   }
   return NULL;
}

int main(void){
   pid_t pid = fork();
   struct sigaction act;
   sigemptyset(&act.sa_mask);
   act.sa_flags=0;
   switch(pid){
   case -1:
      exit(1);
   case 0:
      act.sa_handler = setTime(5);
      sigaction(SIGALRM, &act,0);
      alarm(5);
      for(int i = 0;i<20;i++)
         sleep(100);
      break;
   default:
      act.sa_handler = setTime(2);
      sigaction(SIGALRM, &act,0);
      alarm(5);
      while(1)
         sleep(100);
      break;
   }
   return 0;
}