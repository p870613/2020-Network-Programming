#ifndef NUMBER_PIPE_H
#define NUMBER_PIPE_H

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

class Number_Pipe{
    private:
        int fd[2];
        int number;
    public:
        Number_Pipe(int read, int write, int count);
        bool is_pop();

        int get_pipe_num();
        int get_fd_read();

        int get_fd_write();
};
#endif


