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
class ENV{
    public:
        ENV();
        void set_env(char* name, char *path);
        char* print_env(char *name);
};

void ENV::set_env(char* name, char *path){
    setenv(name, path, 1);
}

char* ENV::print_env(char *name){
    return getenv(name);
}

ENV::ENV(){
    setenv("PATH", "bin:.", 1);
}

class COMMAND{
    private:
        vector<string>arg;
        string output_file;
    public:
        COMMAND(string input){
            output_file = "";
            vector<string> ar = split(input, ' ');
            for(int i = 0; i < ar.size(); i++){
                if(ar[i] == ">"){
                    if((i + 1) < ar.size())
                        output_file = ar[++i];
                }else{
                    arg.push_back(ar[i]);
                }
            }
        }

        void print(){
            for(int i = 0; i < arg.size(); i++)
                cout  << arg[i] << " ";
            cout << endl;
            cout << output_file <<" " << endl;
        }
        bool exe_cmd(){
            if(output_file != ""){
                int fd = open(output_file.c_str(), O_RDWR| O_APPEND | O_CREAT | O_TRUNC, 0600);
                dup2(fd, 1);
                close(fd);
            }

            vector<char*> ar(arg.size() + 1, nullptr);
            for(int i = 0; i < ar.size(); i++)
                transform(arg.begin(), arg.end(), ar.begin(), 
                        [](const string& s){return const_cast<char*>(s.c_str());});
                if(execvp(ar[0], ar.data()) == -1)
                {
                    cerr << "Unknown command: [" << ar[0] << "]." << endl;
                    return false;
                }
            return true;
        }

        bool build_in_system_command(){
            return arg[0] == "setenv" || arg[0] == "printenv" || arg[0] == "exit";
        }
        void exe_build_in_system_cmd(ENV *e){
          
            if(arg[0] == "setenv"){
                e->set_env((char*)arg[1].c_str(), (char*)arg[2].c_str());
            }else if(arg[0] == "printenv"){
                cout << e->print_env((char*) arg[1].c_str()) << endl;
        
            }else if(arg[0] == "exit"){
                exit(0);
            }
        }

        int output_number_pipe(){
            stringstream ss(arg[0]);
            int re;
            ss >> re;
            return re; 
        }
};

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
class Number_Pipe{
    private:
        int fd[2];
        int number;
    public:
        Number_Pipe(int read, int write, int count){
            fd[0] = read;
            fd[1] = write;
            number = count;
        }
        bool is_pop(){
            number --;
            if(number == 0)
                return true;
            return false;
        }

        int get_pipe_num(){
            return number;
        }
        int get_fd_read(){
            return fd[0];
        }

