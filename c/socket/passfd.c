/* passfd.с */

/* Программа ведет себя подобно обычной команде /bin/cat, которая обрабатывает
   только один аргумент (имя файла). Мы создаем сокеты домена Unix при помощи
   socketpair(), затем разветвляем через fork(). Дочерний процесс открывает файл,
   имя которого передается в командной строке, пересылает файловый дескриптор и
   имя файла обратно в порождающий процесс, после этого завершается. Родительский
   процесс ожидает файловый дескриптор от дочернего процесса, а потом копирует
   данные из файлового дескриптора в stdout до тех пор, пока данные не
   заканчиваются. Затем родительский процесс завершается. */

#include <alloca.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sockutil.h" /* простые служебные функции */

/* Дочерний процесс. Он пересылает файловый дескриптор. */
int childProcess(char * filename, int sock){
  int fd;
  struct iovec vector;		/* некоторые данные для передачи fd в w/ */
  struct msghdr msg;		/* полное сообщение */
  struct cmsghdr* cmsg;		/* управляющее сообщение, которое включает в себя fd */

  /* Открыть файл, дескриптор которого будет передан. */
  if ((fd = open(filename, O_RDONLY)) < 0){
    perror("open");
    return 1;
  }
  
  /* Передать имя файла через сокет, включая завершающий символ '' */
  vector.iov_base = filename;
  vector.iov_len = strlen(filename) + 1;

  /* Соединить первую часть сообщения. Включить имя файла iovec */
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &vector;
  msg.msg_iovlen = 1;
 
  /* Теперь управляющее сообщение. Нужно выделить участок памяти для файлового дескриптора. */

  cmsg = alloca(sizeof(struct cmsghdr) + sizeof(fd));
  cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  
  /* Копировать файловый дескриптор в конец управляющего сообщения */
  memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));
  
  msg.msg_control = cmsg;
  msg.msg_controllen = cmsg->cmsg_len;
  
  if (sendmsg(sock, &msg, 0) != vector.iov_len)
    die("sendmsg");
  
  return 0;
}

/* Родительский процесс. Он получает файловый дескриптор. */
int parentProcess(int sock) {
  char buf[80];          /* пространство для передачи имени файла */
  struct iovec vector;   /* имя файла от дочернего процесса */
  struct msghdr msg;     /* полное сообщение */
  struct cmsghdr * cmsg; /* управляющее сообщение с fd */
  int fd;
  
  /* установка iovec для имени файла */
  vector.iov_base = buf;
  vector.iov_len = 80;
  
  /* сообщение, которое мы хотим получить */
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &vector;
  msg.msg_iovlen = 1;
  
  /* динамическое распределение (чтобы выделить участок памяти для файлового дескриптора) */
  cmsg = alloca(sizeof(struct cmsghdr) + sizeof(fd));
  cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
  msg.msg_control = cmsg;
  msg.msg_controllen = cmsg->cmsg_len;
  
  if (!recvmsg(sock, &msg, 0))
    return 1;
  printf("получен файловый дескриптор для '%s'n",
         (char *) vector.iov_base);
  
  /* присвоение файлового дескриптора из управляющей структуры */
  memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));
  copyData(fd, 1);
  return 0;
}

int main(int argc, char ** argv) {
  int socks[2];
  int status;
  
  if (argc != 2) {
    fprintf(stderr, "поддерживается только один аргументn");
    return 1;
  }
  
  /* 	Создание сокетов. Один служит для родительского процесса,
  		второй — для дочернего (мы можем поменять их местами,
  		если нужно). */
  if (socketpair(PF_UNIX, SOCK_STREAM, 0, socks))
    die("socketpair");
  if (!fork()) {
    /* дочерний процесс */
    close(socks[0]);
    return childProcess(argv[1], socks[1]);
  }
  
  /* родительский процесс */
  close(socks[1]);
  parentProcess(socks[0]);
  
  /* закрытие дочернего процесса */
  wait(&status);
  
  if (WEXITSTATUS(status))
    fprintf(stderr, "childfailedn");
  
  return 0;
}
