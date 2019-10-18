#include "../npshell.h"

std::vector<Command> parse_cmd(std::string usr_input) {
    std::vector<Command> cmds;
    std::stringstream ss;
    ss.str(usr_input);
    ss.exceptions(std::ios::failbit);
    std::vector<std::string> buf;
    std::string str;
    while(ss >> str){
        if (str == "|" || str == "!"){
            // the pipe out idx of previous cmd
            int cmd_start = 0;
            if (is_number(buf[0])){
                cmds.back().pipe_out = std::stoi(str);
                cmd_start = 1;
            }
            
            Command cmd;
            cmd.cmd = buf[cmd_start];
            for (size_t i = cmd_start; i < buf.size(); i++){
                cmd.args.push_back(buf[i]);
                cmd.fd_type = str[0];
            }
        }
        buf.push_back(str);
    }
    return cmds;
}



int main(){


    return 0;
}