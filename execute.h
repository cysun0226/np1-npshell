#ifndef EXECUTE_H
#define EXECUTE_H

#include "npshell.h"
#include <sys/stat.h>
#include <fcntl.h>

// variables
extern std::vector<Pipe> pipe_table;
extern std::vector< std::pair <int*, int> > table_delete;
extern std::vector<int*> tmp_delete;


// functions

void child_handler(int signo);

int build_pipe(std::vector<Command> &cmds);

pid_t exec_cmd(Command cmd, bool last);

int exec_cmds(std::pair<std::vector<Command>, std::string> parsed_cmd);

void clean_up();

void set_env(std::string usr_input);

std::string print_env(std::string);

#endif