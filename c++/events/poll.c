
#include 
#include 
#include 

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include 
#include 
#include 

#define POLL_SIZE 2048

int set_nonblock(int fd){
  // См. выше перевод сокетов в неблокирующий режим
}

int main(int argc, char** argv){
  int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  std::set SlaveSockets;
  
  struct sockaddr_in SockAddr;
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port = htons(12345);
  SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(MasterSocket, (struct sockaddr*)(&SockAddr), sizeof(SockAddr));
  
  set_nonblock(MasterSocket);
  
  listen(MasterSocket, SOMAXCONN);
  
  /*
  Создаем массив структур pollfd и вносим в него Master-сокет
  Поля структуры:
  1. fd - дескриптор
  2. events - отслеживаемые события (маска)
  */
  struct pollfd Set[POLL_SIZE];
  Set[0].fd = MasterSocket;
  Set[0].events = POLLIN;
  
  while(true){
    // Вносим в массив структур все Slave-сокеты
    unsigned int index = 1;
    for(auto Iter=SlaveSockets.begin(); Iter!=SlaveSockets.end(); Iter++){
      Set[index].fd = *Iter;
      Set[index].events = POLLIN;
      index++;
    }
    
    // Синхронный вызов poll
    unsigned int SetSize = 1 + SlaveSockets.size();
    poll(Set, SetSize, -1);			// timeout = -1 => infinity
    
    // По очереди просматриваем каждую структуру на наличие соединений
    for(unsigned int i=0; i<SetSize; i++){
      if(Set[i].revents & POLLIN){
        if(i){
          // Обработчик для Slave-сокетов
          static char Buffer[1024];
          int RecvSize = recv(Set[i].fd, Buffer, 1024, MSG_NOSIGNAL);
          
          if(RecvSize==0 && errno!=EAGAIN){
            // Разрыв соединения
            shutdown(Set[i].fd, SHUT_RDWR);
            close(Set[i].fd);
            SlaveSockets.erase(Set[i].fd);
          }
          else if(RecvSize>0){
            // Echo-response
            send(Set[i].fd, Buffer, RecvSize, MSG_NOSIGNAL);
          }
        }
        else{
          // Обработчик для Master-сокета
          int SlaveSocket = accept(MasterSocket, 0, 0);
          set_nonblock(SlaveSocket);
          SlaveSockets.insert(SlaveSocket);
        }
      }
    }
}
