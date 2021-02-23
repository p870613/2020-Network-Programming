#ifndef CMD_H
#define CMD_H

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <fcntl.h>
#include <regex>


#include "env.h"
#include "shell.h"
#include "client.h"
#include "user_pipe.h"
using namespace std;

class COMMAND{
    private:
        vector<string>arg;
        string output_file;
        int user_pipe_in_id;
        int user_pipe_out_id;
        bool in_flag;
        bool out_flag;
        bool error_in_flag;
        bool error_out_flag;
        
    public:
        COMMAND(string input, Client*, Client_Mananger*, string);
        void print();
        bool exe_cmd(Client* , Client_Mananger*);
        bool build_in_system_command();
        void exe_build_in_system_cmd(ENV *, Client*, Client_Mananger*, string);
        int output_number_pipe();
        int get_in_id();
        int get_out_id();
        bool get_in_flag();
        bool get_out_flag();
        bool get_error_in_flag(){
            return error_in_flag;
        }
        bool get_error_out_flag(){
            return error_out_flag;
        }
};
#endif
