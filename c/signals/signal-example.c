/// Необходимо разработать приложение, открывающее сокет (для простоты UDP) и выполняющее в потоке операцию чтения. 
// Данное приложение должно корректно без задержек завершаться по требованию пользователя.

#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>

bool stop = false;

void hdl(int sig){
stop = true;
}

void* blocking_read(void* arg){
if(stop){
// не успели стартовать, а нас уже прикрыли ?
std::cout << "Thread was aborted\n";
pthread_exit((void *)0);
}
// Блокируем сигнал SIGINT
sigset_t set, orig;
sigemptyset(&set);
sigaddset(&set, SIGINT);
sigemptyset(&orig);
pthread_sigmask(SIG_BLOCK, &set, &orig);

if(stop){
// пока мы устанавливали блокировку сигнала он уже произошел
// возвращаем все как было и выходим
std::cout << "Thread was aborted\n";
pthread_sigmask(SIG_SETMASK, &orig, 0);
pthread_exit((void *)0);
}

// Здесь нас не могут прервать сигналом SIGINT
std::cout << "Start thread to blocking read\n";
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
const int optval = 1;
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

struct sockaddr_in addr;
bzero(&addr, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port = htons(10000);
addr.sin_addr.s_addr = htonl(INADDR_ANY);
bind(sockfd, (sockaddr *)&addr, sizeof(addr));

struct pollfd clients[1];
clients[0].fd = sockfd;
clients[0].events = POLLIN | POLLHUP;
clients[0].revents = 0;

ppoll((struct pollfd*) &clients, 1, NULL, &orig); // Может быть корректно прервана сигналом
// сдесь сигнал SIGINT все еще заблокирован

if(stop){
// получили сигнал о завершении работы
std::cout << "Thread was aborted\n";
close(sockfd);
pthread_sigmask(SIG_SETMASK, &orig, 0);
}

// Мы либо считали данные, либо произошла какая-то ошибка. Но мы не получали
// сигнала о завершении работы и продолжаем работать "по плану"

close(sockfd);
pthread_exit((void *)0);
}

int main(){
std::cout << "Start application\n";

struct sigaction act;
memset(&act, 0, sizeof(act));
act.sa_handler = hdl;
sigemptyset(&act.sa_mask);
sigaddset(&act.sa_mask, SIGINT);
sigaction(SIGINT, &act, 0);

pthread_t th1;
pthread_create(&th1, NULL, blocking_read, NULL);
// даже если мы его впихнем тут, наш тред корректно завершится
//pthread_kill(th1, SIGINT);
sleep(1);

std::cout << "Wait 5 seconds and stop\n";
sleep(5);
pthread_kill(th1, SIGINT);
pthread_join(th1, NULL);
}
