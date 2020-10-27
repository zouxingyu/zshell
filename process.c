#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "headers/function.h"
#include "headers/job.h"
/* for debug */
int Processing(Job *jobPtr, int foreGround) {
   // printf("cmd:%s\njid:%d\nargList:\n", jobPtr->command, jobPtr->jid);
   // PrintArgList(jobPtr->firstProcess->argList);
    int ret;
    if (!IsBuildIn(jobPtr, &ret)) {
        ret = Execute(jobPtr, foreGround);
    }
    return ret;
}
int Execute(Job *jobPtr, int foreGround) {
    Process *ptr;
    int myPipe[2], inFile, outFile, errFile;
    pid_t pid;
    inFile = jobPtr->jstdin;
    errFile = jobPtr->jstderr;
    for (ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
        if (ptr->next != NULL) {
            if (pipe(myPipe) < 0) Perror("pipe");
            outFile = myPipe[1];
        } else {
            outFile = jobPtr->jstdout;
        }
        sigset_t mask_all, mask_one, prev_one;
        sigfillset(&mask_all);
        sigemptyset(&mask_one);
        sigaddset(&mask_one, SIGCHLD);
        sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
        pid = fork();
        if (pid == 0) {
            sigprocmask(SIG_SETMASK, &prev_one, NULL);
            pid_t pid, pgid = jobPtr->pgid;
            if (shellIsInteractive) {
                pid = getpid();
                if (pgid == 0) pgid = pid;
                setpgid(pid, pgid);
                if (foreGround) tcsetpgrp(shellTerminal, pgid);
                //printf("pid:%d,pgid:%d,foreground pgid:%d\n", pid, pgid, tcgetpgrp(shellTerminal));
            }
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
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
            execvp(ptr->argList[0], ptr->argList);
            Perror("execvp");
        } else if (pid > 0) {
            ptr->pid = pid;
            if (shellIsInteractive) {
                if (jobPtr->pgid == 0) jobPtr->pgid = pid;
                setpgid(pid, jobPtr->pgid);
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                jobList = InsertJobList(jobList, jobPtr);
                sigprocmask(SIG_SETMASK, &prev_one, NULL);
            }
        } else {
            Perror("fork");
        }
        if (inFile != jobPtr->jstdin) close(inFile);
        if (outFile != jobPtr->jstdout) close(outFile);
        inFile = myPipe[0];
    }
    if (!shellIsInteractive)
        WaitForJob(jobPtr);
    else if (foreGround)
        PutJobInFg(jobPtr, 0);
    else
        PutJobInBg(jobPtr, 0);
    return GetJobState(jobPtr);
}
int IsBuildIn(Job *jobPtr, int *ret) {
    char **argList = jobPtr->firstProcess->argList;
    if (!strcmp(argList[0], "quit")) {
        exit(0);
    } else if (!strcmp(argList[0], "jobs")) {
        ListJobs(jobList);
        return 1;
    } else if (!strcmp(argList[0], "bg") || !strcmp(argList[0], "fg")) {
        *ret = DoBgFg(jobPtr);
        return 1;
    }
    return 0;
}
int DoBgFg(Job *jobPtr) {
    char **argList = jobPtr->firstProcess->argList;
    if (argList[1] == NULL) return 1;
    int jid = atoi(argList[1]);
    Job *tmp = GetJobJid(jobList, jid);
    if (tmp == NULL) return -1;
    int cont = 0;
    if (IfJobStopped(tmp)) cont = 1;
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
        for (Job *jobPtr = jobList, *jobPrv = NULL; jobPtr != NULL;
             jobPtr = jobPtr->next) {
            Process *ptr;
            for (ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
                if (ptr->pid == pid) {
                    if (WIFEXITED(status)) {
                        printf("process exit normally\n");
                        ptr->completed = 1;
                    } else if (WIFSIGNALED(status)) {
                        printf("process terminated by signal[%d]\n",
                               WTERMSIG(status));
                        ptr->completed = 1;
                    } else if (WIFSTOPPED(status)) {
                        ptr->stopped = 1;
                    } else if (WIFCONTINUED(status)) {
                        ptr->stopped = 0;
                    }
                    break;
                }
            }
            if (ptr == NULL) continue;
            if (IfJobCompleted(jobPtr)) {
                printf("job[%d] completed %s\n", jobPtr->jid, jobPtr->command);
                if (jobPrv == NULL) {
                    jobList = jobPtr->next;
                } else {
                    jobPrv->next = jobPtr->next;
                }
                DeleteJob(jobPtr);
            } else if (IfJobStopped(jobPtr)) {
                printf("job[%d] stopped %s\n", jobPtr->jid, jobPtr->command);
            }
            jobPrv = jobPtr;
        }
    }
    if (pid == 0 || errno == ECHILD) return;
    else Perror("waitpid");
}
