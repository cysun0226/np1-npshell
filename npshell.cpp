#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>

#define PROMPT_SYMBOL "%" 
enum Status{SUCCESS, EXIT, ERROR};

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
    std::stringstream cmd_stream;
    cmd_stream.str(c);
    
    // frist word (cmd)
    cmd_stream >> cmd.cmd;

    // second word (input file)
    std::string i_file;
    cmd_stream >> i_file;
    
    if (!cmd_stream.fail()){
      cmd.in_file = i_file;
    }
    
    cmds.push_back(cmd);

    // if no any pipe symbol
    if (usr_ipt.find('|') == std::string::npos)
      break;

    ptr = strtok(ptr+strlen(ptr)+1, "|");
  }

  return cmds;
}

int exec_cmd(Command cmd){
  int status;
  pid_t pid;

  // check if exit
  if (cmd.cmd == "exit"){
    return EXIT;
  }
  

  // fork a child to exec the cmd
  pid = fork();

  switch (pid){
  case -1:
    std::cerr << "fork error" << std::endl;
    break;

  case 0: // child
    std::cout << "I'm child process, exec " << cmd.cmd << std::endl;
    exit(0);
    break;
  
  default: // pid > 0, parent
    std::cout << "I'm parent, wait for child" << std::endl;
    wait(&status);
    std::cout << "catch child, status=" << status << std::endl;
    status = SUCCESS;
    break;
  }

  return status;
}


int get_cmd(){
  int status = 0;

  // prompt
  std::cout << PROMPT_SYMBOL << " " << std::flush;
  // get user input
  std::vector<Command> cmds;
  std::string usr_input;
  std::getline(std::cin, usr_input);
  // parse
  cmds = parse_cmds(usr_input);

  // exec
  for (size_t i = 0; i < cmds.size(); i++){
    status = exec_cmd(cmds[i]);
  }
  // return exec status

  return status;
}




int main(){
  // set PATH
  char default_path[] = "PATH=./bin:.";
  putenv(default_path);

  int status;
  do{
    status = get_cmd();
  } while (status == SUCCESS);
  
  return 0;
}