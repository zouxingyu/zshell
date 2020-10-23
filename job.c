#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include "headers/function.h"
#include "headers/job.h"
extern pid_t shellPgid;
int GetNextJid(char *jidList){
    for(int i = 0; i < MAXJOB; ++i){
        if(jidList[i] == 0){
            jidList[i] = 1;
            return i;
        }
    }
    return -1;
}
Job *InsertJobList(Job *jobList, Job *jobPtr){
    if(jobList == NULL){
        return jobPtr;
    }else{
        while(jobList->next)
            jobList = jobList->next;
        jobList->next = jobPtr;
        return jobList;
    }
}
Job *CreateJob(char *cmd, char **argList, pid_t pgid, int jid, struct termios *tmodesPtr){
    Job *jobPtr = Malloc(sizeof(Job));
    memset(jobPtr, 0, sizeof(Job));
    jobPtr->command = cmd;
    jobPtr->first_process = CreateProcessList(argList); 
    memcpy(&jobPtr->tmodes, tmodesPtr, sizeof(struct termios));
    jobPtr->pgid = pgid;
    jobPtr->jid = jid;
    jobPtr->next = NULL;
}  
Process *CreateProcessList(char **argList){
    Process *ptr = Malloc(sizeof(Process));
    memset(ptr, 0, sizeof(Process))
    ptr->argList = argList;
}
void DeleteJobList(Job *jobPtr){
    while(jobPtr){
        Job *next = jobPtr->next;
        DeleteJob(jobPtr);
        jobPtr = next;
    }
}
void DeleteJob(Job *jobPtr) {
    DeleteProcessList(jobPtr->first_process);
    jidList[jobPtr->jid] = 0;
    free(command);
    free(jobPtr);
}
void DeleteProcessList(Process *ptr){
    while(ptr){
        Process *next = ptr->next;
        FreeArgList(ptr->argList);
        free(ptr);
        ptr = next;
    }
}
//Job *GetJobJid(Job *jobPtr, int jid) {
//    while(jobPtr){
//        if(jobPtr->jid == jid)
//            return jobPtr;
//    }
//    return NULL;
//}
void ListJobs(Job *jobPtr){
    while(jobPtr){
        if(IfJobStopped(jobPtr)){
            printf("job[%d] stopped %s\n", jobPtr->jid, jobPtr->command);
        }else{
            printf("job[%d] runnning %s &\n", jobPtr->jid, jobPtr->command);
        }
        jobPtr=jobPtr->next;
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
    if(cont && jobPtr->jobstate == ST){
        if(kill(-jobPtr->pid, SIGCONT) == -1)
            Perror("kill SIGCONT");
    }
}
void ContinueJob(Job *jobPtr, int foreGround){
    if(foreGround){
        PutJobInFg(jobPtr, foreGround);
    }else{
        PutJobInBg(jobPtr, foreGround);
    }
}
Job *FindJob(Job *jobList, pid_t pgid){
    while(jobList != NULL){
        if(jobList->pgid == pgid)
            return jobList;
    }    
    return NULL;
}
int IfJobStopped(Job *jobPtr){
    for(Process *ptr = jobPtr->firstProcess; ptr != NULL: ptr = ptr->next){
        if(!ptr->stopped && !ptr->completed)
            return 0;
    }
    return 1;
}
int IfJobCompleted(Job *jobPtr){
    for(Process *ptr = jobPtr->firstProcess; ptr != NULL: ptr = ptr->next){
        if(!ptr->completed)
            return 0;
    }
    return 1;
}
int GetJobState(Job *jobPtr){
    for(Process *ptr = jobPtr->firstProcess; ptr != NULL: ptr = ptr-next){
        if(!pre->completed ||!WIFEXITED(ptr->status))
            reurn 1;
    }
    return 0;
}
void WaitForJob(Job *jobPtr){
    while(!IfJobStopped(jobPtr))
        sleep(0.1);
}
