#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <string>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include "env.h"
using boost::asio::ip::tcp;
using boost::algorithm::replace_all;
using namespace std;
string client_addr;
int client_port;
boost::asio::io_service io_context;
tcp::socket a(io_context);

void input_handler(string &a){
    a.erase(std::remove(a.begin(), a.end(), '\r'), a.end());
}
void replace_html_char(string &content){
    replace_all(content, "&", "&amp;");
    replace_all(content, "\"", "&quot;");
    replace_all(content, "\'", "&apos;");
    replace_all(content, "<", "&lt;");
    replace_all(content, ">", "&gt;");
    replace_all(content, "\n", "&NewLine;");
}


static vector<string>split(const string& s, const char d){
    vector<string> re;
    string tmp;
    stringstream str(s);
    while(getline(str, tmp, d)){
        if(tmp.length() > 0)
            re.push_back(tmp);
    }
    return re;
}

string get_value(string input){
    vector<string> ar = split(input, '=');
    if(ar.size() == 2){
        return ar[1];
    }
    return "";
}

class shell_connect
  : public std::enable_shared_from_this<shell_connect>
{
public://(boost::asio::io_context &io_context
  shell_connect(std::shared_ptr<tcp::socket> s, boost::asio::io_context &io_context_test, /*tcp::endpoint &t,*/ /*tcp::socket &b_socket*/string f, int id, string h, string p)
      :socket_(io_context_test),
      b_socket(s),
      resolver(io_context_test)
  {
    host = h;
    port = p;
    file = "test_case/" + f;
    session_id = id;

    if(host != ""){
        valid = 1;
    }
    else
        valid = 0;

  }

  void start(){
      if(valid == 1){
          do_connect();
      }
  }


private:

    void output_command(string content){
        replace_html_char(content);
        string session = "s" + to_string(session_id);
        replace_all(content, "\n", "&#13");
        replace_all(content, "\r", "");
        string str = (boost::format("<script>document.getElementById('%1%').innerHTML += '%2%';</script>") %session %content).str();
        strcpy(data_b, str.c_str());
        cout << session_id << " " << data_b << endl;
        b_socket->async_write_some(
                boost::asio::buffer(data_b, strlen(data_b)),
                [](boost::system::error_code ec, std::size_t len) {});
    }
    void output_shell(string content){
        replace_html_char(content);
        string session= "s" + to_string(session_id);
        replace_all(content, "\n", "&#13");
        replace_all(content, "\r", "");
        string str = (boost::format("<script>document.getElementById('%1%').innerHTML += '<b>%2%</b>';</script>") %session %content).str();
        strcpy(data_b, str.c_str());
        b_socket->async_write_some(
                boost::asio::buffer(data_b, strlen(data_b)),
                [](boost::system::error_code ec, std::size_t len) {});
    }
    void output_host(string content){
        replace_html_char(content);
        string session = "h" + to_string(session_id);
        replace_all(content, "\n", "&#13");
        replace_all(content, "\r", "");
        string str = (boost::format("<script>document.getElementById('%1%').innerHTML += '%2%';</script>") %session %content).str();
        strcpy(data_b, str.c_str());
        b_socket->async_write_some(
                boost::asio::buffer(data_b, strlen(data_b)),
                [](boost::system::error_code ec, std::size_t len) {
                    if(ec){
                        cout << "error" << endl;
                    }
                });
    }
  void do_connect(){
      auto self(shared_from_this());
      tcp::resolver::query q(host, port);
      tcp::endpoint tmp;
      resolver.async_resolve(q, [this, self]
      (boost::system::error_code ec, tcp::resolver::iterator iter)
      {
          if(!ec){
            // tmp = std::move(iter->endpoint());
            this->endpoint_ = std::move(iter->endpoint());
            std::cout << "1: " << (this->endpoint_.address()).to_string() << " " << (this->endpoint_.port()) << std::endl;
            auto self1(shared_from_this());
            socket_.async_connect(this->endpoint_,
              [this, self1](boost::system::error_code ec)
              {
                std::cout << "2: " << (this->endpoint_.address()).to_string() << " " << (this->endpoint_.port()) << std::endl;

              if(!ec){
                output_host(host+":"+port);
                filestream.open(file);
                do_read();
              }
              else{
                cout << "fail to connect server: " << host << endl;
                cout << ec.message();
              }
            });
          }else{
            cout << "resolve error" << endl;
            cout << ec.message();
          }
      });
      // auto self1(shared_from_this());
      //       socket_.async_connect(this->endpoint_,
      //         [this, self](boost::system::error_code ec)
      //         {
      //           std::cout << "2: " << (this->endpoint_.address()).to_string() << " " << (this->endpoint_.port()) << std::endl;

      //     // cout << "0.0" <<tmp.address().to_string() << " " << tmp.port() << endl;
      //         if(!ec){
      //           output_host(host+":"+port);
      //           filestream.open(file);
      //           // do_read();
      //         }
      //         else{
      //           cout << "fail to connect server: " << host << endl;
      //           cout << ec.message();
      //         }
      //       });
      // cout << "0.0" <<tmp.address().to_string() << " " << tmp.port() << endl;


  }

  bool check_read(char* data){
    for(int i = 0; i < strlen(data); i++)
        if(data[i] == '%')
            return true;
    return false;
  }
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this,self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            data_[length] = '\0';
            cout << session_id << endl<< data_ << endl;
            string command;
            if(check_read(data_)){
                output_shell(string(data_));
                if(getline(filestream, command)){
                    input_handler(command);
                    command = command + "\n";
                    strcpy(data_, command.c_str());
                    output_command(command);
                }
                do_write(strlen(data_));
            }else{
                output_shell(string(data_));
                do_read();
            }

          }else{
            cout << "read error" << endl;
            cout << ec.message() << endl;
            socket_.close();
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    socket_.async_write_some(boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t len)
        {
          if (!ec)
          {
            if(data_[0] == 'e' && data_[1] == 'x' && data_[2] == 'i' && data_[3] == 't')
                socket_.close();
            else{
                do_read();
            }
          }
        });
  }

  std::shared_ptr<tcp::socket> b_socket;
  tcp::socket socket_;
  tcp::endpoint endpoint_;
  tcp::resolver resolver;
  string host;
  string file;
  string port;
  int session_id;
  fstream filestream;

  bool valid;
  enum { max_length = 7500 };
  char data_[7500];
  char data_b[7500];
};


