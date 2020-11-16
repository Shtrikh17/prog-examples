#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void hdl(int signum){
  exit(0);
}                                                                                               

int main(){
  pid_t pid = fork();
  FILE* f;
  if(pid){
    f = fopen("/home/box/pid_parent", "w");
    fprintf(f, "%d", getpid()); 
    fclose(f); 
    waitpid(pid, NULL, 0);
    exit(0);
  }
  else{
    f = fopen("/home/box/pid_child", "w");
    fprintf(f, "%d", getpid());
    fclose(f);
    signal(SIGTERM, hdl);
    pause();
  }
}   
