#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <algorithm>
#include <sstream>

#define PROMPT_SYMBOL "%" 


typedef struct {
    std::string cmd;
    std::string in_file = "";
    std::string out_file = "";
    int pipe_in = -1;
    int pipe_out = -1;

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

Command parse_cmd(std::string cmd_str){

}

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

std::vector<Command> parse_cmds(std::string usr_ipt){
  std::vector<Command> cmds;
  std::string out_file = "";
  char usr_input[15000];
  strcpy(usr_input, usr_ipt.c_str());
  
  // seperate command by pipe
  char* ptr = strtok(usr_input, "|");
  while (ptr != NULL){
    Command cmd;
    std::string c(ptr);

    // if first word is number (pipe indicator)
    std::stringstream ss;
    ss.str(c);
    ss.exceptions(std::ios::failbit);
    ss.clear();
    std::string pipe_num;
    ss >> pipe_num;
    if (is_number(pipe_num)){
      // take off the number in front of cmd
      c = c.substr(pipe_num.size());
      cmds.back().pipe_out = std::stoi(pipe_num);
    }

    // if there is '>'
    if (c.find('>') != std::string::npos){
      char o_str[256];
      strcpy(o_str, c.c_str());
      char* ptr_o = strtok(o_str, ">");
      // ptr = strtok(ptr, ">");
      // c = std::string(ptr);
      ptr_o = strtok(NULL, ">");
      std::string of(ptr_o);
      // remove the space in str
      while (of.find(' ') != std::string::npos){
        of.erase(std::remove(of.begin(), of.end(), ' '), of.end());  
      }
      cmd.out_file = of;
    }

    // split cmd by space
    char cmd_str[256];
    strcpy(cmd_str, c.c_str());
    char* ptr_s = strtok(cmd_str, " ");

    // frist word (cmd)
    cmd.cmd = std::string(ptr_s);
    while (cmd.cmd.find(' ') != std::string::npos){
        cmd.cmd.erase(std::remove(cmd.cmd.begin(), cmd.cmd.end(), ' '), cmd.cmd.end());  
    }
    // second word (input file)
    ptr_s = strtok(ptr_s+strlen(ptr_s)+1, " ");
    if (ptr_s != NULL && ptr_s[0]!='>'){
      cmd.in_file = std::string(ptr_s);
      while (cmd.in_file.find(' ') != std::string::npos){
          cmd.in_file.erase(std::remove(cmd.in_file.begin(), cmd.in_file.end(), ' '), cmd.in_file.end());  
      }
    }

    ptr = strtok(ptr+strlen(ptr)+1, "|");
    cmds.push_back(cmd);
  }

  return cmds;
}

void get_cmd(){
  // prompt
  std::cout << PROMPT_SYMBOL << " " << std::flush;
  // get user input
  std::vector<Command> cmds;
  std::string usr_input;
  std::getline(std::cin, usr_input);
  // parse
  cmds = parse_cmds(usr_input);

  for (size_t i = 0; i < cmds.size(); i++){
    std::cout << cmds[i] << std::endl;
  }
  // exec
  // return exec status
  
}

int exec_cmd(){
  int status;
  return status;
}


int main(){

  get_cmd();
  
  return 0;
}