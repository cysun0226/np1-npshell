# include "npshell.h" 

std::vector<Command> parse_cmds(std::string usr_ipt){
  std::vector<Command> cmds;
  std::string out_file = "";
  char usr_input[15000];
  strcpy(usr_input, usr_ipt.c_str());
  int idx = 0;
  
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

    cmd.idx = idx;
    idx++;

    cmds.push_back(cmd);

    // if no any pipe symbol
    if (usr_ipt.find('|') == std::string::npos)
      break;

    ptr = strtok(ptr+strlen(ptr)+1, "|");
  }

  cmds.back().idx = -1;
  return cmds;
}


int exec_cmd(Command cmd, int io_fd[]){
  int status;
  pid_t pid;
  int pipe_id = 0;

  // check if exit
  if (cmd.cmd == "exit"){
    return EXIT;
  }
  
  pid = fork();

  switch (pid){
  case -1:
    perror("fork error");
    std::cerr << "fork error" << std::endl;
    exit(1);
    break;

  case 0: // child
    std::cout << "I'm child process, exec " << cmd.cmd << std::endl;
    std::cout << "in_file = " << cmd.in_file << std::endl;
    if (cmd.idx == 0){ // first instr 
      std::cout << "[first cmd]" << std::endl;
      // dup2(io_fd[pipe_id*2+WRITE], STDOUT_FILENO); // dup stdout to pipe_id
      close(io_fd[READ]);
      dup2(io_fd[WRITE], STDOUT_FILENO); // dup stdout to pipe_id
      // close(io_fd[pipe_id*2+WRITE]);
    }
    else if(cmd.idx == -1){ // last instr
      std::cout << "[last cmd]" << std::endl;
      close(io_fd[WRITE]);
      dup2(io_fd[READ], STDIN_FILENO); // dup pipe of 1 to in
      // close(io_fd[WRITE]);
      // close(io_fd[READ]);
    }
    else{
      std::cout << "[middle cmd]" << std::endl;
      dup2(io_fd[READ], STDIN_FILENO); // dup stdin to in
      dup2(io_fd[WRITE], STDOUT_FILENO); // dup output to stdout
      // close(io_fd[WRITE]);
      close(io_fd[READ]);
    }

    if (cmd.in_file == ""){
      status = execlp(cmd.cmd.c_str(), cmd.cmd.c_str(), NULL);
    }
    else{
      status = execlp(cmd.cmd.c_str(), cmd.cmd.c_str(), cmd.in_file.c_str(), NULL);
    }

    std::cerr << "unknown command" << std::endl;
    exit(1);
    break;
  
  default: // pid > 0, parent
    std::cout << "I'm parent, wait for child" << std::endl;
    if(cmd.idx != 0){
      close(io_fd[WRITE]);
    }
    waitpid(pid, &status, 0);
    // wait(&status);

    std::cout << "catch child, status=" << status << std::endl;
    status = SUCCESS;
    break;
  }

  return status;
}

int exec_cmds(std::vector<Command> cmds){
  // create pipes for |

  // create pipes for |n
  // after using, push into the global vector

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
  // fork a child to exec the cmd
  // pipe, fd[0] is for reading, fd[1] is for writing
  int io_fd[4];
  int err_fd[4];
  int pipe_id = 0;
  pipe(io_fd);
  for (size_t i = 0; i < cmds.size(); i++){
    status = exec_cmd(cmds[i], io_fd);
  }
  // return exec status

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