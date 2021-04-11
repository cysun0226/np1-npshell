/*
 *   A sample of "ls -l | grep cpp "
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>

enum{READ, WRITE};

int main() {
    char default_path[] = "PATH=bin:.";
    putenv(default_path);

    int in_fd[2];
    int out_fd[2];
    int fd[2];
    pid_t pid;
    char buf[1024];
    pipe(fd);

    pid = fork();

    // child1
    if (pid == 0){
        
        // dup2(in_fd[READ], STDIN_FILENO); // dup output to stdout
        close(fd[READ]);
        dup2(fd[WRITE], STDOUT_FILENO);
        dup2(fd[WRITE], STDERR_FILENO);
        
        // close(fd[WRITE]);
        
        execlp("removetag0", "removetag0", "test.html", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

    
    int status;
    // WNOHANG: return immediately if no child has exited. 
    //          to handle child first
    waitpid(pid, &status, 0); 
    std::cout << "parent catch child 1" << std::endl;
    

    // child2
    pid_t pid2;
    pid2 = fork();
    if (pid2 == 0){
        close(fd[WRITE]);
        dup2(fd[READ], STDIN_FILENO);
        // dup2(fd[WRITE], STDOUT_FILENO);
        // close(fd[READ]);
        
        execlp("number", "number", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

    close(fd[WRITE]);
    waitpid(pid2, &status, 0);
    std::cout << "parent catch child 2" << std::endl;    
    
    return 0;
}

