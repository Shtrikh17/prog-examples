// Функиця, которая пишет в сокет данные и файловый дескриптор
/*
 * sock - сокет, в который пишем
 * buf - буфер, который передается
 * buflen - длина буфера
 * fd - файловый дескриптор
 */
ssize_t sock_fd_write(int sock, void *buf, ssize_t buflen, int fd){
    ssize_t size;
    struct msghdr msg;		// Заголовок сообщения
    struct iovec iov;		// IO vector - представляет буфер для передачи данных
    union{					
      struct cmsghdr  cmsghdr;
      char        control[CMSG_SPACE(sizeof (int))];
    }cmsgu;				
  	// Специальная структура - control msg, используется для передачи дескриптора
    struct cmsghdr* cmsg;

  	// Заполнение буфера - для передачи одного числа
    iov.iov_base = buf;		// Указываем, откуда берем информацию
    iov.iov_len = buflen;	// ВАЖНО: buflen не может быть равен 0
	
  	// Сама структура сообщения
    msg.msg_name = NULL;	// Используется локальный сокет
    msg.msg_namelen = 0;	// Размер записи
    msg.msg_iov = &iov;		// Адрес буфера ввода
    msg.msg_iovlen = 1;		// Длина буфера ввода

    if (fd != -1){
      	// Передается дескриптор
        msg.msg_control = cmsgu.control;	// Устанавливаем для сообщения cmsg и ее длину
        msg.msg_controllen = sizeof(cmsgu.control);

        cmsg = CMSG_FIRSTHDR(&msg);					// Заполняем саму структуру cmsg
        cmsg->cmsg_len = CMSG_LEN(sizeof (int));	// int для передачи дескриптора
        cmsg->cmsg_level = SOL_SOCKET;				// Особые значения для ядра
        cmsg->cmsg_type = SCM_RIGHTS;

        printf ("passing fd %d\n", fd);
        *((int *) CMSG_DATA(cmsg)) = fd;
    } else {
      	// Передаются данные
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        printf ("not passing fd\n");
    }

    size = sendmsg(sock, &msg, 0);	// Как sendto, но пересылает именно сообщение

    if (size < 0)
        perror ("sendmsg");
    return size;
}

// Соответствующая функция получателя
ssize_t sock_fd_read(int sock, void *buf, ssize_t bufsize, int *fd){
    ssize_t     size;

    if (fd){
        struct msghdr msg;	      	// Структура для сообщения
        struct iovec iov;			// Вектор IO - для переданных данных
        union{
            struct cmsghdr cmsghdr;
            char control[CMSG_SPACE(sizeof (int))];
        }cmsgu;				// Специальная структура, используемая для передачи дескриптора
        struct cmsghdr* cmsg;	// Указатель на часть структуры
		
      	// Заполняем буферную структуру
        iov.iov_base = buf;
        iov.iov_len = bufsize;
		
      	// Заполняем структуру для сообщения
        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_control = cmsgu.control;
        msg.msg_controllen = sizeof(cmsgu.control);
        size = recvmsg (sock, &msg, 0);					// Получаем сообщение
        if (size < 0) {
            perror ("recvmsg");
            exit(1);
        }
      	// Получаем часть структуры сообщения и извлекаем дескриптор
        cmsg = CMSG_FIRSTHDR(&msg);
        if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
            if (cmsg->cmsg_level != SOL_SOCKET) {
                fprintf (stderr, "invalid cmsg_level %d\n", cmsg->cmsg_level);
                exit(1);
            }
            if (cmsg->cmsg_type != SCM_RIGHTS) {
                fprintf (stderr, "invalid cmsg_type %d\n", сmsg->cmsg_type);
                exit(1);
            }

            *fd = *((int *) CMSG_DATA(cmsg));
            printf ("received fd %d\n", *fd);
        } else
            *fd = -1;
    } else {
      // Иначе просто читаем даннные
        size = read (sock, buf, bufsize);
        if (size < 0) {
            perror("read");
            exit(1);
        }
    }
    return size;
}

// Пример работы
void child(int sock){
    int fd;
    char buf[16];
    ssize_t size;

    sleep(1);
    for (;;) {
        size = sock_fd_read(sock, buf, sizeof(buf), &fd);
        if (size <= 0)
            break;
        printf ("read %d\n", size);
        if (fd != -1) {
            write(fd, "hello, world\n", 13);
            close(fd);
        }
    }
}

void parent(int sock){
    ssize_t size;
    int i;
    int fd;

    fd = 1;
    size = sock_fd_write(sock, "1", 1, 1);
    printf ("wrote %d\n", size);
}

int main(int argc, char **argv){
    int sv[2];
    int pid;

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        exit(1);
    }
    switch ((pid = fork())) {
    case 0:
        close(sv[0]);
        child(sv[1]);
        break;
    case -1:
        perror("fork");
        exit(1);
    default:
        close(sv[1]);
        parent(sv[0]);
        break;
    }
    return 0;
}
