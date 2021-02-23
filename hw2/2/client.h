#ifndef CLIENT_H
#define CLIENT_H

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
#include <algorithm>
#include "number_pipe.h"
#include "user_pipe.h"
using namespace std;
class Client{
    private:
        string env;
        string name;
        int fd;
        string ip_addr;
        int port;
        int id;
        vector<Number_Pipe> number_pipe;
        static int global_id;
    public:
        Client(int fd, string, int);
        void client_exit(fd_set*, fd_set*);
        void talk();
        void change_name(string);
        int get_fd();
        string get_ip();
        int get_port();
        string get_name();
        int get_id();
        void change_env(string);
        vector<Number_Pipe>* get_number_pipe();
        string get_env();
        void set_id(int);
};

class Client_Mananger{
    private:
        fd_set *rfds;
        fd_set *afds;
        vector<Client>total;
        vector<User_Pipe>user_pipe;
        bool id_map[30];
    public:
        Client_Mananger(fd_set*, fd_set*);
        void add(Client);
        void del(int);
        void broadcast(int, string);
        Client find_client(int fd);
        fd_set* get_afds();
        fd_set* get_rfds();
        void cmd_who(int fd);
        void print();
        void cmd_change_name(int, string);
        Client* get_client(int index);
        int get_index(int);
        bool check_name(string);
        bool check_id(int);
        int get_fd_from_id(int);

        bool check_user_pipe_id_in();
        bool check_user_pipe_id_out();
        bool  add_user_pipe(int, int);
        //void add_user_pipe(User_Pipe);
        void del_user_pipe(int, int);
        User_Pipe* find_user_pipe(int, int);
        int user_size(){
            return user_pipe.size();
        }
        bool check_repeat(int, int);

        int occupy_id();
        void free_id(int);

        bool check_user_pipe_id(int);

        string from_id_get_name(int);
        void print_user_pipe();
        void free_user_pipe(int);
};
#endif
