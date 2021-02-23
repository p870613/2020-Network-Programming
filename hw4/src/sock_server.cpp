#include "sock_server.h"

#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <cstdio>
#include <string>
using namespace std;
using boost::asio::ip::tcp;

boost::asio::io_context io_service;

void packet_handler(unsigned char *data, struct packet_format &req){
    req.vn = data[0];
    req.cd = data[1];
    req.port = ((unsigned int)data[2])<<8 | ((unsigned int)data[3]);
    req.ip = ((unsigned int)data[4])<<24| ((unsigned int)data[5])<< 16 | ((unsigned int)data[6])<<8 | ((unsigned int)data[7]);
}

string socks4a_domain_name(unsigned char *data, int len){
    int counter = 0;
    string re = "";
    for(int i = len-1; i >= 0; i--){
        if(data[i] == 0){
            counter ++;
        }
        if(counter == 1){
            unsigned char a[1] = {data[i]};
            re = string((char*)a) + re;
        }

        if(counter == 2)
            break;
    }
    return re;

}
server::server(tcp::socket s)
    :signal_(io_service),
    acceptor_(io_service),
    resolver_(io_service),
    source_socket_(std::move(s)),
    des_socket_(io_service){}

void server::start(){
    auto self(shared_from_this());
    receive_sock_header();
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

bool firewall(unsigned int *a, int status){
    fstream in("socks.conf");
    string input;
    bool re = true;
    while(getline(in, input)){
        vector<string>ar = split(input, ' ');
        if(status == 1 && ar[1] == "c"){
            vector<string>ip = split(ar[2], '.');
            int i;
            for(i = 3; i >= 0; i--){
                if(ip[i] == "*" || ip[i] == to_string(a[i]))
                    continue;
                else{
                    re = false;
                    break;
                }
            }
            if(i == -1)
                re = true;
            if(re == true)
                return true;
        }else if(status == 2 && ar[1] == "b"){
            vector<string>ip = split(ar[2], '.');
            int i;
            for(i = 3; i >= 0; i--){
                if(ip[i] == "*" || ip[i] == to_string(a[i]))
                    continue;
                else{
                    re = false;
                    break;
                }
            } 
            if(i == -1)
                re = true;

            if(re == true)
                return true;
        }

    }
    return re;
}

void server::receive_sock_header(){
    auto self(shared_from_this());
    source_socket_.async_read_some(boost::asio::buffer(source_data, max_length),
            [this, self](boost::system::error_code ec, std::size_t len){
                if(!ec){
                    struct packet_format sock_req;
                    packet_handler(source_data, sock_req);
                    string remote_ip = source_socket_.remote_endpoint().address().to_string();
                    unsigned short remote_port = source_socket_.remote_endpoint().port();
                    
                    cout << "<S_IP>:" << remote_ip << endl;
                    cout << "<S_PORT>" << remote_port << endl;

                    if((sock_req.ip>>24) == 0 && ((sock_req.ip%(1<<24))>>16) == 0 && ((sock_req.ip%(1<<16))>>8) == 0){

                    }else{
                        printf("<D_IP>:%u.%u.%u.%u\n",(sock_req.ip>>24),((sock_req.ip%(1<<24))>>16),((sock_req.ip%(1<<16))>>8),((sock_req.ip%(1<<8))));
                        printf("<D_PORT>:%u\n", sock_req.port);
                    }
                    
                    //fireware
                    //flag pass
                    
                    unsigned int ar[4] = {(sock_req.ip>>24),((sock_req.ip%(1<<24))>>16),((sock_req.ip%(1<<16))>>8),((sock_req.ip%(1<<8)))};
                    if(sock_req.cd == 1){

                        string host = to_string((sock_req.ip>>24)) + "."  + to_string((sock_req.ip%(1<<24))>>16) + "." + to_string(((sock_req.ip%(1<<16))>>8)) + "."+  to_string(((sock_req.ip%(1<<8))));
                        string port = to_string(sock_req.port);

                        if(host.find("0.0.0.") == std::string::npos){
                            
                            cout << "<COMMAND>:CONNECT" << endl;
                            if(firewall(ar, 1)){
                                cout << "<Reply>:Accept" << endl;
                                do_connect(host, port);
                            }else{
                                cout << "<Reply>:Reject" << endl;
                                exit(0);
                            }
                        }else{
                            host = socks4a_domain_name(source_data, len);
                            tcp::resolver::query query(host, port);

                            resolver_.async_resolve(query, 
                                [this, self](boost::system::error_code ec, tcp::resolver::iterator iter){
                                    if(!ec){
                                        string ip = iter->endpoint().address().to_string();
                                        int tmp_port = iter->endpoint().port();
                                        vector<string> ar = split(ip, '.');
                                        unsigned int tmp[4] = {(unsigned int)stoi(ar[0]), (unsigned int)stoi(ar[1]), (unsigned int)stoi(ar[1]), (unsigned int)stoi(ar[1])};
                                       if(!firewall(tmp, 1)){
                                            cout << "<D_IP>:" << ip << endl;
                                            cout << "<D_PORT>:" << tmp_port << endl;
                                            cout << "<COMMAND>:CONNECT" << endl;
                                            cout << "<Reply>:Reject" << endl;
                                            exit(0);
                                            return;
                                       }else{
                                            cout << "<D_IP>:" << ip << endl;
                                            cout << "<D_PORT>:" << tmp_port << endl;
                                            cout << "<COMMAND>:CONNECT" << endl;
                                            cout << "<Reply>:Accept" << endl;
                                       }
                                    }
                            });
                            do_connect(host, port);
                        }
                    } else if(sock_req.cd == 2){
                        cout << "<COMMAND>:BIND" << endl;
                        bool pass = firewall(ar, sock_req.cd);
                        if(pass == true){
                            cout << "<Reply>:Accept" << endl;
                            std::make_shared<bind_session>(std::move(source_socket_))->start();
                        }else{
                            cout << "<Reply>:Reject" << endl;
                            exit(0);
                        }
                    }
                }
            });
}

void server::do_connect(string host, string port){
    auto self(shared_from_this());
    tcp::resolver::query query(host, port);
    resolver_.async_resolve(query, 
            [this, self](boost::system::error_code ec, tcp::resolver::iterator iter){
                if(!ec){
                    auto self2(shared_from_this());
                    des_socket_.async_connect(*iter, 
                            [this, self](const boost::system::error_code& ec)
                            {
                                if(!ec){
                                    send_sock_header();
                                }else{
                                    cout << "connect error" << endl;
                                }
                            });
                }else{
                    cout << "resolver error" << endl;
                }
        });
}

void server::send_sock_header(){
    auto self(shared_from_this());
    
    source_data[0] = 0;
    source_data[1] = 90;
    source_socket_.async_write_some(boost::asio::buffer(source_data,8),
       [this,self](boost::system::error_code ec, std::size_t length){
         if(!ec){
           do_read_from_client();
           do_read_from_server();
         }else{
            cout << "send_sock_header write error" << endl;
         } 
       });
}

void server::do_read_from_client(){
    auto self(shared_from_this());
    source_socket_.async_read_some(boost::asio::buffer(source_data,max_length),
       [this,self](boost::system::error_code ec , std::size_t length){
         if(!ec){
           do_write_to_server(length);
         }else{
            //cout << "do read from client error" << endl;
            source_socket_.close();
            des_socket_.close();
            exit(0);
         }
       });
}

void server::do_read_from_server(){
    auto self(shared_from_this());
    des_socket_.async_read_some(boost::asio::buffer(des_data,max_length),
       [this,self](boost::system::error_code ec , std::size_t length){
         if(!ec){
           do_write_to_client(length);
         }else{
            //cout << "do read from server error" << endl;
            source_socket_.close();
            des_socket_.close();
            exit(0);
         }
       });
}

void server::do_write_to_client(std::size_t len){
    auto self(shared_from_this());
    source_socket_.async_write_some(boost::asio::buffer(des_data,len),
       [this,self](boost::system::error_code ec , std::size_t length){
         if(!ec){
           do_read_from_server();
         }else{
            cout << "do write to client error" << endl;
         }
       });
}

void server::do_write_to_server(std::size_t len){
    auto self(shared_from_this());
    des_socket_.async_write_some(boost::asio::buffer(source_data,len),
        [this,self](boost::system::error_code ec , std::size_t length){
        if(!ec){
            do_read_from_client();
        }else{
            cout << "do write to server error" << endl;
        }
    });
}

/*bind_session*/
bind_session::bind_session(tcp::socket s)
    :ftp_client_socket_(std::move(s)),
    ftp_server_socket_(io_service),
    acceptor_(io_service, tcp::endpoint(tcp::v4(), 0)){}

void bind_session::start(){
    send_sock_header();
}

void bind_session::send_sock_header(){
    auto self(shared_from_this());

    unsigned short port = acceptor_.local_endpoint().port();
    to_client_data[0] = 0;
    to_client_data[1] = 90;
    to_client_data[2] = port >> 8;
    to_client_data[3] = port % (1 << 8);
    to_client_data[4] = 0;
    to_client_data[5] = 0;
    to_client_data[6] = 0;
    to_client_data[7] = 0;

    ftp_client_socket_.async_write_some(boost::asio::buffer(to_client_data, 8),
            [this, self](boost::system::error_code ec, std::size_t len)
            {
                if(!ec){
                    do_accept();
                }else{
                    cout << "bind send sock header error" << endl;
                }
            });
}

void bind_session::do_accept(){
    auto self(shared_from_this());

    acceptor_.async_accept(ftp_server_socket_,
            [this, self](boost::system::error_code ec)
            {
                if(!ec){
                    do_read_from_client();
                    do_read_from_server();
                }else{
                    cout << "bind accept error" << endl;
                }
            });
}

void bind_session::do_read_from_client(){
    auto self(shared_from_this());
    
    ftp_client_socket_.async_read_some(boost::asio::buffer(to_server_data, max_length),
        [this, self](boost::system::error_code ec, std::size_t len){
            if(!ec){
                do_write_to_server(len);
            }else{

                ftp_client_socket_.close();
                ftp_server_socket_.close();
                //if(ec.message() == "End of file")
                    //do_write_to_server(len);
                //cout << "bind do read from client error" << endl;
                //cout << ec.message() << endl;
            }
        });
}

void bind_session::do_read_from_server(){
    auto self(shared_from_this());
    
    ftp_server_socket_.async_read_some(boost::asio::buffer(to_client_data, max_length),
        [this, self](boost::system::error_code ec, std::size_t len){
            if(!ec){
                do_write_to_client(len);
            }else{

                ftp_client_socket_.close();
                ftp_server_socket_.close();
                //if(ec.message() == "End of file")
                    //do_write_to_client(len);
                //cout << ec.message() << endl;
                //cout << "bind do read from server error" << endl;
            }
        });
}

void bind_session::do_write_to_client(std::size_t len){
    auto self(shared_from_this());

    ftp_client_socket_.async_write_some(boost::asio::buffer(to_client_data, len),
            [this, self](boost::system::error_code ec, std::size_t length)
            {
                if(!ec){
                    do_read_from_server();
                }else{
                    ftp_client_socket_.close();
                    ftp_server_socket_.close();
                }
            });
}

void bind_session::do_write_to_server(std::size_t len){
    auto self(shared_from_this());

    ftp_server_socket_.async_write_some(boost::asio::buffer(to_server_data, len),
            [this, self](boost::system::error_code ec, std::size_t length)
            {

                //if(length == 0){
                    //ftp_client_socket_.close();
                    //ftp_server_socket_.close();
                //}else 
                if(!ec){
                    do_read_from_client();
                }else{
                    ftp_client_socket_.close();
                    ftp_server_socket_.close();
                }
            });
}

/*init_server*/
init_server::init_server(unsigned short port)
    :acceptor_(io_service, tcp::endpoint(tcp::v4(), port)){
        do_accept();
    }

void init_server::do_accept(){
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket){
                if(!ec){
                    //std::make_shared<server>(std::move(socket))->start();
                    //do_accept();
                    io_service.notify_fork(boost::asio::io_context::fork_parent);
                    if(fork() == 0){
                        io_service.notify_fork(boost::asio::io_context::fork_child);
                        acceptor_.close();
                        std::make_shared<server>(std::move(socket))->start();
                        
                    }else{
                        io_service.notify_fork(boost::asio::io_context::fork_parent);
                        do_accept();
                    }
                }else{
                    do_accept();
                    cout << "error" << endl;
                }
    });
}

int main(int argc, char *argv[])
{
    if(argc != 2){
        cout << "Usage: ./socks_server <port>" << endl;
        return 1;
    }
    try{
		init_server s(atoi(argv[1]));
		io_service.run();
    }catch(std::exception& ec){
        cout << ec.what();
    }
}
