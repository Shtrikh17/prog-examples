#include <iostream>
#include <map>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string.h>

#include <event2/buf>

int set_nonblock(int fd){
    int flags;
#ifdef O_NONBLOCK
    if(-1==(flags=fcntl(fd, F_GETFL, 0))){
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags|O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, F_IOBIO, &flags);
#endif
}

#define MAX_EVENTS 32

int main() {

    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    std::map<int, std::string> SlaveSockets;

    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(1234);
    SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(MasterSocket, (struct sockaddr*)(&SockAddr), sizeof(SockAddr));

    set_nonblock(MasterSocket);

    listen(MasterSocket, SOMAXCONN);

    int EPoll = epoll_create1(0);
    struct epoll_event Event;
    Event.data.fd = MasterSocket;
    Event.events = EPOLLIN;
    epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event);


    while(true){
        struct epoll_event Events[MAX_EVENTS];
        int N = epoll_wait(EPoll, Events, MAX_EVENTS, -1);

        for(unsigned int i=0; i<N; i++){
            if(Events[i].data.fd==MasterSocket){
                // Accept
                struct sockaddr_in sSockAddr;
                socklen_t addrlen;
                char address[256];

                int SlaveSocket = accept(MasterSocket, (struct sockaddr*)(&sSockAddr), &addrlen);
                inet_ntop(AF_INET, &(sSockAddr.sin_addr), address, 256);
                std::string addr(address);
                set_nonblock(SlaveSocket);

                // Register
                struct epoll_event Event;
                Event.data.fd = SlaveSocket;
                Event.events = EPOLLIN;
                epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event);

                // Store
                SlaveSockets[SlaveSocket] = addr;

                // Notify
                for(auto each: SlaveSockets){
                    char msg[1024];
                    strcpy(msg, "[Server]: ");
                    strcat(msg, address);
                    strcat(msg, " connected!\n");
                    send(each.first, msg, strlen(msg), MSG_NOSIGNAL);
                }
            }
            else{
                char Buffer[1024];
                int RecvResult = recv(Events[i].data.fd, Buffer, 1024, MSG_NOSIGNAL);
                if(RecvResult==0 && errno!=EAGAIN){
                    shutdown(Events[i].data.fd, SHUT_RDWR);
                    close(Events[i].data.fd);
                    char address[256];
                    strcpy(address, SlaveSockets[Events[i].data.fd].c_str());
                    SlaveSockets.erase(Events[i].data.fd);
                    // Notify
                    for(auto each: SlaveSockets){
                        char msg[1024];
                        strcpy(msg, "[Server]: ");
                        strcat(msg, address);
                        strcat(msg, " disconnected!\n");
                        send(each.first, msg, strlen(msg), MSG_NOSIGNAL);
                    }
                }
                else if(RecvResult>0){
                    // Notify
                    for(auto each: SlaveSockets){
                        if(each.first==Events[i].data.fd) continue;
                        char msg[1024];
                        strcpy(msg, "[");
                        strcat(msg, SlaveSockets[Events[i].data.fd].c_str());
                        strcat(msg, "]: ");
                        strcat(msg, Buffer);
                        send(each.first, msg, 4+strlen(SlaveSockets[Events[i].data.fd].c_str())+RecvResult, MSG_NOSIGNAL);
                    }
                }

            }
        }
    }
    return 0;
}
