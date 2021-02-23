#include "cmd.h"

COMMAND::COMMAND(string input){
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


void COMMAND::print(){
    for(int i = 0; i < arg.size(); i++)
        cout  << arg[i] << " ";
    cout << endl;
    cout << output_file <<" " << endl;
}


bool COMMAND::exe_cmd(){
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
    return arg[0] == "setenv" || arg[0] == "printenv" || arg[0] == "exit";
}
void COMMAND::exe_build_in_system_cmd(ENV *e){
    if(arg[0] == "setenv"){
        e->set_env((char*)arg[1].c_str(), (char*)arg[2].c_str());
    }else if(arg[0] == "printenv"){
        cout << e->print_env((char*) arg[1].c_str()) << endl;

    }else if(arg[0] == "exit"){
        exit(0);
    }
}


int COMMAND::output_number_pipe(){
    stringstream ss(arg[0]);
    int re;
    ss >> re;
    return re; 
}
