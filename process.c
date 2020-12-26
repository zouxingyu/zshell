#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "headers/parse.h"
#include "headers/process.h"
#include "headers/env.h"
#include "headers/job.h"
#include "headers/util.h"
#include "headers/init.h"
int Processing(ReadBuf *readBuf) {
    int ret;
    if (!IsBuildIn(readBuf->argList, &ret)) {
        pid_t pgid = shellIsInteractive ? 0 : getpgrp();
        ret = LaunchJob(readBuf, pgid);
    }
    return ret;
}
int LaunchJob(ReadBuf *readBuf, pid_t pgid) {
    Job *jobPtr = CreateJob(readBuf, pgid, &shellTmodes); 
    int foreGround = readBuf->foreGround;
    Process *ptr;
    int myPipe[2], inFile, outFile, errFile;
    pid_t pid;
    inFile = jobPtr->jstdin;
    errFile = jobPtr->jstderr;
    sigset_t mask_all, mask_one, prev_one;
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
    for (ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
        if (ptr->next != NULL) {
            if (pipe(myPipe) < 0) Perror("pipe");
            outFile = myPipe[1];
        } else {
            outFile = jobPtr->jstdout;
        }
        pid = fork();
        if (pid == 0) {
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            ptr->pid = getpid();
            if (shellIsInteractive) {
                if (jobPtr->pgid == 0) jobPtr->pgid = ptr->pid;
                setpgid(ptr->pid, jobPtr->pgid);
                if (foreGround) tcsetpgrp(shellTerminal, jobPtr->pgid);
                //printf("pid:%d,pgid:%d,foreground pgid:%d\n", pid, pgid, tcgetpgrp(shellTerminal));
            }
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            if (inFile != STDIN_FILENO) {
                dup2(inFile, STDIN_FILENO);
                close(inFile);
            }
            if (outFile != STDOUT_FILENO) {
                dup2(outFile, STDOUT_FILENO);
                close(outFile);
            }
             if (errFile != STDERR_FILENO) {
                dup2(errFile, STDERR_FILENO);
                close(errFile);
            }
            environ = VTable2Environ();
            if(environ == NULL) Ferror("VTable2Environ failed", 1);
            execvp(ptr->argList[0], ptr->argList);
            Perror("execvp");
        } else if (pid > 0) {
            ptr->pid = pid;
            if (shellIsInteractive) {
                if (jobPtr->pgid == 0) jobPtr->pgid = pid;
                setpgid(pid, jobPtr->pgid);
            }
        } else {
            Perror("fork");
        }
        if (inFile != jobPtr->jstdin) close(inFile);
        if (outFile != jobPtr->jstdout) close(outFile);
        inFile = myPipe[0];
    }
    sigprocmask(SIG_BLOCK, &mask_all, NULL);
    InsertJobList(jobPtr);
    sigprocmask(SIG_SETMASK, &prev_one, NULL);
    if (!shellIsInteractive)
        WaitForJob(jobPtr);
    else if (foreGround)
        PutJobInFg(jobPtr, 0);
    else
        PutJobInBg(jobPtr, 0);
    return GetJobState(jobPtr);
}
int IsBuildIn(char **argList, int *ret) {
    if (!strcmp(argList[0], "quit")) {
        exit(0);
    } else if (!strcmp(argList[0], "jobs") ) {
        ListJobs(jobList);
        return 1;
    } else if ((!strcmp(argList[0], "bg") || !strcmp(argList[0], "fg"))) {
        *ret = DoBgFg(argList);
        return 1;
    } else if (!strcmp(argList[0], "set") ){
        VList();
    } else if(strchr(argList[0], '=') ){
        if(Assign(argList[0]) == -1)
            fprintf(stderr, "add exvironment variable failed\n");
        return 1;
    } else if(!strcmp(argList[0], "export")){
        if(VExport(argList[1]) == -1)
            fprintf(stderr, "export environment variable failed\n");
        return 1;
    }
    return 0;
}
int DoBgFg(char **argList) {
    if (argList[1] == NULL) return 1;
    int jid = atoi(argList[1]);
    Job *tmp = GetJobJid(jid);
    if (tmp == NULL) return -1;
    int cont = 0;
    if (IsJobStopped(tmp)) cont = 1;
    int fg = 1;
    if (!strcmp(argList[0], "bg")) {
        fg = 0;
    }
    SwitchState(tmp, fg, cont);
    return 0;
}
void SigchldHandler(int signum) {
    //printf("enter SigchldHandler\n");
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        Job *jobPre = NULL;
        Job *jobPtr = jobList;
        while(jobPtr){
            Process *ptr = jobPtr->firstProcess;
            while(ptr != NULL) {
                if (ptr->pid == pid) {
                    if (WIFEXITED(status)) {
                        ptr->completed = 1;
                    } else if (WIFSIGNALED(status)) {
                        ptr->completed = 1;
                    } else if (WIFSTOPPED(status)) {
                        ptr->stopped = 1;
                    } else if (WIFCONTINUED(status)) {
                        ptr->stopped = 0;
                    }
                    break;
                }
                ptr = ptr->next;
            }
            // remove this part later, put job delete into Processing function after Execute 
            if (ptr && IsJobCompleted(jobPtr)) {
                Job *tmp = jobPtr;
                if (jobPre == NULL) {
                    jobList = jobPtr->next;
                    jobPtr = jobList;
                } else {
                    jobPre->next = jobPtr->next;
                    jobPtr = jobPtr->next;
                }
                DeleteJob(tmp);
                break;
            } else if (ptr && IsJobStopped(jobPtr)) {
                break;
            }
            jobPre = jobPtr;
            jobPtr = jobPtr->next;
        }
    }
    if (pid == 0 || errno == ECHILD) return;
    else Perror("waitpid");
}
