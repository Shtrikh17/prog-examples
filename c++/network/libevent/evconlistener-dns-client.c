#include <event2/util.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

evutil_socket_t get_tcp_socket_for_host(const char *hostname, ev_uint16_t port){
  char port_buf[6];
  struct evutil_addrinfo hints;
  struct evutil_addrinfo *result = NULL;
  int error;
  evutil_socket_t sckt;
  evutil_snprintf(port_buf, sizeof(port_buf), "%d", (int)port);
  /* Создание набора правил для выполнения поиска */
  memset(&hints, 0, sizeof(hints));
  
  hints.ai_family = AF_UNSPEC;
  /* рассматривать и IPv4, и IPv6 адреса */
  hints.ai_socktype = SOCK_STREAM;
  /* требуется потоковое соединение */
  hints.ai_protocol = IPPROTO_TCP;
  /* требуется TCP-сокет */
  /* IPv4-адреса возвращаются, только если для локального хоста сконфигурирован как минимум один IPv4-адрес; то же - для IPv6-адресов */
  hints.ai_flags = EVUTIL_AI_ADDRCONFIG;
  /* Выполнение операции поиска hostname */
  error = evutil_getaddrinfo(hostname, port_buf, &hints, &result);
  if( error < 0){
    fprintf(stderr, "Ошибка при выполнении операции разрешения '%s': %s\n",
            hostname, evutil_gai_strerror(error));
    return -1;
  }
  // Если ошибок не было, то в result должен содержаться по крайней мере один найденный адрес
  assert(result);
  /* Используется самый первый (или единственный) полученный адрес */
  sckt = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if( sckt < 0)
    return -1;
  if(connect(sckt, result->ai_addr, result->ai_addrlen)){
    /* Следует ещё раз отметить, что в данном случае устанавливается
     * блокирующее соединение. Если бы соединение было неблокирующим,
     * то потребовалась бы дополнительная специальная обработка
     * некоторых ошибок (например, EINTR, EAGAIN и т.д.)
     */
    EVUTIL_CLOSESOCKET(sckt);
    return -1;
  }
  return sckt;
}
