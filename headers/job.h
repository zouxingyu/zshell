#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#define MAXSIZE 256
typedef struct Process{
    struct Process *next;
    char **argList;
    pid_t pid;
    int status;
    char stopped;
    char completed;
}Process;
typedef struct Job{
    struct Job *next;           
    char *command;             
    Process *firstProcess;   
    pid_t pgid;                 
    int jid;
    char notified;
    struct termios tmodes;     
    int jstdin, jstdout, jstderr;  
}Job;
extern char jidList[MAXSIZE];
extern Job *jobList;
extern int foreGround;
extern pid_t shellPgid;
extern struct termios shellTmodes;
extern int shellTerminal;
extern int shellIsInteractive;

Job *CreateJob(char *cmd, char **argList, pid_t pgid, int jid, struct termios *tmodesPtr);  
Process *CreateProcessList(char **argList);
void InsertJobList(Job *jobList, Job *jobPtr);
void DeleteJobList(Job *jobPtr);
void DeleteJob(Job *jobPtr);
void DeleteProcessList(Process *ptr);

//Job *GetJobJid(Job *jobPtr, int jid);
void ListJobs(Job *jobPtr);
void PutJobInFg(Job *jobPtr, int cont);
void PutJobInBg(Job *jobPtr, int cont);
int GetNextJid(char *jidList);
Job *FindJob(pid_t pgid);
int IfJobStopped(Job *jobPtr);
int IfJobCompleted(Job *jobPtr);
int GetJobState(Job *jobPtr);
void ContinueJob(Job *jobPtr, int foreGround);
void WaitForJob(Job *jobPtr);
