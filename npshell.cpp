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

  // if EOF or exit
  if (std::cin.eof() || usr_input == "exit"){
    clean_up();
    return EXIT;
  }

  // env command
  if (usr_input.substr(0, 6) == "setenv"){
    set_env(usr_input.substr(7));
    update_up_target();
    return SUCCESS;
  }
  if (usr_input.substr(0, 8) == "printenv"){
    std::cout << print_env(usr_input.substr(9)) << std::endl;
    update_up_target();
    return SUCCESS;
  }

  // parse
  std::pair<std::vector<Command>, std::string> parsed_cmd\
  = parse_cmd(usr_input);

  // exec
  exec_cmds(parsed_cmd);

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