        int get_fd_write(){
            return fd[1];
        }
};
bool test(string& a){
    cout.flush();
    cout << "% ";
    cout.flush();
    if(getline(cin, a))
        return true;
    return false;
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
int main(){
    ENV *e = new ENV();
    string input;
    vector<Number_Pipe>number_pipe;
    vector<Number_Pipe>cur;
    bool flag_print = 0;
    int sta; 
    while(test(input)){
        if(input == ""){continue;} 
        waitpid(-1, &sta, WNOHANG);

        bool err_number_pipe_flag = false;
        err_number_pipe_flag = is_error_number_pipe(input);
        if(err_number_pipe_flag == true){
            replace_input(input);
        }
        vector<string> cmd = split(input.c_str(), '|');
        /*number pipe*/
        if(number_pipe.size() != 0){
            for(int i = number_pipe.size()-1; i >= 0; i--){
                if(number_pipe[i].is_pop()){
                    cur.push_back(number_pipe[i]);
                    number_pipe.erase(number_pipe.begin() + i);
                } 
            }
        }
        
        int pi[cmd.size()][2];
        int status = 0;
        if(cmd.size() == 1){
            COMMAND *p = new COMMAND(cmd[0]);
            pid_t pid;
            if(p->build_in_system_command()){
                p->exe_build_in_system_cmd(e); 
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
                    if(cur.size()!=0){
                        dup2(cur[0].get_fd_read(), 0);
                        p->exe_cmd();
                        close(cur[0].get_fd_read());
                        close(cur[0].get_fd_write());
                    }
                    else
                        p-> exe_cmd();
                    exit(0);
                }else{
                    waitpid(pid, &status, 0);
                    if(cur.size()!=0){
                        close(cur[0].get_fd_read());
                        close(cur[0].get_fd_write());
                        cur.pop_back();
                    }
                }
            }
        }else if((is_number_pine(cmd[cmd.size()-1]) && is_execution_file(input)) || err_number_pipe_flag == true){
            COMMAND *pipe_cmd = new COMMAND(cmd[cmd.size()-1]);
            int cur_pipe[2];
            int n = pipe_cmd->output_number_pipe();
            bool flag = 0;
            for(int i = 0; i < number_pipe.size(); i++){
                if(n == number_pipe[i].get_pipe_num()){
                    flag = 1;
                    cur_pipe[0] = number_pipe[i].get_fd_read();
                    cur_pipe[1] = number_pipe[i].get_fd_write();
                    break;
                }
            }
            if(flag == 0){
                if(pipe(cur_pipe))
                    return 0;
                Number_Pipe *new_pipe = new Number_Pipe(cur_pipe[0], cur_pipe[1], pipe_cmd->output_number_pipe());
                number_pipe.push_back(*new_pipe);
            }

            if(cmd.size() == 2){
                COMMAND *p = new COMMAND(cmd[0]);
                pid_t pid = fork();
                
                if(cur.size() != 0)
                    close(cur[0].get_fd_write());
                while(pid < 0){
                    waitpid(-1, &status, 0);
                    pid = fork();
                }

                if(pid  == 0){
                    close(cur_pipe[0]);
                    if(cur.size() != 0)
                        dup2(cur[0].get_fd_read(), 0);
                    dup2(cur_pipe[1], 1);
                    if(err_number_pipe_flag == true)
                        dup2(cur_pipe[1], 2);

                    p->exe_cmd();

                    if(cur.size()!=0)
                        close(cur[0].get_fd_read());
                    close(cur_pipe[0]);
                    close(cur_pipe[1]);
                    exit(0);

                }else{
                    if(cur.size()!=0){
                        close(cur[0].get_fd_read());
                        close(cur[0].get_fd_write());
                        cur.pop_back();
                    }
                }

            }else{
                if(cur.size() != 0)
                    close(cur[0].get_fd_write());
                
                for(int i = 0; i < cmd.size()-1; i++){
                    COMMAND *p = new COMMAND(cmd[i]);
                    if(pipe(pi[i]))
                        return 0;
                    pid_t pid ;
                    pid = fork();
                    while(pid < 0){
                        waitpid(-1, &status, 0);
                        pid = fork();
                    }

                    if(pid == 0){
                        if(i == 0){
                            close(pi[i][0]);
                            dup2(pi[i][1], 1);
                            if(cur.size() != 0)
                                dup2(cur[0].get_fd_read(), 0);
                            if(err_number_pipe_flag)
                                dup2(pi[i][1], 2);
                            p->exe_cmd();
                            if(cur.size() != 0)
                                close(cur[0].get_fd_read());
                            close(pi[i][1]);
                            exit(0); 
                        }else if(i != cmd.size()-2){
                            dup2(pi[i-1][0], 0);
                            dup2(pi[i][1], 1);
                            if(err_number_pipe_flag)
                                dup2(pi[i][1], 2);
                            p->exe_cmd();
                            close(pi[i-1][0]);
                            close(pi[i][1]);
                            exit(0);
                        }else{//cmd.size()-2
                            /*pipe*/
                            if(err_number_pipe_flag)
                                dup2(cur_pipe[1], 2);
                            dup2(cur_pipe[1], 1);
                            dup2(pi[i-1][0], 0);
                            p->exe_cmd();
                            close(pi[i-1][0]);
                            close(cur_pipe[1]);
                        }
                    }else{
                        close(pi[i][1]);
                        close(pi[i-1][0]);
                        if(cur.size()!=0){
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
            for(int i = 0; i < cmd.size(); i++){
                COMMAND *p = new COMMAND(cmd[i]);
                if(pipe(pi[i]))
                    return 0;
                pid_t pid ;
                pid = fork();
                if(cur.size() != 0){
                    close(cur[0].get_fd_write());
                }
                while(pid < 0){
                    waitpid(-1, &status, 0);
                    pid = fork();
                }
                gc_pid.push_back(pid);
                
                if(pid == 0){
                    if(i == 0){
                        close(pi[i][0]);
                        if(cur.size()!=0)
                            dup2(cur[0].get_fd_read(), 0);
                        dup2(pi[i][1], 1);
                        p->exe_cmd();
                        close(pi[i][1]);
                        if(cur.size() != 0)
                            close(cur[0].get_fd_read());
                        exit(0);
                    }else if(i != cmd.size()-1){
                        dup2(pi[i-1][0], 0);
                        dup2(pi[i][1], 1);
                        p->exe_cmd();
                        close(pi[i-1][0]);
                        close(pi[i][1]);
                        exit(0);
                    }else{
                        dup2(pi[i-1][0], 0);
                        p->exe_cmd();
                        close(pi[i-1][0]);
                        exit(0);
                    }
                }else{
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
            for(int i = 0; i < gc_pid.size(); i++)
                waitpid(gc_pid[i], &status, 0);
        }
        waitpid(-1, &sta, WNOHANG);
    }

}
