#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "headers/function.h"
#include "headers/job.h"
int Processing(Job *jobPtr, int foreGround) {
    int ret;
    if (!IsBuildIn(jobPtr, &ret)) {
        ret = Execute(jobPtr, foreGround);
    }
    return ret;
}
int Execute(Job *jobPtr, int foreGround) {
    Process *ptr;
    int myPipe[2], inFile, outFile;
    pid_t pid;
    inFile = jobPtr->jstdin;
    for (ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
        if (p->next != NULL) {
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
            pid_t pid;
            if (shellIsInteractive) {
                pid = getpid();
                if (pgid == 0) pgid = pid;
                setpgid(pid, pgid);
                if (foreGround) tcsetattr(shellTerminal, pgid);
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTIN, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
            }
            signal(SIGCHLD, SigchldHandler);
        } else if (pid > 0) {
            p->pid = pid;
            if (shellIsInteractive) {
                if (jobPtr->pgid == 0) jobPtr->pgid = pid;
                setpgid(pid, jobPtr->pgid);
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                jobList = InsertJobList(jobList, jobPtr);
                sigprocmask(SIG_SETMASK, &prev_one, NULL);
            }
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
    if (argList[1] == NULL) return;
    int jobid = atoi(argList[1]);
    Job *jobPtr = GetJobJid(jobId);
    if (jobPtr == NULL || jobPtr->jobstate == NON) return;
    int fg = 1;
    if (!strcmp(argList[0], "bg")) {
        fg = 0;
    }
    (jobPtr, fg);
}
void SigchldHandler(int signum) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WSTOPPED | WCONTINUED)) > 0) {
        for (Job *jobPtr = jobList, *jobPrv = NULL; jobPtr != NULL;
             jobPtr = jobPtr->next) {
            Process *ptr;
            for (ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
                if (ptr->pid == pid) {
                    if (WIFEXITED(status)) {
                        ptr->completed = 1;
                    } else if (WIFSIGNALED(status)) {
                        fprintf(stderr, "process terminated by signal[%d]\n",
                                WTERMSIG(status));
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
                fprintf(stderr, "job[%d] completed %s\n", jobPtr->jid,
                        jobPtr->command);
                if(jobPrv == NULL){
                    jobList = jobPtr->next;
                }else{
                    jobPrv->next = jobPtr->next;
                }
                DeleteJob(jobPtr);
            } else if (IfJobStopped(jobPtr)) {
                fprintf(stderr, "job[%d] stopped %s\n", jobPtr->jid,
                        jobPtr->command);
            }
            jobPrv = jobPtr;
        }
    }
    if (pid == -1) Perror("waitpid");
}
