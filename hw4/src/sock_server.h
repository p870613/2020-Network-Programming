#include <sys/wait.h>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using boost::asio::ip::tcp;

struct packet_format{
    unsigned char vn;
    unsigned char cd;
    unsigned int port;
    unsigned int ip;
};
class server
    :public std::enable_shared_from_this<server>
{
    private:

        boost::asio::signal_set signal_;
        tcp::resolver resolver_;
        tcp::acceptor acceptor_;
        tcp::socket source_socket_;
        tcp::socket des_socket_;

        unsigned char source_data[15000];
        unsigned char des_data[15000];
        enum {max_length = 15000};

        unsigned short port;
        
        void receive_sock_header();
        void do_connect(string, string);
        void send_sock_header();
        
        void do_read_from_client();
        void do_read_from_server();
        void do_write_to_server(std::size_t);
        void do_write_to_client(std::size_t);
        
    public:
        server(tcp::socket);
        void start();

};

class bind_session
    : public std::enable_shared_from_this<bind_session>
{
    private:
        tcp::socket ftp_client_socket_;
        tcp::socket ftp_server_socket_;
        tcp::acceptor acceptor_;

        enum {max_length = 15000};
        unsigned char to_client_data[15000];
        unsigned char to_server_data[15000];

        void send_sock_header();
        void do_accept();

        void do_read_from_client();
        void do_read_from_server();
        void do_write_to_server(std::size_t);
        void do_write_to_client(std::size_t);
    public:
        bind_session(tcp::socket);
        void start();
};

class connect_session{

};

class init_server{
    public:
        init_server(unsigned short);
    private:
        void do_accept();

        tcp::acceptor acceptor_;
};
