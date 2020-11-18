#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#define UNIX_SOCK_PATH "/tmp/echo.sock"

static void echo_read_cb(struct bufferevent* bev, void* ctx){
  // Поступили данные для чтения
  // Получаем находящиеся внутри bufferevent'а буферы ввода и вывода
  struct evbuffer* input = bufferevent_get_input(bev);
  struct evbuffer* output = bufferevent_get_output(bev);
  
  // Узнаем, сколько данных пришло, и копируем их в локальный буфер
  size_t length = evbuffer_get_length(input);
  char* data;
  data = malloc(length);
  evbuffer_copy_out(input, data, length);
  printf("data: %s\n", data);
  
  // echo-response, причем вся отправка осуществляется прозрачно, уровнем ниже
  // самим bufferevent'ом
  evbuffer_add_buffer(output, input);
  free(data);
}

struct void echo_event_cb(struct bufferevent* bev, short events, void* ctx){
  // Обработка ошибок
  if(events & BEV_EVENT_ERROR){
    perror("Error");
    bufferevent_free(bev);
  }
  if(events & BEV_EVENT_EOF){
    bufferevent_free(bev);
  }
}

static void accept_conn_cb(struct evconnlistener* listener,
                           evutil_socket_t fd,
                           struct sockaddr* address,
                           int socklen,
                           void* ctx){
  // Получаем для этого listener'а соответствующий event_base
  struct event_base* base = evconnlistener_get_base(listener);
  // Создаем bufferevent для сокета
  struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  // Устанавливаем callback'и - для чтения и для событий
  bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
  // Включаем bufferevent
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void accept_error_cb(struct evconnlistener* listener,
                           evutil_socket_t fd,
                           struct sockaddr* address,
                           int socklen,
                           void* ctx){
  // Получаем для этого listener'а соответствующий event_base
  struct event_base* base = evconnlistener_get_base(listener);
  // Получаем код ошибки
  int err = EVUTIL_SOCKET_ERROR();
  // Вывод сообщения об ошибке
  fprintf(stderr, "Error = %d \"%s\"\n", err, evutil_socket_error_to_string(err));
  // Выход из цикла обработки сообщений
  event_base_loopexit(base, NULL);
}

int main(int argc, char* argv[]){
  // Создаем event_base для управления событиями
  struct event_base* base = event_base_new();
  
  // Создаем сокет и настраиваем его
  struct sockaddr_un sun;
  memset(&sun, 0, sizeof(sun));
  sun.sun_family = AF_UNIX;
  strcpy(sun.sun_path, UNIX_SOCK_PATH);
  
  // Создаем TCP-listener и регистрируем функцию обратного вызова для него
  struct evconnlistener* listener = 
    evconnlistener_new_bind(base,
                            accept_conn_cb,
                            NULL,
                            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSABLE,
                            -1,
                            (struct sockaddr*)&sun,
                            sizeof(sun));
  // Также устанавливаем функцию обратного вызова для обработки ошибок
  evconnlistener_set_error_cb(listener, accept_error_cb);
  // Запуск цикла обработки событий
  event_base_dispatch(base);
  return 0;
}
