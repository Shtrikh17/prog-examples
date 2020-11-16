// Программа mycat: копирование файла в stdout

ssize_t read_fd(int fd, void *ptr, size_t nbytes, int *recvfd){
  struct msghdr msg;
  struct iovec iov[1];
  ssize_t n;
  int newfd;
  
  // Заполняем структуру msghdr - заголовок сообщения, передаваемого через сокет
  #ifdef HAVE_MSGHDR_MSG_CONTROL
  union {
    struct cmsghdr cm;
    char control[CMSG_SPACE(sizeof(int))];
  } control_un;
  struct cmsghdr *cmptr;
  
  msg.msg_control = control_un.control;
  msg.msg_controllen = sizeof(control_un.control);
  #else
  msg.msg_accrights = (caddr_t)&newfd;
  msg.msg_accrightslen = sizeof(int);
  #endif
  
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  
  // Добавляем в заголовок информацию о векторе для записи
  // Будет передан 1 байт совместно с дескриптором
  iov[0].iov_base = ptr;
  iov[0].iov_len = nbytes;
  
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  
  // Получаем сообщение
  if ((n = recvmsg(fd, &msg, 0)) <= 0)
    return (n);
  
  #ifdef HAVE_MSGHDR_MSG_CONTROL
  if ((cmptr = CMSG_FIRSTHDR(&msg)) != NULL && cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {
    if (cmptr->cmsg_level != SOL_SOCKET)
      err_quit("control level != SOL_SOCKET");
    if (cmptr->cmsg_type != SCM_RIGHTS)
      err_quit("control type != SCM_RIGHTS");
    *recvfd = *((int*)CMSG_DATA(cmptr));
  } else
    *recvfd = -1; /* дескриптор не был передан */
  #else
  if (msg.msg_accrightslen == sizeof(int))
    *recvfd = newfd;
  else
    *recvfd = -1; /* дескриптор не был передан */
  #endif
  return (n);
}

int my_open(const char *pathname, int mode){
  int fd, sockfd[2], status;
  pid_t childpid;
  
  char c, argsockfd[10], argmode[10];
  
  // Создаем пару сокетов
  socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);	
  
  // Дочерний процесс
  if ((childpid = fork()) == 0){
    // Сразу закрываем ненужный конец сокета, доставшийся от родителя - STDIN
    close(sockfd[0]);
    // Готовим аргументы командной строки для дочернего процесса и вызываем его
    snprintf(argsockfd, sizeof(argsockfd), "%d", sockfd[1]);
    snprintf(argmode, sizeof(argmode), "%d", mode);
    execl("./openfile", "openfile", argsockfd, pathname, argmode, (char*)NULL);
    err_sys("execl error");
  }

  // Родительский процесс - дочерний никогда сюда не доберется
  // Сразу закрываем ненужный конец сокета - используемый для записи
  close(sockfd[1]);
  // Ожидаем завершения дочернего процесса
  waitpid(childpid, &status, 0);
  
  if (WIFEXITED(status) == 0)
    err_quit("child did not terminate");
  
  // По завершении дочернего процесса читаем из сокета файловый дескриптор
  if ((status = WEXITSTATUS(status)) == 0)
    read_fd(sockfd[0], &c, 1, &fd);
  else{
    errno = status; /* установка значения errno в статус дочернего процесса */
    fd = -1;
  }
  // Закрываем сокет
  close(sockfd[0]);
  // Возвращаем файловый дескриптор программе
  return (fd);
}

int main(int argc, char **argv){
  int fd, n;
  char buff[BUFFSIZE];
  if (argc != 2)
    err_quit("usage: mycat <pathname>");
  if((fd = my_open(argv[1], O_RDONLY)) < 0)
    err_sys("cannot open %s", argv[1]);
  
  while ((n = read(fd, buff, BUFFSIZE)) > 0)
    write(STDOUT_FILENO, buff, n);
  
  exit(0);
}
