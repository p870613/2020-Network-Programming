#include "shell.h"
using namespace std;

vector<string>split(const string& s, const char d){
    vector<string> re;
    string tmp;
    stringstream str(s);
    while(getline(str, tmp, d)){
        if(tmp.length() > 0)
            re.push_back(tmp);
    }
    return re;
}

bool is_file(const char *path){
    struct stat s;
    if(stat(path, &s) == 0){
        if(s.st_mode & S_IFREG)
            return true;
    }else{
        cout << path << " has error" << endl;
        return false;
    }
    return false;
}
//change set_evn get_evn
bool is_number_pine(string number){
    vector<string>ar = split(number, ' ');
    if(ar.size() == 1){
        regex reg("[0-9]?[0-9]?[0-9]");
        if(regex_match(ar[0], reg) || ar[0] == "1000")
            return true;
    }
    return false;
}

bool is_execution_file(string input){
    char *tmp = (char*)input.c_str();
    for(int i = strlen(tmp)-1; i >= 0; i--){
        if(tmp[i] == '|')
            if(tmp[i+1] == ' ')
                return false;
            else
                return true;
    }
    return false;
}
void input_handler(string &a){
    a.erase(std::remove(a.begin(), a.end(), '\r'), a.end());
    a.erase(std::remove(a.begin(), a.end(), '\n'), a.end());
    return;  
}
void test(string& a, int fd){
    //write(fd, "% ", 2);
    while(true){
        if(getline(cin, a)){
            input_handler(a);
            if(a == ""){
                write(fd, "% ", 2);
                continue;
            }
            break;
        }
    }
}

bool is_error_number_pipe(string& input){
    char *tmp = (char*)input.c_str();
    for(int i = strlen(tmp) -1; i>=0; i--){
        if(tmp[i] == '!'){
            if(i < strlen(tmp)-1 && i >= 0){
                if(tmp[i-1] == ' ')
                    return true;
                else
                    return false;
            }
        }
    }
    return false;
}
string replace_input(string& input){
    char *tmp = (char*)input.c_str();
    int index = 0;
    for(int i = strlen(tmp) -1; i >= 0; i--){
        if(tmp[i] == '!'){
            index = i;
            break;
        }
    }
    return input.replace(index, 1, 1, '|');
}

void output_to_socket(int sockfd){
    dup2(sockfd, 1);
    dup2(sockfd, 2);
}

void err_to_socket(int sockfd){
    dup2(sockfd, 2);
}
void both_in_out_status(int* out_in_id, int* out_out_id, int* in_in_id, int* in_out_id, string input, Client *cli){
    vector<string>ar = split(input, ' ');
    regex reg_out("[>][0-9]?[0-9]?"); 
    regex reg_in("[<][0-9]?[0-9]?"); 
    for(int i = 0; i < ar.size(); i++){
        if(regex_match(ar[i], reg_in)){
            string tmp = ar[i].replace(ar[i].find("<"), 1, "");
            *in_in_id = atoi(tmp.c_str());
            *in_out_id = cli->get_id();
        }else if(regex_match(ar[i], reg_out)){
            string tmp = ar[i].replace(ar[i].find(">"), 1, "");
            *out_in_id = cli->get_id();
            *out_out_id = atoi(tmp.c_str());
        }
    }
}

