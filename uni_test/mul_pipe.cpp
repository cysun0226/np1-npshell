#include "../npshell.h"

std::vector<Command> create_cmds(std::string* cmds_str, 
                                 std::string* args,
                                 int* pipe_out,
                                 int cmd_num){
    
    std::vector<Command> cmds;
    
    for (size_t i = 0; i < cmd_num; i++)
    {
        Command cmd;
        cmd.cmd = cmds_str[i];
        std::vector<std::string> arg;
        if (args[i] != ""){
            arg.push_back(args[i]);
        }
        cmd.args = arg;
        cmd.pipe_out = pipe_out[i];
        cmd.idx = i;
        cmds.push_back(cmd);
    }
    
    return cmds;
}

std::pair <std::vector<Command>, std::string> parse_cmd(std::string usr_input) {
    std::vector<Command> cmds;
    std::string out_file = "";
    std::stringstream ss;
    ss.str(usr_input);
    // ss.exceptions(std::ios::failbit);
    std::vector<std::string> buf;
    std::string str;
    int idx = 0;
    while(ss >> str){
        if (str[0] == '|' || str[0] == '!' || str[0] == '>'){
            Command cmd;
            cmd.idx = idx; idx++;
            cmd.fd_type = str[0];
            // the pipe out idx
            if (str.size()>1){
                cmd.pipe_out = std::stoi(str.substr(1));
            }
            cmd.cmd = buf[0];
            for (size_t i = 0; i < buf.size(); i++){
                cmd.args.push_back(buf[i]);
            }
            if (str[0] == '>'){
                ss >> out_file;
            }   
            cmds.push_back(cmd);
            buf.clear();
        }
        else{
            buf.push_back(str);
        }
    }

    // last cmd (to stdout)
    if (buf.size() > 0){
        Command cmd;
        cmd.idx = idx; idx++;
        cmd.fd_type = '-';
        cmd.cmd = buf[0];
        for (size_t i = 0; i < buf.size(); i++){
            cmd.args.push_back(buf[i]);
        }
        cmds.push_back(cmd);
    }
    
    return std::pair<std::vector<Command>, std::string>(cmds, out_file);
}

std::vector<Pipe> pipe_table;
std::vector< std::pair <int*, int> > table_delete;
std::vector<int*> tmp_delete;
// pair.first , pair.second

void child_handler(int signo){
    int status;
    // -1 : wait for any child
    while (waitpid(-1, &status, WNOHANG) > 0);
}

pid_t exec_cmd(Command cmd, std::vector<int*> &fd_list){
  int status;
  pid_t pid;
  int pipe_id = 0;

  // check if exit
  if (cmd.cmd == "exit"){
    return EXIT;
  }
  
  pid = fork();

  switch (pid){
  case -1:{
    perror("fork error");
    std::cerr << "fork error" << std::endl;
    exit(1);
    break;
  }

  // child
  case 0:{
    // reditect I/O
    dup2(cmd.in_fd, STDIN_FILENO);  
    dup2(cmd.out_fd, STDOUT_FILENO);

    // close unuse pipes
    for (size_t i = 0; i < tmp_delete.size(); i++){
      close(tmp_delete[i][READ]);
      close(tmp_delete[i][WRITE]);
    }
    for (size_t i = 0; i < table_delete.size(); i++){
      close(table_delete[i].first[READ]);
      close(table_delete[i].first[WRITE]);
    }
    
    // convert cmd.args to exec format
    // const char *p_name = cmd.cmd.c_str();
    const char **args = new const char* [cmd.args.size()+1];
    // args[0] = p_name;
    for (int i = 0; i < cmd.args.size(); i++){
        args[i] = cmd.args[i].c_str();
    }   
    args[cmd.args.size()] = NULL;

    // execute    
    status = execvp(args[0], (char**)args); // process name, args

    exit(status);
    break;
  }
  
  // pid > 0, parent
  default:{
    // use signal handler to catch child that is not output to stdout
    if (cmd.out_fd != STDOUT_FILENO){
      signal(SIGCHLD, child_handler);
    }
    else{ // close pipe
      for (size_t i = 0; i < tmp_delete.size(); i++){      
        close(tmp_delete[i][READ]);
        close(tmp_delete[i][WRITE]);
      }
      for (size_t i = 0; i < table_delete.size(); i++){
        close(table_delete[i].first[READ]);
        close(table_delete[i].first[WRITE]);
      }
      // wait for the stdout process
      waitpid(pid, &status, 0);
    }
    break;
  }
  }

  return pid;
}

