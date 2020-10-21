#include <unistd.h>
#define MAXJOB 256
#define MAXSIZE 256
typedef enum JobState {BG, FG, ST, RN, NON}JobState;
typedef struct Job{
    pid_t pid;
    pid_t pgid;
    int jobId;
    JobState jobstate;
    char command[MAXSIZE];
}Job;
extern Job jobList[MAXJOB];
extern int foreGround;
extern pid_t shellPgid;

void InitializeJobList();
Job *GetJobJid(int jobId);
Job *GetJobPid(pid_t pid);
void PutJobInBg(Job *jobPtr, int cont);
void PutJobInFg(Job *jobPtr, int cont);
void WaitForJob(Job *jobPtr);
int DeleteJob(pid_t pid);
void CLearJob(Job *jobPtr);
void ContinueJob(Job *jobPtr, int foreGround);
int Foreground(Job *jobPtr);
Job *AddJob(pid_t pid, pid_t pgid, JobState state);
void DoBgFg(char **argList);
