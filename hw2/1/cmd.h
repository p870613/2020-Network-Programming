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
using namespace std;

class COMMAND{
    private:
        vector<string>arg;
        string output_file;
    public:
        COMMAND(string input);
        void print();
        bool exe_cmd();
        bool build_in_system_command();
        void exe_build_in_system_cmd(ENV *e);
        int output_number_pipe();
};
#endif
