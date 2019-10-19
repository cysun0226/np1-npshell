#include "npshell.h" 
#include "parse.h"
#include "execute.h" 


int get_cmd(){
  int status = SUCCESS;

  // prompt
  std::cout << PROMPT_SYMBOL << " " << std::flush;
  // get user input
  std::vector<Command> cmds;
  std::string usr_input;
  std::getline(std::cin, usr_input);

  // if exit
  if (usr_input == "exit"){
    return EXIT;
  }

  // parse
  std::pair<std::vector<Command>, std::string> parsed_cmd\
  = parse_cmd(usr_input);

  // exec
  exec_cmds(parsed_cmd.first);

  return status;
}



int main(int argc, char *argv[], char *envp[]){
  // set PATH
  char default_path[] = "PATH=bin:.";
  putenv(default_path);

  int status;
  do{
    status = get_cmd();
  } while (status == SUCCESS);
  
  return 0;
}