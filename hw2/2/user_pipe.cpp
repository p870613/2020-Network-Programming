#include "user_pipe.h"


User_Pipe::User_Pipe(int in, int out){
    in_id = in;
    out_id = out;
    del_flag = false;
    if(pipe(fd)){
        std::cout << "user pipe init err";
    }
    std::cout << fd[0] << " " << fd[1] << std::endl;
}

int User_Pipe::get_in_id(){
    return in_id;
}

int User_Pipe::get_out_id(){
    return out_id;
}

int User_Pipe::get_read_fd(){
    return fd[0];
}

int User_Pipe::get_write_fd(){
    return fd[1];
}

bool User_Pipe::get_del_flag(){
    return del_flag;
}

void User_Pipe::res_del_flag(){
    del_flag = true;
}
