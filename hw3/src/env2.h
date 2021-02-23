#ifndef ENV_H
#define ENV_H
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>
using namespace std;
class ENV{
    private:
        string env_var[9];
        string var_name[9] = {"REQUEST_METHOD",
                               "REQUEST_URI",
                               "QUERY_STRING",
                               "SERVER_PROTOCOL",
                               "HTTP_HOST",
                               "SERVER_ADDR",
                               "SERVER_PORT",
                               "REMOTE_ADDR",
                               "REMOTE_PORT"};
        enum state{
            method,
            url,
            query,
            protocol,
            host,
            server_addr,
            server_port,
            remote_addr,
            remote_port
        }state_;
    public:
        ENV(string, string, int);
        void env_parse(string);
        void senv();
        void print_env();

};

#endif
