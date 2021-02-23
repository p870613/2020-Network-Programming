#include "client.h"

#include <cstdlib>
#include <cstdio>
int Client::global_id = 1;

Client::Client(int fd, string ip, int p){
    this->fd = fd;
    name = "(no name)";
    env = "bin:.";
    this->ip_addr = ip;
    port = p;
}

void Client::client_exit(fd_set *afds, fd_set* rfds){
    close(fd);
    FD_CLR(fd, afds);
}

void Client::change_name(string new_name){
    name = new_name;
}

void Client::change_env(string new_evn){
    env = new_evn;
}
int Client::get_fd(){
    return fd;
}

string Client::get_ip(){
    return ip_addr;
}

int Client::get_port(){
    return port;
}

string Client::get_name(){
    return name;
}

int Client::get_id(){
    return id;
}

string Client::get_env(){
    return env;
}
vector<Number_Pipe>* Client::get_number_pipe(){
    return &number_pipe;
}

void Client::set_id(int n){
    id = n;
}

Client Client_Mananger::find_client(int fd){
    for(int i = 0; i < total.size(); i++)
        if(total[i].get_fd() == fd)
            return total[i];
}

Client_Mananger::Client_Mananger(fd_set* rfds, fd_set* afds){
    this->afds = afds;
    this->rfds = rfds;
    for(int i = 0; i < 30; i++)
        id_map[i] = 0;
}
bool cmp(Client a, Client b){
    return a.get_id() < b.get_id();
}
void Client_Mananger::add(Client add_node){
    add_node.set_id(occupy_id());
    total.push_back(add_node);
    sort(total.begin(), total.end(), cmp);
    string broadcast_str = "*** User '" + add_node.get_name() + "' entered from " + add_node.get_ip() + ":" + to_string(add_node.get_port())+ ". ***\n";
    string str = "****************************************\n** Welcome to the information server. **\n****************************************\n";
    string a = "% ";
    write(add_node.get_fd(), str.c_str(), strlen(str.c_str()));
    this->broadcast(-1, broadcast_str);
    write(add_node.get_fd(), a.c_str(), strlen(a.c_str()));
}

void Client_Mananger::del(int fd){
    for(int i = 0; i < total.size(); i++){
        if(total[i].get_fd() == fd){
            this->free_id(total[i].get_id());
            this->free_user_pipe(total[i].get_id());
            total.erase(total.begin() + i);
            break;
        }
    }
}

void Client_Mananger::free_user_pipe(int id){
    for(int i = 0; i < user_pipe.size(); i++){
        if(user_pipe[i].get_in_id() == id){
            this->del_user_pipe(id, user_pipe[i].get_out_id());
        }
        if(user_pipe[i].get_out_id() == id){
            this->del_user_pipe(user_pipe[i].get_in_id(), id);
        }

    }
}


fd_set* Client_Mananger::get_afds(){
    return afds;
}


fd_set* Client_Mananger::get_rfds(){
    return rfds;
}

void Client_Mananger::cmd_who(int fd){
    string str = string("<ID>\t") + "<nickname>\t" + "<IP:port>\t" + "<indicate me>\n"; 
    const char *t = str.c_str();
    write(fd, t, strlen(t));

    for(int i = 0; i < total.size(); i++){
        str =  to_string(total[i].get_id()) + "\t" +total[i].get_name() + "\t" + total[i].get_ip() + ":" +  to_string(total[i].get_port());

    if(total[i].get_fd() == fd){
            str = str  + "\t<-me";
        }
        str = str + "\n";
        const char *tmp = str.c_str();
        write(fd, tmp, strlen(tmp));
    }
}

void Client_Mananger::print(){
    for(int i = 0; i < total.size(); i++)
        cout << total[i].get_port() << endl;
}

void Client_Mananger::broadcast(int fd, string str){
    const char *out = str.c_str();
    for(int i = 0; i < total.size(); i++){
        if(total[i].get_fd() != fd){
            write(total[i].get_fd(), out, strlen(out));
        }
    }
}

void Client_Mananger::cmd_change_name(int fd, string name){
    for(int i = 0; i < total.size(); i++){
        if(total[i].get_fd() == fd){
            total[i].change_name(name);
            string str = "*** User from " + total[i].get_ip() + ":" + to_string(total[i].get_port()) + " is named '" + name + "'. ***\n";
            const char *tmp = str.c_str();
            write(fd, tmp, strlen(tmp)); 
            this->broadcast(fd, str);
            break;
        }
    }
}

Client* Client_Mananger::get_client(int index){
    return &total[index];
}

int Client_Mananger::get_index(int fd){
    for(int i = 0; i < total.size(); i++)
        if(total[i].get_fd() == fd)
            return i;
}

bool Client_Mananger::check_name(string str){
    for(int i = 0; i < total.size(); i++){
        if(total[i].get_name() == str)
            return false;
    }
    return true;
}

bool Client_Mananger::check_id(int id){
    for(int i = 0; i < total.size(); i++){
        if(total[i].get_id() == id)
            return true;
    }
    return false;
}

int Client_Mananger::get_fd_from_id(int id){
    for(int i = 0; i < total.size(); i++){
        if(total[i].get_id() == id)
            return total[i].get_fd();
    }
}

bool Client_Mananger::check_repeat(int in, int out){
    for(int i = 0; i < user_pipe.size(); i++){
        if(user_pipe[i].get_in_id() == in && user_pipe[i].get_out_id() == out){
            return false;
        }
    }
    return true;

}
bool Client_Mananger::add_user_pipe(int in, int out){
    if(this->check_repeat(in, out)){
       User_Pipe *new_user = new User_Pipe(in, out);
       user_pipe.push_back(*new_user); 
       return true;
    }
    return false;
}

bool Client_Mananger::check_user_pipe_id(int n){
    if(id_map[n-1] == 1)
        return true;
    return false;
}

User_Pipe* Client_Mananger::find_user_pipe(int in, int out){
    for(int i = 0; i < user_pipe.size(); i++){
        if(user_pipe[i].get_in_id() == in && user_pipe[i].get_out_id() == out){
            return &user_pipe[i];
        }
    }
    return NULL;
}

void Client_Mananger::del_user_pipe(int in, int out){
    for(int i = 0; i < user_pipe.size(); i++){
        if(user_pipe[i].get_in_id() == in && user_pipe[i].get_out_id() == out){
            close(user_pipe[i].get_read_fd());
            close(user_pipe[i].get_write_fd());
            user_pipe.erase(user_pipe.begin() + i);
            break;
        }
    }
}

int Client_Mananger::occupy_id(){
    for(int i = 0; i < 30; i++)
        if(id_map[i] == 0){
            id_map[i] = 1;
            return i + 1;
        }
}

void Client_Mananger::free_id(int n){
    id_map[n-1] = 0;
}

string Client_Mananger::from_id_get_name(int id){
    for(int i = 0; i < total.size(); i++){
        if(total[i].get_id() == id){
            return total[i].get_name();
        }
    }
}

void Client_Mananger::print_user_pipe(){
    for(int i = 0; i < user_pipe.size(); i++){
        cout << user_pipe[i].get_in_id()  << " " << user_pipe[i].get_out_id() << endl;
    }
}
