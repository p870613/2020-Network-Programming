#include "env.h"
void ENV::set_env(char* name, char *path){
    setenv(name, path, 1);
}

char* ENV::print_env(char *name){
    return getenv(name);
}

ENV::ENV(std::string path){
    setenv("PATH", path.c_str(), 1);
}

