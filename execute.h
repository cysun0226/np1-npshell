#ifndef EXECUTE_H
#define EXECUTE_H

#include "npshell.h"

// variables
extern std::vector<Pipe> pipe_table;
extern std::vector< std::pair <int*, int> > table_delete;
extern std::vector<int*> tmp_delete;


// functions

void child_handler(int signo);

std::vector<int*> build_pipe(std::vector<Command> &cmds);

pid_t exec_cmd(Command cmd, bool last);

int exec_cmds(std::vector<Command> cmds);

void clean_up();

void set_env(std::string usr_input);

std::string print_env(std::string);

#endif