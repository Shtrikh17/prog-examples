
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include 
#include 
#include <sys/epoll.h>

#define MAX_EVENTS 32

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
  
  /*
  Создадим epoll-дескриптор и зарегистрируем в нем Master-сокет и соответствующую структуру событий с помощью epoll_ctl
  Поля структуры, регистрируемой вместе с сокетом
  1. data.fd - дескриптор
  2. events - по аналогии с poll
  */
  int EPoll = epoll_create1(0);
  struct epoll_event Event;
  Event.data.fd = MasterSocket;
  Event.events = EPOLLIN;
  epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event);
    
  while(true){
    // Создаем массив структур событий и передаем его в синхронный системный вызов
    struct epoll_event Events[MAX_EVENTS];
    int N = epoll_wait(EPoll, Events, MAX_EVENTS, -1);
    
    // Цикл проверки при возврате
    for(unsigned int i=0; i<N; i++){
      if(Events[i].data.fd == MasterSocket){
        /*
        Обработчик для Master-сокета: принимаем соединение и регистрируем сокет в epoll по аналогии с Master-сокетом
        */
        int SlaveSocket = accept(MasterSocket, 0, 0);
        set_nonblock(SlaveSocket);
        struct epoll_event Event;
        Event.delta.fd = SlaveSocket;
        Event.events = EPOLLIN;
        epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event);
      }
      else{
        // Разрыв соединения или echo-response
        static char Buffer[1024];
        int RecvResult = recv(Events[i].data.fd, Buffer, 1024, MSG_NOSIGNAL);
        if(RecvResult==0 && errno!=EAGAIN){
          shutdown(Events[i].data.fd, SHUT_RDWR);
          close(Events[i].data.fd);
        }
        else if(RecvResult>0){
          send(Events[i].data.fd, Buffer, RecvResult, MSG_NOSIGNAL);
        }
      }
    }   
  }   
}
