#include<iostream>
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

int main() {

    std::vector<std::string> args;
    args.push_back("-l");

    std::vector<char*> arg_char;
    // char *tmp = "ls";
    // arg_char.push_back(tmp);
    for (size_t i = 0; i < args.size(); i++){
        char *cstr = new char[args[i].size()+1];
        strcpy(cstr, args[i].c_str());
        arg_char.push_back(cstr);
    }

    char ** arg_ptr = arg_char.data();

    execvp("ls", arg_ptr);

    return 0;

}


