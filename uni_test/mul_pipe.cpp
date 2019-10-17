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
        arg.push_back(args[i]);
        cmd.args = arg;
        cmd.pipe_out = pipe_out[i];
        cmd.idx = i;
        cmds.push_back(cmd);
    }
    
    return cmds;
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
  case -1:
    perror("fork error");
    std::cerr << "fork error" << std::endl;
    exit(1);
    break;

  case 0: // child
    // std::cout << "I'm child process, exec " << cmd.cmd << " " << cmd.args[0] << std::endl;
    // std::cerr << "I'm child process, exec " << cmd.cmd << " " << cmd.args[0] << std::endl;

    // if (cmd.in_fd != STDIN_FILENO){
      // std::cerr << "I'm child process " << cmd.cmd << " dup in " << std::endl;
      dup2(cmd.in_fd, STDIN_FILENO);  
      // close(cmd.in_fd);
    
        
    // if (cmd.out_fd != STDOUT_FILENO){
      // std::cerr << "I'm child process " << cmd.cmd << " dup out " << std::endl;
      dup2(cmd.out_fd, STDOUT_FILENO);
      // close(cmd.out_fd);
    

    for (size_t i = 0; i < tmp_delete.size(); i++){
      close(tmp_delete[i][READ]);
      close(tmp_delete[i][WRITE]);
    }

    for (size_t i = 0; i < table_delete.size(); i++){
      close(table_delete[i].first[READ]);
      close(table_delete[i].first[WRITE]);
    }
    
    

    // for (size_t i = 0; i < cmd.relate_pipe.size(); i++){
    //   close(cmd.relate_pipe[i][READ]);
    //   close(cmd.relate_pipe[i][WRITE]);
    // }
    
    

    // for (size_t i = 0; i < cmds.size(); i++){
    //   close(cmds[i].in_fd);
    //   // if (cmds[i].out_fd != STDOUT_FILENO){
    //     close(cmds[i].out_fd);
    //   // }
    // }
    
        

    // TODO modify arg to []
    if (cmd.args[0] == ""){
      status = execlp(cmd.cmd.c_str(), cmd.cmd.c_str(), NULL);
    }
    else{
      status = execlp(cmd.cmd.c_str(), cmd.cmd.c_str(), cmd.args[0].c_str(), NULL);
    }

    std::cerr << "unknown command" << std::endl;
    exit(status);
    break;
  
  default: // pid > 0, parent
    // close(cmd.in_fd);
    // if (cmd.out_fd != STDOUT_FILENO)
    //     close(cmd.out_fd);

    // wait for the last command
    int status;
    if (cmd.out_fd != STDOUT_FILENO){
      signal(SIGCHLD, child_handler);
    }

    // if (cmd.out_fd == STDOUT_FILENO){
    //   for (size_t i = 0; i < cmds.size(); i++){
    //     close(cmds[i].in_fd);
    //     if (cmds[i].out_fd != STDOUT_FILENO){
    //       close(cmds[i].out_fd);
    //     }
    //   }
    //   waitpid(pid, &status, 0);
    // }
        
    // use signal handler to catch child
        

    status = SUCCESS;
    break;
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

    for (size_t i = 0; i < tmp_delete.size(); i++){
      close(tmp_delete[i][READ]);
      close(tmp_delete[i][WRITE]);
    }

    for (size_t i = 0; i < table_delete.size(); i++){
      close(table_delete[i].first[READ]);
      close(table_delete[i].first[WRITE]);
    }

    
    

    // for (size_t i = 0; i < fd_list.size(); i++){
    //   close(fd_list[i][READ]);
    //   close(fd_list[i][WRITE]);
    // }

    waitpid(last_pid, &status, 0);
    
    
    
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
        pipe_table.erase(pipe_table.begin()+table_delete[i].second);
    }

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
    /* sample */
    // cat test.html |5 ls -l |6 cat test.html | number | number
    // std::string cmds_str[] = {"cat", "ls -l", "cat", "number", "number"};
    // std::string args[] = {"cat", "ls -l", "cat", "number", "number"};
    // int pipe_out[] = {5, 6, -1, -1, -1};
    std::string cmds_str[] = {"cat", "ls", "removetag", "number", "number"};
    std::string args[] = {"test.html", "-l", "test.html", "", ""};
    int pipe_out[] = { 3, 2, 4, -1, -1 };

    std::vector<Command> cmds = create_cmds(cmds_str, args, pipe_out, 5);
    int status = exec_cmds(cmds);

    std::string cmds_str_2[] = {"cat", "number", "number"};
    std::string args_2[] = {"test.html", "", ""};
    int pipe_out2[] = { -1, -1, -1 };
    
    std::vector<Command> cmds2 = create_cmds(cmds_str_2, args_2, pipe_out2, 3);
    int status2 = exec_cmds(cmds2);

    
    
    return 0;
}