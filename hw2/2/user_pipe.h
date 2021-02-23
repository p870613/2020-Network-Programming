#ifndef USER_PIPE_H
#define USER_PIPE_H


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

class User_Pipe{
    private:
        int fd[2];
        int in_id;
        int out_id;
        bool del_flag;
    public:
       User_Pipe(int, int);
       int get_in_id();
       int get_out_id();
       int get_read_fd();
       int get_write_fd();
       bool get_del_flag();
       void res_del_flag();
};
#endif
