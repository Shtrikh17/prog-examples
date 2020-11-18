// Простой асинхронный сервер

using boost::asio;
typedef boost::shared_ptr<ip::tcp::socket> socket_ptr;
io_service service;
ip::tcp::endpoint ep( ip::tcp::v4(), 2001)); // listen on 2001
ip::tcp::acceptor acc(service, ep);
socket_ptr sock(new ip::tcp::socket(service));
acc.async_accept(*sock, boost::bind(handle_accept, sock, _1));
service.run();

void handle_accept(socket_ptr sock, const boost::system::error_code & err){
    if (err) return;
    // at this point, you can read/write to the socket
    socket_ptr sock(new ip::tcp::socket(service));
    acc.async_accept(*sock, boost::bind(handle_accept, sock, _1));
}
