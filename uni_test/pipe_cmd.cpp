#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>
#include "npshell.h"

enum{READ, WRITE};



int main() {
    /* sample input */
    // removetag test.html |2 removetag test.html | number

    int fd[4];
    pid_t pid;
    char buf[1024];
    pipe(fd);

    pid = fork();

    // child1
    // pipe_id = 0;
    if (pid == 0){
        int pipe_id = 0;
        dup2(fd[pipe_id*2+WRITE], STDOUT_FILENO); // dup output to stdout
        close(fd[pipe_id*2+READ]);
        close(fd[pipe_id*2+WRITE]);
        execlp("bin/cat", "bin/cat", "test.html", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }
    
    int status;
    waitpid(pid, &status, WNOHANG); 
    // WNOHANG: return immediately if no child has exited. 
    //          to handle child first

    std::cout << "parent catch child 1" << std::endl;

    // child2
    pid_t pid2;
    pid2 = fork();
    if (pid2 == 0){
        dup2(fd[READ], STDIN_FILENO); // dup stdin to in
        dup2(fd[WRITE], STDOUT_FILENO); // dup output to stdout
        close(fd[WRITE]);
        close(fd[READ]);
        execlp("bin/removetag", "bin/removetag", "test.html", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

        
    waitpid(pid2, &status, WNOHANG);
    std::cout << "parent catch child 2" << std::endl;    

    // child3
    pid_t pid3;
    pid3 = fork();
    if (pid3 == 0){
        dup2(fd[READ], STDIN_FILENO);
        close(fd[WRITE]);
        close(fd[READ]);
        execlp("bin/number", "bin/number", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

        
    waitpid(pid3, &status, WNOHANG);
    std::cout << "parent catch child 3" << std::endl;    
    
    return 0;
}


