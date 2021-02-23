#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include "env.h"
#include "exe.h"
using boost::asio::ip::tcp;
using namespace std;
string client_addr;
int client_port;
void input_handler(string &a){
    a.erase(std::remove(a.begin(), a.end(), '\r'), a.end());
}

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec) 
          {
            string data = string(data_);
            input_handler(data);
            ENV *env = new ENV(data, client_addr, client_port);
            env->print_env();
            EXE_CGI *exe = new EXE_CGI(data);
            int socketfd = socket_.native_handle();
            pid_t pid = fork();
            while(pid < 0){
                waitpid(-1, nullptr, 0);
                pid = fork();
            }

            if(pid == 0){
                dup2(socketfd, 1);
                if(exe->exe()){
                    return ;
                }
            }else{
                socket_.close();
            }
            do_write(length);
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(&data_[0], length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            do_read();
          }
        });
  }

  tcp::socket socket_;
  enum { max_length = 15000 };
  char data_[15000];
};

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            client_addr = socket.remote_endpoint().address().to_string();
            client_port = socket.remote_endpoint().port();
            std::make_shared<session>(std::move(socket))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    server s(io_context, std::atoi(argv[1]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
