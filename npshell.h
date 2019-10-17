#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define PROMPT_SYMBOL "%" 
#define PIPE_BUFFER_SIZE 15000
enum Status{SUCCESS, EXIT, ERROR};
enum {READ, WRITE};
enum {PIPE_STDOUT=-1};

typedef struct {
    std::string cmd;
    std::string in_file = "";
    std::string out_file = "";
    int pipe_in = -1;
    int pipe_out = -1;
    int in_fd = STDIN_FILENO;
    int out_fd = STDOUT_FILENO;
    int idx = -1;
    int pipe_idx;
    std::vector<std::string> args;
    // copy constructor
    // Command() : (c.n) { } // user-definced copy ctor
} Command;

std::ostream& operator<< (std::ostream &o, const Command &c){
      return o << "cmd = " << c.cmd << " | "
               << "in_file = " << c.in_file << " | "
               << "out_file = " << c.out_file << " | "
               << "pipe_in   = " << c.pipe_in   << " | "
               << "pipe_out  = " << c.pipe_out;
} 

typedef struct {
    int fd[2];
    int instr_cnt_down;
} Pipe;

int count_digit(int n){
  int cnt = 0;
  while (n != 0){
    n = n/10;
    cnt ++;
  }
  return cnt;
}

bool is_number(std::string s) {
  bool is_num = true;
  for (size_t i = 0; i < s.length(); i++){
    is_num = isdigit(s[i]);
  }
  return is_num;
}

// std::vector<Command> parse_cmds(std::string usr_ipt);