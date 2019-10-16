#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>
#include "../npshell.h"
#include <vector>




int main() {
    /* sample input */
    // cat test.html |2 removetag test.html | number
    std::vector<Command> cmds;

    int fd[2][2];

    pid_t pid;
    char buf[1024];
    for (size_t i = 0; i < 2; i++){
        pipe(fd[i]);
    }
    
    pid = fork();

    // child1
    if (pid == 0){
        
        dup2(fd[0][WRITE], STDOUT_FILENO); // dup stdout to pipe_id

        for (size_t i = 0; i < 2; i++){
            close(fd[i][READ]);
            close(fd[i][WRITE]);
        }  
        
        execlp("bin/cat", "bin/cat", "test.html", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

    int status;

    // child2
    pid_t pid2;
    pid2 = fork();
    if (pid2 == 0){
        dup2(fd[0][READ], STDIN_FILENO); // dup output to stdout
        dup2(fd[1][WRITE], STDOUT_FILENO); // dup stdin to in

        for (size_t i = 0; i < 2; i++){
            close(fd[i][READ]);
            close(fd[i][WRITE]);
        }  
        
        execlp("bin/number", "bin/number", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

    // child3
    pid_t pid3;
    pid3 = fork();
    if (pid3 == 0){
        // waitpid(pid2, &status, 0); 
        
        dup2(fd[1][READ], STDIN_FILENO); // dup pipe of 1 to in

        for (size_t i = 0; i < 2; i++){
            close(fd[i][READ]);
            close(fd[i][WRITE]);
        }  
        
        // std::cout << "I'm child 3" << std::endl;
        execlp("bin/ls", "bin/ls", "-l", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

    // child4
    pid_t pid4;
    pid4 = fork();
    if (pid4 == 0){       
        dup2(fd[1][READ], STDIN_FILENO); // dup pipe of 1 to in

        for (size_t i = 0; i < 2; i++){
            close(fd[i][READ]);
            close(fd[i][WRITE]);
        }  
        
        // std::cout << "I'm child 3" << std::endl;
        execlp("bin/number", "bin/number", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

    for (size_t i = 0; i < 2; i++){
            close(fd[i][READ]);
            close(fd[i][WRITE]);
    }  

    for (size_t i = 0; i < 4; i++){
        wait(&status);    
        // std::cout << "parent catch child" << std::endl;
    }
    
    return 0;
}