class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket)
    : socket_(std::make_shared<tcp::socket>(std::move(socket)))
  {
  }

  void start()
  {
    do_http_response();
  }

private:
  void send_header(){
    auto self(shared_from_this());
	string str = "HTTP/1.1 200 OK\r\n";
    strcpy(data_, str.c_str());
    cout << data_ << endl;

    socket_->async_write_some(
			boost::asio::buffer(data_, strlen(data_)),
			[](boost::system::error_code ec, std::size_t len) {


		});
  }

  void do_panel(){
    string str = "Content-type: text/html\r\n\r\n\0";
    strcpy(data_, str.c_str());
    socket_->async_write_some(
			boost::asio::buffer(data_, strlen(data_)),
			[](boost::system::error_code ec, std::size_t len) {
		});

    fstream html;
    html.open("panel.html");
    string input, tmp;
    while(getline(html, tmp)){
        input = input + tmp;
    }
    input = input + "\0";
    strcpy(data_, input.c_str());
    socket_->async_write_some(
			boost::asio::buffer(data_, strlen(data_)),
			[](boost::system::error_code ec, std::size_t len) {
                if(!ec){

                }
		});
  }

  void do_console(){
    string str = "Content-type: text/html\r\n\r\n\0";
    strcpy(data_, str.c_str());
    socket_->async_write_some(
			boost::asio::buffer(data_, strlen(data_)),
			[](boost::system::error_code ec, std::size_t len) {});

    fstream html;
    html.open("base_html.html");
    string input, tmp;
    while(getline(html, tmp)){
        input = input + tmp;
    }
    input = input + "\0";
    strcpy(data_, input.c_str());
    socket_->async_write_some(
			boost::asio::buffer(data_, strlen(data_)),
			[this](boost::system::error_code ec, std::size_t len) {
                    if(!ec){
                    }
            });

    string query_str = string(getenv("QUERY_STRING"));
    string query = split(query_str, '?')[1];
    vector<string>ar = split(query, '&');
    boost::asio::io_service io_context_test;
    // tcp::resolver resolver(io_context_test);
    tcp::endpoint endpoint;


    int session_id = 0;

    // a = std::move(socket_);

    std::shared_ptr<tcp::socket> sock(socket_);
    for(int i = 0; i < ar.size(); i++){
        string host = get_value(ar[i++]);
        string port = get_value(ar[i++]);
        string file = get_value(ar[i]);
        int id = session_id ++;
        input_handler(host);
        input_handler(port);
        tcp::resolver::query query(host, port);
        cout << host << " " << port << endl;
        if(host == "")
          continue;

        std::shared_ptr<shell_connect> s = std::make_shared<shell_connect>(sock,io_context_test, file, id, host, port);
        s->start();
    }
    io_context_test.run();
    return;
  }

  void do_http_response(){
    auto self(shared_from_this());
    socket_->async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            string data = string(data_);
            input_handler(data);
            ENV *env = new ENV(data, client_addr, client_port);
            env->print_env();
            string query = getenv("QUERY_STRING");
            send_header();
            if(query.find("panel.cgi")!=std::string::npos){
                do_panel();
            }else if(query.find("console.cgi") != std::string::npos){
                do_console();
            }
          }
        });
  }
  std::shared_ptr<tcp::socket> socket_;
  enum { max_length = 15000 };
  char data_[15000];
};

class server
{
public:
  server(short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
  {
    // tcp::socket tmp(io_context);
    // socket_ = make_shared<tcp::socket>(std::move(tmp));
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
    server s(std::atoi(argv[1]));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