std::vector<int*> build_pipe(std::vector<Command> &cmds){
  std::vector<int*> fd_list;
  /* Check if previous pipe occurs */
  for (size_t i = 0; i < cmds.size(); i++){
    for (size_t p = 0; p < pipe_table.size(); p++){
      // exist pipe to stdin
      if (pipe_table[p].out_target ==  cmds[i].idx){
          cmds[i].in_fd = pipe_table[p].fd[READ];
          cmds[i].relate_pipe.push_back(pipe_table[p].fd);
          std::pair <int*, int> table_entry(pipe_table[p].fd, p);
          // can delete after used
          table_delete.push_back(table_entry);
      }
      // output target has existing pipe
      if (cmds[i].pipe_out == pipe_table[p].out_target){
          cmds[i].out_fd = pipe_table[p].fd[WRITE];
          cmds[i].relate_pipe.push_back(pipe_table[p].fd);
      }
      // next input has existing pipe
      if (cmds[i].pipe_out == PIPE_STDOUT && cmds[i].idx+1==pipe_table[p].out_target){
          cmds[i].out_fd = pipe_table[p].fd[WRITE];
          cmds[i].relate_pipe.push_back(pipe_table[p].fd);
      }
    }
  }

  /* Create required new pipes */
  for (size_t i = 0; i < cmds.size(); i++){
      // no previous existing pipe for next command
      if (cmds[i].pipe_out == PIPE_STDOUT &&
          cmds[i].out_fd == STDOUT_FILENO &&
          cmds[i+1].in_fd == STDIN_FILENO &&
          i != cmds.size()-1)  // the last command use stdout
      {
          // allocate new pipe
          int* fd = new int[2];
          if (pipe(fd) < 0){
              std::cerr << "[pipe error]" << cmds[i].cmd << std::endl;
              break;
          }
          
          cmds[i].out_fd = fd[WRITE];
          cmds[i].relate_pipe.push_back(fd);
          cmds[i+1].in_fd = fd[READ];
          cmds[i+1].relate_pipe.push_back(fd);
          // append from before
          for (size_t j = 0; j < cmds[i].relate_pipe.size(); j++){
            cmds[i+1].relate_pipe.push_back(cmds[i].relate_pipe[j]);
          }
          fd_list.push_back(fd);
          tmp_delete.push_back(fd);
      }      

      // no existing pipe for the output target
      if (cmds[i].pipe_out != PIPE_STDOUT && cmds[i].out_fd == STDOUT_FILENO){
          // allocate new pipe
          int* fd = new int[2];
          if (pipe(fd) < 0){
              std::cerr << "[pipe error]" << cmds[i].cmd << std::endl;
              break;
          }
          fd_list.push_back(fd);

          cmds[i].out_fd = fd[WRITE];
          cmds[i].relate_pipe.push_back(fd);
          // output target is in the current cmds
          if (cmds[i].pipe_out < cmds.size()-cmds[i].idx){
              cmds[i+cmds[i].pipe_out].in_fd = fd[READ];
              cmds[i+cmds[i].pipe_out].relate_pipe.push_back(fd);
              tmp_delete.push_back(fd);
              // redirct all cmd.out_fd that output to the same cmd
              for (size_t j = i+1; j<cmds.size(); j++){
                if (cmds[j].pipe_out != PIPE_STDOUT){
                  if (cmds[j].pipe_out + cmds[j].idx == cmds[i].pipe_out+cmds[i].idx){
                    cmds[j].out_fd = cmds[i].out_fd;
                    cmds[j].relate_pipe.push_back(fd);
                  }
                }
                else{
                  if (cmds[i].idx+cmds[i].pipe_out==cmds[j].idx+1){
                    cmds[j].out_fd = cmds[i].out_fd;
                    cmds[j].relate_pipe.push_back(fd);          
                  } 
                }
              }
          }
          // relay to following commands
          else{
              // update pipe_out for following commands
              cmds[i].pipe_out += (cmds[i].idx);
              Pipe p = {fd, cmds[i].pipe_out};
              pipe_table.push_back(p);
          }
      }
  }
  return fd_list;
}

int exec_cmds(std::vector<Command> cmds){
    int status;
    pid_t last_pid;

    // build pipes
    std::vector<int*> fd_list = build_pipe(cmds);

    // execute commands
    for (size_t i = 0; i < cmds.size(); i++){
        last_pid = exec_cmd(cmds[i], fd_list);
    }
    
    // delete tmp pipes for current cmds
    for (size_t i = 0; i < tmp_delete.size(); i++){
        delete [] tmp_delete[i];
    }
    tmp_delete.clear();

    // delete used pipe in pipe_table
    for (size_t i = 0; i < table_delete.size(); i++){
        // delete fd
        delete[] table_delete[i].first; 
        // delete entry in pipe_table
        pipe_table[table_delete[i].second] = pipe_table.back();
        pipe_table.pop_back();
    }
    table_delete.clear();

    // update out_target in pipe_table
    for (size_t i = 0; i < pipe_table.size(); i++){
        pipe_table[i].out_target -= cmds.size();
    }

    return status;
}

int main() {
    // set PATH
    char default_path[] = "PATH=bin:.";
    putenv(default_path);
    

    std::string usr_input_1 = "cat test.html |3 ls -l |2\
     removetag test.html |4 number | number";

    std::pair<std::vector<Command>, std::string> cmds_1 =\
     parse_cmd(usr_input_1);
    
    int status = exec_cmds(cmds_1.first);

    std::string usr_input_2 = "cat test.html | number |3 \
     number test.html";
    std::pair<std::vector<Command>, std::string> cmds_2 =\
     parse_cmd(usr_input_2);
    
    int status2 = exec_cmds(cmds_2.first);

    std::string usr_input_3 = "removetag test.html | number ";
    std::pair<std::vector<Command>, std::string> cmds_3 =\
     parse_cmd(usr_input_3);

    int status3 = exec_cmds(cmds_3.first);

    
    
    return 0;
}