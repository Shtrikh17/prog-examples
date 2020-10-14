
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include 
#include 
#include 
#include 

#include <sys/event.h>

int set_nonblock(int fd){
  // См. выше перевод сокетов в неблокирующий режим
}

int main(int argc, char** argv){
  int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  struct sockaddr_in SockAddr;
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port = htons(12345);
  SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(MasterSocket, (struct sockaddr*)(&SockAddr), sizeof(SockAddr));
  
  set_nonblock(MasterSocket);
  
  listen(MasterSocket, SOMAXCONN);
  
  // Create descriptor
  int KQueue = kqueue();
  // Create kevent and fill it
  struct kevent KEvent;
  bzero(&KEvent, sizeof(KEvent));
  EV_SET(&KEvent, MasterSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
  kevent(KQueue, &KEvent, 1, NULL, 0, NULL);
  
  while(true){
  	bzero(&KEvent, sizeof(KEvent));
  	kevent(KQueue, NULL, 0, &KEvent, 1, NULL);
  	if(KEvent.filter==EVFILT_READ){
      if(KEvent.ident==MasterSocket){
        int SlaveSocket = accept(MasterSocket, 0, 0);
        set_nonblock(SlaveSocket);
        bzero(&KEvent, sizeof(KEvent));
        EV_SET(&KEvent, SlaveSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
        kevent(KQueue, &KEvent, 1, NULL, 0, NULL); 
      }
      else{
        static char Buffer[1024];
        int RecvSize = recv(KEvent.ident, Buffer, 1024, MSG_NOSIGNAL);
        if(RecvSize<=0){
          close(KEvent.ident);
          printf("disconnected!\n");
        }
        else{
          send(KEvent.ident, Buffer, RecvSize, MSG_NOSIGNAL);
        }
      }
  	} 
  }
} 
