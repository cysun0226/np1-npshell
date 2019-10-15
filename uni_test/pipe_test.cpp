/*
 *   A sample of "ls -l | grep cpp"
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>

enum{READ, WRITE};

int main() {
    int fd[2];
    pid_t pid;
    char buf[1024];
    pipe(fd);

    pid = fork();

    // child1
    if (pid == 0){
        dup2(fd[WRITE], STDOUT_FILENO); // dup output to stdout
        close(fd[READ]);
        close(fd[WRITE]);
        execlp("ls", "ls", "-l", (char*) NULL);
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
        dup2(fd[READ], STDIN_FILENO);
        close(fd[WRITE]);
        close(fd[READ]);
        execlp("grep", "grep", "cpp", (char*) NULL);
        std::cerr << "execute failed" << std::endl;
        exit(1);
    }

        
    waitpid(pid2, &status, WNOHANG);
    std::cout << "parent catch child 2" << std::endl;    
    
    return 0;
}

