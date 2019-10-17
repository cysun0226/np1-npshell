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

int exec_cmd(Command cmd){
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
    std::cout << "I'm child process, exec " << cmd.cmd << " " << cmd.args[0] << std::endl;
    if (cmd.in_fd != STDIN_FILENO)
        dup2(cmd.in_fd, STDIN_FILENO);
        
    if (cmd.out_fd != STDOUT_FILENO)
        dup2(cmd.out_fd, STDOUT_FILENO);

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
    close(cmd.in_fd);
    close(cmd.out_fd);
    status = SUCCESS;
    break;
  }

  return status;
}

void build_pipe(std::vector<Command> cmds, int std_pipe_num, int relay_pipe_num){

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
    std::string cmds_str[] = {"cat", "number", "number"};
    std::string args[] = {"test.html", "", ""};
    int pipe_out[] = {-1, -1, -1};

    std::vector<Command> cmds = create_cmds(cmds_str, args, pipe_out, 3);

    int std_pipe_num = 2, relay_pipe_num = 0;
    // create pipes before execute
    int** std_pipe = new int*[std_pipe_num];
    for (size_t i = 0; i < std_pipe_num; i++){
        std_pipe[i] = new int[2];
        pipe(std_pipe[i]);
    }

    int** relay_pipe = new int*[relay_pipe_num];
    for (size_t i = 0; i < relay_pipe_num; i++){
        relay_pipe[i] = new int[2];
        pipe(relay_pipe[i]);
    }

    int std_idx = 0, relay_idx = 0;
    for (size_t i = 0; i < cmds.size(); i++){
        // check if relay previous pipe
        for (size_t p = 0; p < pipe_table.size(); p++){
            if (pipe_table[p].instr_cnt_down == cmds[i].idx){
                cmds[i].in_fd = pipe_table[p].fd[READ];
            }
        }

        if (cmds[i].pipe_out != PIPE_STDOUT){
            cmds[i].pipe_idx = relay_idx;
            cmds[i].out_fd = relay_pipe[relay_idx][WRITE];
            // add to pipe_table
            Pipe p;
            p.instr_cnt_down = cmds[i].pipe_idx;
            p.fd[WRITE] = relay_pipe[relay_idx][WRITE];
            p.fd[READ] = relay_pipe[relay_idx][READ];
            pipe_table.push_back(p);
            relay_idx++;
        }
        else{
            cmds[i].pipe_idx = std_idx;
            if (i != cmds.size()-1){
                cmds[i].out_fd = std_pipe[std_idx][WRITE];
                cmds[i+1].in_fd = std_pipe[std_idx][READ];
            }
            std_idx++;
        }
    }

    // check if relay in current cmds
    for (size_t i = 0; i < cmds.size(); i++){
        for (size_t j = 0; j < cmds.size(); j++){
            if (i == j)
                break;
            if (cmds[i].idx == cmds[j].pipe_out && cmds[j].out_fd!=STDOUT_FILENO)
                cmds[i].in_fd = relay_pipe[cmds[j].pipe_idx][READ];
        }
    }

    // execute
     for (size_t i = 0; i < cmds.size(); i++){
        exec_cmd(cmds[i]);
    }

    int status;
    wait(&status);
    wait(&status);
    wait(&status);

    // delete array
    for(int i = 0; i < std_pipe_num; ++i) {
        delete [] std_pipe[i];
    }
    delete [] std_pipe;

    for(int i = 0; i < relay_pipe_num; ++i) {
        delete [] relay_pipe[i];
    }
    delete [] relay_pipe;
    
    return 0;
}