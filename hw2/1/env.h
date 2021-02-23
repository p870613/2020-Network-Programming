#ifndef ENV_H
#define ENV_H

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
#include <cstdlib>

class ENV{
    public:
        ENV();
        void set_env(char* name, char *path);
        char* print_env(char *name);
};

#endif
