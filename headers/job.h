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
extern int foreGround;
extern pid_t shellPgid;
extern struct termios shellTmodes;
extern int shellTerminal;
extern int shellIsInteractive;

Job *CreateJob(char *cmd, char **argList, pid_t pgid, int jid, struct termios *tmodesPtr);  
Process *CreateProcessList(char **argList);
Job *InsertJobList(Job *jobList, Job *jobPtr);
void DeleteJobList(Job *jobPtr);
void DeleteJob(Job *jobPtr);
void DeleteProcessList(Process *ptr);

Job *GetJobJid(Job *jobList, int jid);
void ListJobs(Job *jobList);
void PutJobInFg(Job *jobPtr, int cont);
void PutJobInBg(Job *jobPtr, int cont);
int GetNextJid(char *jidList);
Job *FindJob(Job *jobList, pid_t pgid);
int IfJobStopped(Job *jobPtr);
int IfJobCompleted(Job *jobPtr);
int GetJobState(Job *jobPtr);
void SwitchState(Job *jobPtr, int foreGround, int cont);
void WaitForJob(Job *jobPtr);
int DoBgFg(Job *jobPtr); 

#endif 
