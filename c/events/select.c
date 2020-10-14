#include 
#include 
#include 

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include 
#include 


int set_nonblock(int fd){
  // См. выше перевод сокетов в неблокирующий режим
}

int main(int argc, char** argv){
  // Создадим Master-сокет для приема соединений
  int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  // Создадим множество сокетов для обслуживания соединений
  std::set SlaveSockets;
  
  // Выполним связывание с портом
  struct sockaddr_in SockAddr;
  SockAddr.sin_family = AF_INET;
  SockAddr.sin_port = htons(12345);
  SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(MasterSocket, (struct sockaddr*)(&SockAddr), sizeof(SockAddr));
  
  // Установка в неблокирующий режим Master-сокета
  set_nonblock(MasterSocket);
  
  // Запуск прослушивания очереди сообщений
  listen(MasterSocket, SOMAXCONN);
  
  while(true){
    /*
    1. Создадим массив из 1024 бит, где каждый бит означает наличие данных в сокете
    2. Обнулим его
    3. Добавим в него все существующие сокеты
    */
    fd_set Set;
    FD_ZERO(&Set);
    FD_SET(MasterSocket, &Set);
    for(auto Iter=SlaveSockets.begin(); Iter!=SlaveSockets.end(); Iter++){
 		FD_SET(*Iter, &Set);
	}
    
 	/*
 	Вызов select - блокирует процесс до появления событий
    Параметры:
    1. Наибольший из дескрипторов + 1
    2. fd_set для операций чтения (появились данные для чтения)
    3. fd_set для операций записи (появилось место для записи)
    4. fd_set для исключений (появилась исключительная ситуация)
    5. Таймаут
    */
    int Max = std::max(MasterSocket,
                       *std::max_element(SlaveSockets.begin(), SlaveSockets.end()));
    select(Max+1, &Set, NULL, NULL, NULL);
	
    // Цикл проверки событий для Slave-сокетов
    for(auto Iter=SlaveSockets.begin(); Iter!=SlaveSockets.end(); Iter++){
      if(FD_ISSET(*Iter, &set)){
        // Обработка события
        static char Buffer[1024];
        int RecvSize = recv(*Iter, Buffer, 1024, MSG_NOSIGNAL);
        if(RecvSize==0 && errno!=EAGAIN){
          /*
          Закончились данные для обработки:
          1. Закрываем соединение
          2. Закрываем сокет
          3. Удаляем дескриптор Slave-сокета 
          */
          shutdown(*Iter, SHUT_RDWR);
          close(*Iter);
          SlaveSockets.erase(Iter);
        }
        else if(RecvSize!=0){
          // echo-response
          send(*Iter, Buffer, RecvSize, MSG_NOSIGNAL);
        }
      }
    }
    if(FD_ISSET(MasterSocket, &set)){
      // Прием соединения
      int SlaveSocket = accept(MasterSocket, 0, 0);
      set_nonblock(SlaveSocket);
      SlaveSockets.insert(SlaveSocket);
    }
  }
}
