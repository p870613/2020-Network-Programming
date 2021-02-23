#include "exe.h"

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

string urlDecode(string SRC) {
    string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])==37) {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return (ret);
}

EXE_CGI::EXE_CGI(string input){
    exe_parse(input);
}

void send_header(){
    cout << "HTTP/1.1 200 OK\r" << endl;
    //cout << "Content-Type: text/html\r\n\r\n";
    cout.flush();
}
bool EXE_CGI::exe(){
    send_header();
    string t = "./" + cgi_name;
    vector<char*>ar{(char*)t.c_str(), nullptr};
    if(execvp(ar[0], ar.data()) == -1)
        return false;
    return true;
}

void EXE_CGI::exe_parse(string input){
   string str = split(input, '\n')[0];
   str = urlDecode(split(str, ' ')[1]);

   if(str.find("/") >= 0)
        str.erase(str.begin() + str.find("/"));

   vector<string> ar = split(str, '?');
   cgi_name = ar[0];

   if(ar.size() != 1)
       query = ar[1];
   //cout << cgi_name << " " << query << endl;   
}
