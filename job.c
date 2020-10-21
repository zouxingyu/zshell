#include "headers/function.h"
#include "headers/job.h"

void InitializeJobList() {
    for (int i = 0; i < MAXJOB; ++i) {
        CLearJob(&jobList[i]);
    }
}
void CLearJob(Job *jobPtr) {
    jobPtr->jobstate = NON;
    jobPtr->command[0] = '\0';
}
Job *GetJobPid(pid_t pid) {
    for (int i = 0; i < MAXJOB; ++i) {
        if (jobList[i].jobstate != NON && jobList[i].pid == pid)
            return &jobList[i];
    }
    return NULL;
}
Job *GetJobJid(int JobId) {
    for (int i = 0; i < MAXJOB; ++i) {
        if (jobList[i].jobstate != NON && jobList[i].pid == pid)
            return &jobList[i];
    }
    return NULL;
}
int DeleteJob(pid_t pid) {
    for (int i = 0; i < MAXJOB; ++i) {
        if (jobList[i].jobstate != NON && jobList[i].pid == pid) {
            CLearJob(&jobList[i]);
            return 0;
        }
    }
    return 1;
}
int Foreground(Job *jobPtr) { return jobPtr->jobstate == FG; }
void WaitForJob(Job *jobPtr) {
    while (Foreground(jobPtr)) {
        sleep(0.1);
    }
}
void PutJobInFg(Job *jobPtr, int cont) {
    tcsetpgrp(STDIN_FILENO, jobPtr->pgid);
    if(cont && jobPtr->jobstate == ST){
        if(kill(-jobPtr->pid, SIGCONT) == -1)
            Perror("kill SIGCONT");
    }
    jobPtr->jobstate = FG;
    WaitForJob(jobPtr);
    tcsetpgrp(STDIN_FILENO, shellPgid);
}
void PutJobInBg(Job *jobPtr, int cont) {
    tcsetpgrp(STDIN_FILENO, jobPtr->pgid);
    if(cont && jobPtr->jobstate == ST){
        if(kill(-jobPtr->pid, SIGCONT) == -1)
            Perror("kill SIGCONT");
    }
    jobPtr->jobstate = BG;
    WaitForJob(jobPtr);
    tcsetpgrp(STDIN_FILENO, shellPgid);
}
void ContinueJob(Job *jobPtr, int foreGround){
    if(foreGround){
        PutJobInFg(jobPtr, foreGround);
    }else{
        PutJobInBg(jobPtr, foreGround);
    }
}
Job *AddJob(pid_t pid, pid_t pgid, JobState state){
    for(int i = 0; i <MAXJOB; ++i){
        if(jobList[i].jobstate == NON){
            jobList[i].pid = pid;
            jobList[i].pgid = pgid;
            jobList[i].jobstate = state;
            return &jobList[i];
        }
    } 
    return NULL;
}
