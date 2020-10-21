#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "headers/function.h"
#include "headers/job.h"
void Perror(char *str) {
    perror(str);
    exit(i);
}
void Ferror(char *str, int n) {
    fprintf(stderr, "%s\n", str);
    exit(n);
}
void *Malloc(size_t size) {
    char *p = malloc(size);
    if (p == NULL) Ferror("malloc", 1);
    return p;
}
void *Realloc(char *str, size_t size) {
    char *p = realloc(str, size);
    if (p == NULL) Ferror("realloc", 1);
    return p;
}
void FreeArgList(char **argList) {
    if (argList != NULL) {
        while (*argList) {
            free(*argList++);
        }
    }
}
void SigchldHandler(int signum) {
    int status;
    pid_t pid;
    while((pid = waitpid(-1, &status, WNOHANG|WSTOPPED)) > 0){
        if(WIFEXITED(status)){
            printf("process exit normally\n");
            DeleteJob(pid);  
        }else if(WIFSIGNALED(status)){
            printf("process terminated by signal\n");
            DeleteJob(pid);
        }else if(WIFSTOPPED(status)){
            printf("process stopped bt signal\n");
            Job *jobPtr = GetJobPid(pid);
            jobPtr->jobstate = ST;
        }
    }
}

