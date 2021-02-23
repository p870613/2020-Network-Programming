#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <fstream>

using boost::asio::ip::tcp;
using boost::algorithm::replace_all;
using namespace std;

//void output_command(int id, string content){
    //string session_id = "s" + to_string(id);
    //string out = "<script>document.getElementById('"+ session_id + "').innerHTML += '<b>" + content +"</b>';</script>\n";
    //replace_html_char(out);
    //cout << out;
    //cout.flush();
//}
//void output_shell(int id, string content){
    //string session_id = "h" + to_string(id);
    //string out = "<script>document.getElementById('"+ session_id + "').innerHTML +=" + content+";</script>";
    //replace_html_char(out);
    //cout << out;
    //cout.flush();
//}
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

void output_command(int id, string content){
    replace_html_char(content);
    string session_id = "s" + to_string(id);
    replace_all(content, "\n", "&#13");
    replace_all(content, "\r", "");
    boost::format fmt("<script>document.getElementById('%1%').innerHTML += '%2%';</script>");
    cout << fmt%session_id%content;
    cout.flush();
}
void output_shell(int id, string content){
    replace_html_char(content);
    string session_id = "s" + to_string(id);
    replace_all(content, "\n", "&#13");
    replace_all(content, "\r", "");
    boost::format fmt("<script>document.getElementById('%1%').innerHTML += '<b>%2%</b>';</script>");
    cout << fmt%session_id%content;
    cout.flush();
}
void output_host(int id, string content){
    replace_html_char(content);
    string session_id = "h" + to_string(id);
    replace_all(content, "\n", "&#13");
    replace_all(content, "\r", "");
    boost::format fmt("<script>document.getElementById('%1%').innerHTML += '%2%';</script>");
    cout << fmt%session_id%content;
    cout.flush();
}


void output_hello(){
    string a = "hello\n";
    replace_html_char(a);
    for(int id = 0; id < 5; id++){
        string session_id = "s" + to_string(id);
        replace_all(a, "\n", "&#13");
        replace_all(a, "\r", "");
        boost::format fmt("<script>document.getElementById('%1%').innerHTML += '%2%';</script>");
        cout << fmt%session_id%a;
        cout.flush();
    }
}

void print_base_html(){
    cout << "Content-type: text/html\r\n\r\n";
    //cout.flush();
    //fstream html;
    //html.open("base_html.html");
    //string input, tmp;
    //while(getline(html, tmp)){
        //input = input + tmp;
    //}
    //cout << input;
    cout << "<!DOCTYPE html>"<< endl;
    cout << "<html lang=\"en\">"<< endl;
    cout << "  <head>"<< endl;
    cout << "    <meta charset=\"UTF-8\" />"<< endl;
    cout << "    <title>NP Project 3 Sample Console</title>"<< endl;
    cout << "    <link"<< endl;
    cout << "      rel=\"stylesheet\""<< endl;
    cout << "      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\""<< endl;
    cout << "      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\""<< endl;
    cout << "      crossorigin=\"anonymous\""<< endl;
    cout << "    />"<< endl;
    cout << "    <link"<< endl;
    cout << "      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""<< endl;
    cout << "      rel=\"stylesheet\""<< endl;
    cout << "    />"<< endl;
    cout << "    <style>"<< endl;
    cout << "      * {"<< endl;
    cout << "        font-family: 'Source Code Pro', monospace"<< endl;
    cout << "        font-size: 1rem !important"<< endl;
    cout << "      }"<< endl;
    cout << "      body {"<< endl;
    cout << "        background-color: #212529"<< endl;
    cout << "      }"<< endl;
    cout << "      pre {"<< endl;
    cout << "        color: #cccccc"<< endl;
    cout << "      }"<< endl;
    cout << "      b {"<< endl;
    cout << "        color: #01b468"<< endl;
    cout << "      }"<< endl;
    cout << "    </style>"<< endl;
    cout << "  </head>"<< endl;
    cout << "  <body>"<< endl;
    cout << "    <table class=\"table table-dark table-bordered\">"<< endl;
    cout << "      <thead>"<< endl;
    cout << "        <tr>"<< endl;
    cout << "          <th id=\"h0\"></th>"<< endl;
    cout << "          <th id=\"h1\"></th>"<< endl;
    cout << "          <th id=\"h2\"></th>"<< endl;
    cout << "          <th id=\"h3\"></th>"<< endl;
    cout << "          <th id=\"h4\"></th>"<< endl;
    cout << "        </tr>"<< endl;
    cout << "      </thead>"<< endl;
    cout << "      <tbody>"<< endl;
    cout << "        <tr>"<< endl;
    cout << "          <td><pre id=\"s0\" class=\"mb-0\"></pre></td>"<< endl;
    cout << "          <td><pre id=\"s1\" class=\"mb-0\"></pre></td>"<< endl;
    cout << "          <td><pre id=\"s2\" class=\"mb-0\"></pre></td>"<< endl;
    cout << "          <td><pre id=\"s3\" class=\"mb-0\"></pre></td>"<< endl;
    cout << "          <td><pre id=\"s4\" class=\"mb-0\"></pre></td>"<< endl;
    cout << "        </tr>"<< endl;
    cout << "      </tbody>"<< endl;
    cout << "    </table>"<< endl;
    cout << "  </body>"<< endl;
    cout << "</html>"<< endl;
    cout.flush();
}

