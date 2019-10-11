#include <iostream>
#include <string>
#include <vector>

#define PROMPT_SYMBOL "%" 


typedef struct {
    std::string cmd;
    std::string in_file;
    int pipe_in;
    int pipe_out;
} Command;

void parse_cmds(std::vector<Command> cmds){
  // find if there is '>' (and > must appear in the last cmd)


}

void get_cmd(){
  // prompt
  std::cout << PROMPT_SYMBOL << " " << std::flush;
  // get user input
  std::vector<Command> commands;
  std::string usr_input;
  std::getline(std::cin, usr_input);
  // parse
  std::cout << usr_input << std::endl;

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