#include "env.h"

static vector<string>split(const string& s, const char d){
    vector<string> re;
    string tmp;
    stringstream str(s);
    while(getline(str, tmp, d)){
        if(tmp.length() > 0)
            re.push_back(tmp);
    }
    return re;
}

ENV::ENV(string input, string cli_addr, int cli_port){
    env_parse(input);
    env_var[remote_addr] = cli_addr;
    env_var[remote_port] = to_string(cli_port);
    this->set_env();
}

void ENV::env_parse(string str){
   vector<string> ar = split(str, '\n');

   //ar[0]
   vector <string>tmp = split(ar[0], ' ');
   env_var[method] = tmp[0];

   if(tmp[1].find("/") >= 0)
        tmp[1].erase(tmp[1].begin() + tmp[1].find("/"));

   env_var[query] = tmp[1];
   env_var[protocol] = tmp[2];

   env_var[host] = split(ar[1], ' ')[1];
   env_var[server_addr] = split(env_var[host], ':')[0];
   env_var[server_port] = split(env_var[host], ':')[1];

   string total_url = env_var[host] + "/" +env_var[query];
   env_var[url] = total_url;
}

void ENV::set_env(){
    for(int i = method; i<= remote_port; i++){
        setenv(var_name[i].c_str(), env_var[i].c_str(), 1);
    }
}

void ENV::print_env(){
    for(int i = method; i<= remote_port; i++){
        cout << var_name[i] << " " << getenv(var_name[i].c_str()) << endl;
    }
}