unsigned char string_to_ip(const char* a){
    unsigned char re = 0;
    for(int i = 0; i < strlen(a); i++){
        re = re * 10+ a[i] - '0';
    }
    return re;
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

class np_session
  : public std::enable_shared_from_this<np_session>
{
public://(boost::asio::io_context &io_context
  np_session(boost::asio::io_context &io_context, tcp::endpoint& t, string f, int id, string h, string p, string sock_host, string sock_port) 
      :socket_(io_context),
       endpoint_(std::move(t)),
       timer(io_context),
       resolver(io_context)
  {

    host = h;
    port = p;
    file = "test_case/" + f;
    session_id = id;
    
    if(host != ""){
        valid = 1;
        unsigned char req[8];
        string tmp_ip = t.address().to_string();
        vector<string>ar = split(tmp_ip, '.');
        int tmp_port = t.port();

        req[0] = 4;
        req[1] = 1;

        req[4] =  string_to_ip(ar[0].c_str());
        req[5] =  string_to_ip(ar[1].c_str());
        req[6] =  string_to_ip(ar[2].c_str());
        req[7] =  string_to_ip(ar[3].c_str());

        req[2] = tmp_port >> 8;
        req[3] = tmp_port % (1 << 8);
        
        filestream.open(file);
        cout << file << endl;
        
        boost::asio::connect(socket_, resolver.resolve(sock_host,  sock_port));
         
        write(socket_.native_handle(),req,8);

        unsigned char rec[8];
        read(socket_.native_handle(), rec, 8);
    }
    else{
        valid = 0;
    }
    
  }

  void start(){
      if(valid == 1){
        output_host(session_id, host + ":" + port);
        do_read();
      }
  }


private:
  void do_send_hello(){
    auto self(shared_from_this());
    timer.expires_from_now(boost::posix_time::seconds(2));

    timer.async_wait([this, self](boost::system::error_code ec){
                if(!ec){
                    output_hello();
                    do_send_hello();
                }
            });
  }
  void do_connect(){
      auto self(shared_from_this());
      socket_.async_connect(endpoint_, 
              [this, self](boost::system::error_code ec)
      {
          if(!ec){
              filestream.open(file);
              //do_send_hello();
              //do_read();
          }
          else{
            cout << "fail to connect server: " << host << endl; 
          }
      });
      
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
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec) 
          {
            data_[length] = '\0';
            string command;
            if(check_read(data_)){
                output_shell( session_id, string(data_));
                if(getline(filestream, command)){
                    command = command + "\n";
                    strcpy(data_, command.c_str());
                    output_command(session_id, string(data_)+"\n");
                }
                do_write(strlen(data_));
            }else{
                output_shell(session_id, string(data_));
                do_read();
            }

          }else{
            cout << "read error" << endl;
            socket_.close();
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    socket_.async_write_some(boost::asio::buffer(&data_[0], length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            if(data_[0] == 'e' && data_[1] == 'x' && data_[2] == 'i' && data_[3] == 't')
                socket_.close();
            else
                do_read();
          }
        });
  }
  

  tcp::socket socket_;
  tcp::endpoint endpoint_;
  boost::asio::deadline_timer timer;
  tcp::resolver resolver;
  string host;
  string file;
  string port;
  int session_id;
  fstream filestream;

  bool valid;
  enum { max_length = 15000 };
  char data_[15000];
};



int main(void)
{
  try
  {
    string query_str = string(getenv("REQUEST_URI"));
    if(query_str.find("/") >= 0)
        query_str.erase(query_str.begin() + query_str.find("/"));
    //string query_str = "console.cgi?h0=nplinux12.cs.nctu.edu.tw&p0=8765&f0=t1.txt&h1=&p1=&f1=&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=";
    string query = split(query_str, '?')[1];
    vector<string>ar = split(query, '&');
    
    boost::asio::io_context io_context;
    tcp::endpoint endpoint;
    tcp::resolver resolver(io_context);
    int session_id = 0;
    print_base_html();
    
    string sock_host = get_value(ar[ar.size() - 2]);
    string sock_port = get_value(ar[ar.size() - 1]);

    //tcp::socket t(io_context);
    //boost::asio::connect(t, resolver.resolve(sock_host, sock_port));
    
    //std::shared_ptr<tcp::socket> sock_socket(std::make_shared<tcp::socket>(std::move(t)));
    //boost::asio::connect(sock_socket, resolver.resolve(sock_host, sock_port));
    print_base_html();

    for(int i = 0; i < ar.size()-2 ; i++){
        string host = get_value(ar[i++]);
        string port = get_value(ar[i++]);
        string file = get_value(ar[i]);
        int id = session_id ++;

        //if(host != "")
            //output_host(id, host+":"+port);

        tcp::resolver::query query(host, port);
        
        resolver.async_resolve(query, [&endpoint, &io_context, file, id, host, port, sock_host, sock_port](boost::system::error_code ec,
                    tcp::resolver::iterator iter){
            if(!ec){
                cout.flush();
                endpoint = std::move(iter->endpoint());
                std::shared_ptr<np_session> s = std::make_shared<np_session>(io_context, endpoint, file, id, host, port, sock_host, sock_port);
                s->start();
            }
            else{
                cout.flush();
            }
        });     
    }
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
