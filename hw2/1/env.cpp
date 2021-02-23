#include "env.h"
void ENV::set_env(char* name, char *path){
    setenv(name, path, 1);
}

char* ENV::print_env(char *name){
    return getenv(name);
}

ENV::ENV(){
    setenv("PATH", "bin:.", 1);
}

