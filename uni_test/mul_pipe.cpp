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
        cmds.push_back(cmd);
    }
    
    return cmds;
}

std::vector<Pipe> pipe_table;

int main() {
    /* sample */
    // cat test.html |5 ls -l |6 cat test.html | number | number
    std::string cmds_str[] = {"cat", "ls -l", "cat", "number", "number"};
    std::string args[] = {"cat", "ls -l", "cat", "number", "number"};
    int pipe_out[] = {5, 6, -1, -1, -1};

    std::vector<Command> cmds = create_cmds(cmds_str, args, pipe_out, 5);

    int std_pipe_num = 3, relay_pipe_num = 2;
    // create pipes before execute
    int** std_pipe = new int*[std_pipe_num];
    for (size_t i = 0; i < std_pipe_num; i++){
        std_pipe[i] = new int[2];
    }

    int** relay_pipe = new int*[relay_pipe_num];
    for (size_t i = 0; i < relay_pipe_num; i++){
        std_pipe[i] = new int[2];
    }

    int std_idx = 0, relay_idx = 0;
    for (size_t i = 0; i < cmds.size(); i++){
        // check if relay previous pipe
        for (size_t p = 0; p < pipe_table.size(); p++){
            if (pipe_table[p].instr_cnt_down == cmds[i].idx){
                cmds[i].in_fd = pipe_table[p].fd;
            }
        }

        if (cmds[i].pipe_out != PIPE_STDOUT){
            cmds[i].pipe_idx = relay_idx;
            // add to pipe_table
            Pipe p;
            p.instr_cnt_down = cmds[i].pipe_idx;
            p.fd = relay_pipe[relay_idx];
            pipe_table.push_back(p);
            relay_idx++;
        }
        else{
            cmds[i].pipe_out = std_idx;
            std_idx++;
        }
    }

    // check if relay in current cmds
    for (size_t i = 0; i < cmds.size(); i++){
        for (size_t j = 0; j < cmds.size(); j++){
            if (i == j)
                break;
            if (cmds[i].idx == cmds[j].pipe_out)
                cmds[i].in_fd = relay_pipe[cmds[j].pipe_idx];
        }
    }




    // delete array
    // for(int i = 0; i < sizeY; ++i) {
    //     delete [] ary[i];
    // }
    // delete [] ary;
    



    

    

}