#ifndef JOB_H
#define JOB_H
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#define MAXSIZE 256
typedef struct Process Process;
typedef struct Job Job;
struct Process{
    Process *next;
    char **argList;
    pid_t pid;
    int status;
    char stopped;
    char completed;
};
struct Job{
    Job *next;           
    char *command;             
    Process *firstProcess;   
    pid_t pgid;                 
    int jid;
    char notified;
    struct termios tmodes;     
    int jstdin, jstdout, jstderr;  
};

extern char jidList[MAXSIZE];
extern Job *jobList;


int Redirect(Job *jobPtr, ReadBuf *readBuf);
Job *CreateJob(ReadBuf *readBuf, pid_t pgid, struct termios *tmodesPtr);  
Process *CreateProcess(char **argList);
void InsertJobList(Job *jobPtr);
void DeleteJobList(Job *jobPtr);
void DeleteJob(Job *jobPtr);
void DeleteProcessList(Process *ptr);

Job *GetJobJid(int jid);
void ListJobs();
void PutJobInFg(Job *jobPtr, int cont);
void PutJobInBg(Job *jobPtr, int cont);
int GetNextJid();
Job *FindJob(pid_t pgid);
int IsJobStopped(Job *jobPtr);
int IsJobCompleted(Job *jobPtr);
int GetJobState(Job *jobPtr);
void SwitchState(Job *jobPtr, int foreGround, int cont);
void WaitForJob(Job *jobPtr);
Job *ComposeJob(char *cmd, char **argList, pid_t pgid, struct termios *tmodesPtr);

#endif 