void error_user_pipe(COMMAND *p){
    int error_fd = open("/dev/null", O_RDWR| O_APPEND | O_CREAT | O_TRUNC, 0600);
    if(p->get_error_in_flag()){
        dup2(error_fd, 0);
    }
    if(p->get_error_out_flag()){
        dup2(error_fd, 1);
    }
    close(error_fd);
}
void shell(int cur_index, Client_Mananger* man){
    Client *cur_client = man->get_client(cur_index);
    ENV *e = new ENV(cur_client->get_env());
    string input;

    test(input, cur_client->get_fd());
    
    
    vector<Number_Pipe>* number_pipe = cur_client->get_number_pipe();
    vector<Number_Pipe>cur;
    int sta; 

    waitpid(-1, &sta, WNOHANG);

    bool err_number_pipe_flag = false;
    err_number_pipe_flag = is_error_number_pipe(input);
    if(err_number_pipe_flag == true){
        replace_input(input);
    }
    vector<string> cmd = split(input.c_str(), '|');
    /*number pipe*/
    if(number_pipe->size() != 0){
        for(int i = number_pipe->size()-1; i >= 0; i--){
            if(((*number_pipe)[i]).is_pop()){
                cur.push_back((*number_pipe)[i]);
                number_pipe->erase(number_pipe->begin() + i);
            } 
        }
    }
    
    int pi[cmd.size()][2];
    int status = 0;

    /*fd*/
    int sockfd = cur_client->get_fd();

    if(cmd.size() == 1){
        COMMAND *p = new COMMAND(cmd[0], cur_client, man, input);
        
        /*user pipe*/
        bool both_in_out = false;
        int out_in_id, out_out_id;
        int in_in_id, in_out_id;
        if(p->get_in_flag() && p->get_out_flag()){
            both_in_out = true;
            both_in_out_status(&out_in_id, &out_out_id, &in_in_id, &in_out_id, cmd[0], cur_client);
            man->add_user_pipe(out_in_id, out_out_id);
        }else if(p->get_out_flag()){
            man->add_user_pipe(p->get_in_id(), p->get_out_id());
        }

        pid_t pid;
        if(p->build_in_system_command()){
            p->exe_build_in_system_cmd(e, cur_client, man, input);
        }else{
            if(cur.size()!=0){
               close(cur[0].get_fd_write()); 
            }
            pid = fork();
            while(pid < 0){
                waitpid(-1, &status, 0);
                pid = fork();
            }
           
            if(pid  == 0){
                if(both_in_out){
                    /*write*/
                    User_Pipe *cur_user_pipe = man->find_user_pipe(out_in_id, out_out_id);
                    dup2(cur_user_pipe->get_write_fd(), 1);
                    close(cur_user_pipe->get_write_fd());
                    close(cur_user_pipe->get_read_fd());
                    /*read*/
                    User_Pipe *cur_user_pipe2 = man->find_user_pipe(in_in_id, in_out_id);
                    dup2(cur_user_pipe2->get_read_fd(), 0);
                    close(cur_user_pipe2->get_read_fd());
                    close(cur_user_pipe2->get_write_fd());
                }
                else{
                    /*write*/
                    if(p->get_out_flag()){
                        User_Pipe *cur_user_pipe = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                        dup2(cur_user_pipe->get_write_fd(), 1);
                        close(cur_user_pipe->get_write_fd());
                        close(cur_user_pipe->get_read_fd());
                    }else{
                        output_to_socket(sockfd);
                    }
                   
                    /*read*/ 
                    if(cur.size()!=0){
                        dup2(cur[0].get_fd_read(), 0);
                        close(cur[0].get_fd_read());
                        close(cur[0].get_fd_write());
                    }else if(p->get_in_flag()){
                        User_Pipe *cur_user_pipe = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                        dup2(cur_user_pipe->get_read_fd(), 0);
                        close(cur_user_pipe->get_read_fd());
                        close(cur_user_pipe->get_write_fd());
                    }
                }
                error_user_pipe(p);
                p-> exe_cmd(cur_client, man);
                exit(0);
            }else{
                if(both_in_out){
                    User_Pipe *tmp = man->find_user_pipe(in_in_id, in_out_id);
                    close(tmp->get_write_fd());
                    man->del_user_pipe(in_in_id, in_out_id);
                }
                else if(p->get_in_flag()){
                    User_Pipe *tmp = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                    close(tmp->get_write_fd());
                    man->del_user_pipe(p->get_in_id(), p->get_out_id());
                }       

                if(!p->get_out_flag())
                    waitpid(pid, &status, 0);

                if(cur.size()!=0){
                    close(cur[0].get_fd_read());
                    close(cur[0].get_fd_write());
                    cur.pop_back();
                }
            }
        }
    }else if((is_number_pine(cmd[cmd.size()-1]) && is_execution_file(input)) || err_number_pipe_flag == true){
        COMMAND *pipe_cmd = new COMMAND(cmd[cmd.size()-1], cur_client, man, input);
        bool user_pipe = false;
        int in_id, out_id;
        int cur_pipe[2];
        int n = pipe_cmd->output_number_pipe();
        bool flag = 0;
        for(int i = 0; i < number_pipe->size(); i++){
            if(n == (*number_pipe)[i].get_pipe_num()){
                flag = 1;
                cur_pipe[0] = (*number_pipe)[i].get_fd_read();
                cur_pipe[1] = (*number_pipe)[i].get_fd_write();
                break;
            }
        }
        if(flag == 0){
            if(pipe(cur_pipe))
                return;
            Number_Pipe *new_pipe = new Number_Pipe(cur_pipe[0], cur_pipe[1], pipe_cmd->output_number_pipe());
            number_pipe->push_back(*new_pipe);
        }

        if(cmd.size() == 2){
            COMMAND *p = new COMMAND(cmd[0], cur_client, man, input);
            if(cur.size() != 0)
                close(cur[0].get_fd_write());

            if(p->get_in_flag()){
                user_pipe = true;
                in_id = p->get_in_id(), out_id = p->get_out_id();
            }

            pid_t pid = fork();
            while(pid < 0){
                waitpid(-1, &status, 0);
                pid = fork();
            }

            if(pid  == 0){
                if(p->get_in_flag()){
                    User_Pipe *cur_user_pipe = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                    dup2(cur_user_pipe->get_read_fd(), 0);
                    close(cur_user_pipe->get_read_fd());
                    close(cur_user_pipe->get_write_fd());
                }

                close(cur_pipe[0]);
                if(cur.size() != 0)
                    dup2(cur[0].get_fd_read(), 0);
                dup2(cur_pipe[1], 1);

                if(err_number_pipe_flag == true)
                    dup2(cur_pipe[1], 2);
                else 
                    err_to_socket(sockfd);
                
                error_user_pipe(p);
                p->exe_cmd(cur_client, man);
                if(cur.size()!=0)
                    close(cur[0].get_fd_read());
                close(cur_pipe[0]);
                close(cur_pipe[1]);
                exit(0);
            }else{
                //if(p->get_in_id()){
                    //User_Pipe *tmp = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                    //close(tmp->get_write_fd());
                    //man->del_user_pipe(p->get_in_id(), p->get_out_flag());
                //}        
                if(cur.size()!=0){
                    close(cur[0].get_fd_read());
                    close(cur[0].get_fd_write());
                    cur.pop_back();
                }
            }
            if(user_pipe){
                User_Pipe *tmp = man->find_user_pipe(in_id, out_id);
                close(tmp->get_write_fd());
                close(tmp->get_read_fd());
                man->del_user_pipe(in_id, out_id);
            }
        }else{
            if(cur.size() != 0)
                close(cur[0].get_fd_write());
            bool user_pipe; 
            int in_id, out_id;
            for(int i = 0; i < cmd.size()-1; i++){
                COMMAND *p = new COMMAND(cmd[i], cur_client, man, input);
                if(pipe(pi[i])){
                    cout << "pipe error" << endl;
                    return;
                }
                user_pipe = false;
                if(p->get_in_flag()){
                    user_pipe = true;
                    in_id = p->get_in_id(), out_id = p->get_out_id();
                    User_Pipe *cur_user_pipe = man->find_user_pipe(in_id, out_id);
                    close(cur_user_pipe->get_write_fd());
                }
                pid_t pid ;
                pid = fork();
                while(pid < 0){
                    waitpid(-1, &status, 0);
                    pid = fork();
                }
                if(pid == 0){
                    if(i == 0){
                        /*user pipe*/
                        if(p->get_in_flag()){
                            User_Pipe *cur_user_pipe = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                            dup2(cur_user_pipe->get_read_fd(), 0);
                            close(cur_user_pipe->get_read_fd());
                            close(cur_user_pipe->get_write_fd());
                        }
                        close(pi[i][0]);
                        dup2(pi[i][1], 1);
                        if(cur.size() != 0)
                            dup2(cur[0].get_fd_read(), 0);
                        if(err_number_pipe_flag)
                            dup2(pi[i][1], 2);
                        else
                            err_to_socket(sockfd);

                        error_user_pipe(p);
                        p->exe_cmd(cur_client, man);
                        if(cur.size() != 0)
                            close(cur[0].get_fd_read());
                        close(pi[i][1]);
                        exit(0); 
                    }else if(i != cmd.size()-2){
                        dup2(pi[i-1][0], 0);
                        dup2(pi[i][1], 1);
                        if(err_number_pipe_flag)
                            dup2(pi[i][1], 2);
                        else
                            err_to_socket(sockfd);

                        p->exe_cmd(cur_client, man);
                        close(pi[i-1][0]);
                        close(pi[i][1]);
                        exit(0);
                    }else{//cmd.size()-2
                        /*pipe*/

                        if(err_number_pipe_flag)
                            dup2(cur_pipe[1], 2);
                        else
                            err_to_socket(sockfd);
                        dup2(cur_pipe[1], 1);
                        dup2(pi[i-1][0], 0);
                        p->exe_cmd(cur_client, man);
                        close(pi[i-1][0]);
                        close(cur_pipe[1]);

                        exit(0);
                    }
                }else{

                    if(user_pipe){
                        User_Pipe *tmp = man->find_user_pipe(in_id, out_id);
                        close(tmp->get_write_fd());
                        close(tmp->get_read_fd());
                        man->del_user_pipe(in_id, out_id);
                    }
                    close(pi[i][1]);
                    close(pi[i-1][0]);
                    if(cur.size()!=0 && i == cmd.size()-2){
                        close(cur[0].get_fd_read());
                        close(cur[0].get_fd_write());
                        cur.pop_back();
                    }
                }
            }
        }
    }else{
        vector<pid_t>gc_pid;
        bool unknown_cmd_flag = true;
        bool user_pipe_out = false;
        bool user_pipe_in = false;
        int in_id, out_id;
        for(int i = 0; i < cmd.size(); i++){
            COMMAND *p = new COMMAND(cmd[i], cur_client, man, input);
            if(pipe(pi[i])){
                cout << "pipe error" << endl;
            }

            if(p->get_in_flag()){
                user_pipe_in = true;
                in_id = p->get_in_id(), out_id = p->get_out_id();
                close((man->find_user_pipe(p->get_in_id(), p->get_out_id()))->get_write_fd());
            }
            if(p->get_out_flag()){
                man->add_user_pipe(p->get_in_id(), p->get_out_id());
            }

            if(cur.size() != 0){
                close(cur[0].get_fd_write());
            }

            pid_t pid ;
            pid = fork();

            while(pid < 0){
                waitpid(-1, &status, 0);
                pid = fork();
            }
            gc_pid.push_back(pid);


            if(pid == 0){
                if(i == 0){
                    close(pi[i][0]);
                    if(p->get_in_flag()){
                        User_Pipe *t = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                        dup2(t->get_read_fd(), 0);
                        close(t->get_read_fd());
                        close(t->get_write_fd());
                    }else if(cur.size()!=0)
                        dup2(cur[0].get_fd_read(), 0);
                    dup2(pi[i][1], 1);
                    error_user_pipe(p);
                    err_to_socket(sockfd);
                    p->exe_cmd(cur_client, man);
                    close(pi[i][1]);
                    if(cur.size() != 0)
                        close(cur[0].get_fd_read());
                    exit(0);
                }else if(i != cmd.size()-1){
                    dup2(pi[i-1][0], 0);
                    dup2(pi[i][1], 1);
                    err_to_socket(sockfd);
                    p->exe_cmd(cur_client, man);
                    close(pi[i-1][0]);
                    close(pi[i][1]);
                    exit(0);
                }else{
                    if(p->get_out_flag()){
                        User_Pipe *t = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                        dup2(t->get_write_fd(), 1);
                        close(t->get_write_fd());
                        close(t->get_read_fd());
                    }else
                        output_to_socket(sockfd);

                    dup2(pi[i-1][0], 0);
                    error_user_pipe(p);
                    p->exe_cmd(cur_client, man);
                    close(pi[i-1][0]);
                    exit(0);
                }
            }else{
                if(p->get_in_flag()){
                    User_Pipe *tmp = man->find_user_pipe(p->get_in_id(), p->get_out_id());
                    man->del_user_pipe(p->get_in_id(), p->get_out_id());
                }
                if(p->get_out_flag()){
                    user_pipe_out = true;
                }
                if(cur.size()!=0){
                    close(cur[0].get_fd_read());
                    close(cur[0].get_fd_write());
                    cur.pop_back();
                }
                int status;
                close(pi[i][1]);
                close(pi[i-1][0]);
            }
        }
        if(user_pipe_in){
            User_Pipe *tmp = man->find_user_pipe(in_id,  out_id);
            man->del_user_pipe(in_id, out_id);
        }
        if(!user_pipe_out)
            for(int i = 0; i < gc_pid.size(); i++)
                waitpid(gc_pid[i], &status, 0);
    }
    waitpid(-1, &sta, WNOHANG);
}
