#ifndef EXE_H
#define EXE_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <boost/asio.hpp>
using namespace std;
class EXE_CGI{
    private:
        string cgi_name;
        string query;
    public:
        EXE_CGI(string);
        bool exe();
        void exe_parse(string);

};
#endif

