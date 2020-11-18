#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost::asio::ip::tcp;

class session{
private:
  tcp::socket socket_;
  enum {max_length = 1024};
  char data_[max_length];
public:
  
  // handler-callback for async reading
  void handle_read(const boost::system::error_code& error, size_t bytes_transferred){
    if(!error){
      boost::asio::async_write(socket_,
                               boost::asio::buffer(data_, bytes_transferred),
                               boost::bind(&session::handle_write,
                                           this,
                                           boost::asio::placeholders::error));
    }
    else{
      delete this;
    }
  }
  
  // handle-callback for async write
  void handle_write(const boost::system::error& error){
    if(!error){
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
                             boost::bind(&session::handle_read,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
    }
    else{
      delete this;
    }
  }  
  
  // constructor
  session(boost::asio::io_service &io_service): socket_(io_service){};
  
  // get socket
  tcp::socket& socket(){return socket;}
  
  // start function
  void start(){
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                           boost::bind(&session::handle_read,
                                       this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
  };
}

class server{
private:
  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor;	// listening socket
public:
  // constructor
  server(boost::asio::io_service& io_service, short port):
  			io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4, port)){
    session* new_session = new session(io_service);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::handle_accept,
                                       this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }
  
  // handler for async connections
  void handle_accept(session* new_session, const boost::system::error_code& error){
    if(!error){
      new_session->start();
      new_session = new session(io_service_);
      acceptor_.async_accept(new_session->socket(),
                             boost::bind(&server::handle_accept,
                                         this,
                                         new_session,
                                         boost::asio::placeholders::error));
    }
    else{
      delete new_session;
    }
  }
}

int main(int argc, char** argv){
  boost::asio::io_service io_service;
  using namespace std;
  server s(io_service, atoi(argv[1]));
  io_service.run();
}
