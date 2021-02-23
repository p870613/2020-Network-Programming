#include "cmd.h"

bool check_string_in(string str){
    regex reg("[<][0-9]?[0-9]?");
    if(regex_match(str, reg))
        return true;
    return false;
}

bool check_string_out(string str){
    regex reg("[>][0-9]?[0-9]?");
    if(regex_match(str, reg))
        return true;
    return false;
}

bool check_both(vector<string>ar){
    int count = 0;
    for(int i = 0; i < ar.size(); i++){
        if(check_string_in(ar[i])){
            count ++;
        }
        if(check_string_out(ar[i])){
            count ++;
        }
    }
    if(count == 2)
        return true;
    return false;
}

COMMAND::COMMAND(string input, Client *cli, Client_Mananger *man, string all_input){
    output_file = "";
    user_pipe_in_id = -1;
    user_pipe_out_id = -1;
    in_flag = false;
    out_flag = false;
    error_in_flag = false;
    error_out_flag = false;
    vector<string> ar = split(input, ' ');
    bool flag = check_both(ar);
    string a = "", b = "";
    for(int i = 0; i < ar.size(); i++){
        if(ar[i] == ">"){
            if((i + 1) < ar.size())
                output_file = ar[++i];
        }else if(check_string_in(ar[i])){
            string tmp = ar[i].replace(ar[i].find("<"), 1, "");
            int tmp_in = atoi(tmp.c_str()), tmp_out = cli->get_id();
            if(!man->check_user_pipe_id(tmp_in)){
                string str = "*** Error: user #" + to_string(tmp_in) +" does not exist yet. ***\n";
                write(cli->get_fd(), str.c_str(), strlen(str.c_str()));
                error_in_flag = true;
            }else if(man->find_user_pipe(tmp_in, tmp_out) == NULL){
                string str = "*** Error: the pipe #" + to_string(tmp_in)+ "->#"+ to_string(tmp_out)+" does not exist yet. ***\n";
                write(cli->get_fd(), str.c_str(), strlen(str.c_str()));
                error_in_flag = true;
            }else{
                user_pipe_in_id = atoi(tmp.c_str());
                user_pipe_out_id = cli->get_id();
                in_flag = true;
                string str = "*** "+ man->from_id_get_name(user_pipe_out_id) + " (#"+ to_string(user_pipe_out_id)+") just received from "+ 
                    man->from_id_get_name(user_pipe_in_id) + " (#" + to_string(user_pipe_in_id) + ") by '" + all_input + "' ***\n";
                if(!flag){
                    man->broadcast(-1, str);
                }else{
                    a = str;
                }
            }
        }else if(check_string_out(ar[i])){
            string tmp = ar[i].replace(ar[i].find(">"), 1, "");
            int tmp_out = atoi(tmp.c_str()), tmp_in = cli->get_id();
            if(man->find_user_pipe(tmp_in, tmp_out) != NULL){
                string str = "*** Error: the pipe #" + to_string(tmp_in) + "->#" + to_string(tmp_out) + "  already exists. ***\n";
                write(cli->get_fd(), str.c_str(), strlen(str.c_str()));   
                error_out_flag = true;
            }else if(!man->check_user_pipe_id(tmp_out)){
                string str = "*** Error: user #" + to_string(tmp_out) +" does not exist yet. ***\n";
                write(cli->get_fd(), str.c_str(), strlen(str.c_str()));
                error_out_flag = true;
            }else{
                user_pipe_out_id = atoi(tmp.c_str()); 
                user_pipe_in_id = cli->get_id();
                out_flag = true;
                string str = "*** "+ man->from_id_get_name(user_pipe_in_id)+" (#" + to_string(user_pipe_in_id) + ") just piped '"+ all_input + 
                    "' to "+ man->from_id_get_name(user_pipe_out_id)+ " (#"+  to_string(user_pipe_out_id)+ ") ***\n";
                if(!flag){
                    man->broadcast(-1, str);
                }else{
                    b = str;
                }
            }
        }else{
            arg.push_back(ar[i]);
        }
    }
    
    if(flag){
        string str = a+ b;
        man->broadcast(-1, str);
    }
}

void COMMAND::print(){
    for(int i = 0; i < arg.size(); i++)
        cout  << arg[i] << " ";
    cout << endl;
    cout << output_file <<" " << endl;
}


bool COMMAND::exe_cmd(Client *cli, Client_Mananger *man){
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


bool COMMAND::build_in_system_command(){
    return arg[0] == "setenv" || arg[0] == "printenv" || arg[0] == "exit" || arg[0] == "who" || arg[0] == "name" || arg[0] == "tell" || arg[0] == "yell";
}

void COMMAND::exe_build_in_system_cmd(ENV *e, Client *cli, Client_Mananger* man, string input){
    if(arg[0] == "setenv"){
        cli->change_env(arg[2]);
        e->set_env((char*)arg[1].c_str(), (char*)arg[2].c_str());
    }else if(arg[0] == "printenv"){
        char *str = e->print_env((char*) arg[1].c_str());
        write(cli->get_fd(), str, strlen(str));
        write(cli->get_fd(), "\n", 1);
    }else if(arg[0] == "exit"){
        cli->client_exit(man->get_afds(), man->get_rfds());
        string str = "*** User '" + cli->get_name() + "' left. ***\n"; 
        man->broadcast(-1, str);
        man->del(cli->get_fd());
    }else if(arg[0] == "who"){
        man->cmd_who(cli->get_fd());
    }else if(arg[0] == "name"){
        if(man->check_name(arg[1]))
            man->cmd_change_name(cli->get_fd(), arg[1]);
        else{
            string str = "*** User '" + arg[1] + "' already exists. ***\n";
            const char *tmp = str.c_str();
            write(cli->get_fd(), tmp, strlen(tmp));
        }
    }else if(arg[0] == "tell"){
        int to_n = atoi(arg[1].c_str());
        if(man->check_id(to_n)){
            string content = input;
            content = content.replace(content.find(arg[0]), arg[0].size(), "");
            content = content.replace(content.find(arg[1]), arg[1].size(), "");
            while(true){
                int index = content.find(" "); 
                if(index != 0)
                    break;
                content = content.replace(index, 1, "");
            }
            string str = "*** " + cli->get_name() + " told you ***: " + content + "\n";
            const char* tmp = str.c_str();
            write(man->get_fd_from_id(to_n), tmp, strlen(tmp));

        }else{
            string str = "*** Error: user# " + arg[1] + " does not exist yet. ***\n";
            const char* tmp = str.c_str();
            write(cli->get_fd(), tmp, strlen(tmp));
        }
         
    }else if(arg[0] == "yell"){
        string content = input;
        content = content.replace(content.find(arg[0]), arg[0].size(), "");
        while(true){
            int index = content.find(" "); 
            if(index != 0)
                break;
            content = content.replace(index, 1, "");
        }
        content = "*** " + cli->get_name() + " yelled ***: " + content + "\n";
        man->broadcast(-1, content);
    }
}

int COMMAND::output_number_pipe(){
    stringstream ss(arg[0]);
    int re;
    ss >> re;
    return re; 
}

int COMMAND::get_in_id(){
    return user_pipe_in_id;
}

int COMMAND::get_out_id(){
    return user_pipe_out_id;
}

bool COMMAND::get_in_flag(){
    return in_flag;
}

bool COMMAND::get_out_flag(){
    return out_flag;
}

