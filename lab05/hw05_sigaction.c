/** 2021114335 박지원
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void parentTimeOut(int sig){
   static int elapsed = 0;
   elapsed += 2;
   if(sig==SIGALRM)
      printf("<Parent> time out: 2, elapsed time: %d seconds\n", elapsed);
   alarm(2);
}

void childTimeOut(int sig){
   static int elapsed = 0;
   elapsed += 5;
   if(sig==SIGALRM)
      printf("[Child] time out: 5, elapsed time: %d seconds\n", elapsed);
   alarm(5);
}

void childExit(int sig){
   int status;
   pid_t id = waitpid(-1, &status, WNOHANG);
   if(WIFEXITED(status))
      printf("Child id: %d, sent: %d\n", id, WEXITSTATUS(status));
}

int main(void){
   struct sigaction childAct;
   childAct.sa_handler = childExit;
   sigemptyset(&childAct.sa_mask);
   childAct.sa_flags = 0;
   sigaction(SIGCHLD, &childAct, 0);

   pid_t pid = fork();
   struct sigaction act;
   sigemptyset(&act.sa_mask);
   act.sa_flags=0;
   switch(pid){
   case -1:
      exit(1);
   case 0:
      act.sa_handler = childTimeOut;
      sigaction(SIGALRM, &act,0);
      alarm(5);
      for(int i = 0;i<20;i++)
         sleep(1);
      return 5;
      break;
   default:
      act.sa_handler = parentTimeOut;
      sigaction(SIGALRM, &act,0);
      alarm(2);
      while(1)
         sleep(1);
      break;
   }
   return 0;
}