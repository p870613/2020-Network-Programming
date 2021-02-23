#include "number_pipe.h"
Number_Pipe::Number_Pipe(int read, int write, int count){
    fd[0] = read;
    fd[1] = write;
    number = count;
}

bool Number_Pipe::is_pop(){
    number --;
    if(number == 0)
        return true;
    return false;
}

int Number_Pipe::get_pipe_num(){
    return number;
}

int Number_Pipe::get_fd_read(){
    return fd[0];
}

int Number_Pipe::get_fd_write(){
    return fd[1];
}
