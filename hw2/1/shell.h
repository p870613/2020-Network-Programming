#ifndef SHELL_H
#define SHELL_H

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

#include "env.h"
#include "cmd.h"
#include "number_pipe.h"
using namespace std;

vector<string>split(const string&, const char);

bool is_file(const char*);

bool is_number_pine();

bool is_execution_file();

bool test();

bool is_error_number_pipe(string &);

string replace_input(string &);

int shell();

void input_handler(string &a);

#endif
