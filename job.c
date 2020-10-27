#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#include "headers/function.h"
#include "headers/job.h"
#define FPRINTF(str)                  \
    do {                              \
        fprintf(stderr, "%s\n", str); \
        return NULL;                  \
    } while (0)
extern pid_t shellPgid;
void mode_to_letters(mode_t mode, char *modestr) {
    strcpy(modestr, "----------");
    if (S_ISDIR(mode))
        modestr[0] = 'd';
    else if (S_ISCHR(mode))
        modestr[0] = 'c';
    else if (S_ISBLK(mode))
        modestr[0] = 'b';
    if (mode & S_IRUSR) modestr[1] = 'r';
    if (mode & S_IWUSR) modestr[2] = 'w';
    if (mode & S_IXUSR) modestr[3] = 'x';
    if (mode & S_IRGRP) modestr[4] = 'r';
    if (mode & S_IWGRP) modestr[5] = 'w';
    if (mode & S_IXGRP) modestr[6] = 'x';
    if (mode & S_IROTH) modestr[7] = 'r';
    if (mode & S_IWOTH) modestr[8] = 'w';
    if (mode & S_IXOTH) modestr[9] = 'x';
}
int GetNextJid(char *jidList) {
    for (int i = 0; i < MAXSIZE; ++i) {
        if (jidList[i] == 0) {
            jidList[i] = 1;
            return i;
        }
    }
    return -1;
}
Job *InsertJobList(Job *jobList, Job *jobPtr) {
    if (jobList == NULL) {
        return jobPtr;
    } else {
        while (jobList->next) jobList = jobList->next;
        jobList->next = jobPtr;
        return jobList;
    }
}
Job *CreateJob(char *cmd, pid_t pgid, int jid, struct termios *tmodesPtr) {
    Job *jobPtr = Malloc(sizeof(Job));
    memset(jobPtr, 0, sizeof(Job));
    jobPtr->command = cmd;
    memcpy(&jobPtr->tmodes, tmodesPtr, sizeof(struct termios));
    jobPtr->pgid = pgid;
    jobPtr->jid = jid;
    jobPtr->jstdin = 0;
    jobPtr->jstdout = 1;
    jobPtr->jstderr = 2;
}
Process *CreateProcess(char **argList) {
    Process *ptr = Malloc(sizeof(Process));
    memset(ptr, 0, sizeof(Process));
    ptr->argList = argList;
    return ptr;
}
void DeleteJobList(Job *jobPtr) {
    while (jobPtr) {
        Job *next = jobPtr->next;
        DeleteJob(jobPtr);
        jobPtr = next;
    }
}
void DeleteJob(Job *jobPtr) {
    DeleteProcessList(jobPtr->firstProcess);
    jidList[jobPtr->jid] = 0;
    free(jobPtr->command);
    free(jobPtr);
}
void DeleteProcessList(Process *ptr) {
    while (ptr) {
        Process *next = ptr->next;
        FreeArgList(ptr->argList);
        free(ptr);
        ptr = next;
    }
}
Job *GetJobJid(Job *jobList, int jid) {
    while (jobList) {
        if (jobList->jid == jid) return jobList;
    }
    return NULL;
}
void ListJobs(Job *jobList) {
    while (jobList) {
        if (IfJobStopped(jobList)) {
            printf("job[%d] stopped %s\n", jobList->jid, jobList->command);
        } else {
            printf("job[%d] runnning %s &\n", jobList->jid, jobList->command);
        }
        jobList = jobList->next;
    }
}
void PutJobInFg(Job *jobPtr, int cont) {
    tcsetpgrp(STDIN_FILENO, jobPtr->pgid);
    if (cont) {
        if (kill(-jobPtr->pgid, SIGCONT) == -1) Perror("kill SIGCONT");
    }
    WaitForJob(jobPtr);
    tcsetpgrp(STDIN_FILENO, shellPgid);
}
void PutJobInBg(Job *jobPtr, int cont) {
    if (cont) {
        if (kill(-jobPtr->pgid, SIGCONT) == -1) Perror("kill SIGCONT");
    }
}
void SwitchState(Job *jobPtr, int foreGround, int cont) {
    if (foreGround) {
        PutJobInFg(jobPtr, cont);
    } else {
        PutJobInBg(jobPtr, cont);
    }
}
Job *FindJob(Job *jobList, pid_t pgid) {
    while (jobList != NULL) {
        if (jobList->pgid == pgid) return jobList;
    }
    return NULL;
}
int IfJobStopped(Job *jobPtr) {
    for (Process *ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
        if (!ptr->stopped && !ptr->completed) return 0;
    }
    return 1;
}
int IfJobCompleted(Job *jobPtr) {
    for (Process *ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
        if (!ptr->completed) return 0;
    }
    return 1;
}
int GetJobState(Job *jobPtr) {
    for (Process *ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
        if (!ptr->completed || !WIFEXITED(ptr->status)) return 1;
    }
    return 0;
}
void WaitForJob(Job *jobPtr) {
    while (!IfJobStopped(jobPtr)) sleep(0.1);
}
Job *ComposeJob(char *cmd, char **argList, pid_t pgid, int jid,
                struct termios *tmodesPtr) {
    Job *jobPtr = CreateJob(cmd, pgid, jid, tmodesPtr);
    char **start = argList;
    char **ptr = argList;
    Process *head = NULL, *tail;
    while (*argList) {
        if (!strcmp(*argList, "|")) {
            *ptr = NULL;
            if (head == NULL) {
                head = CreateProcess(start);
                tail = head;
            } else {
                Process *tmp = CreateProcess(start);
                tail->next = tmp;
                tail = tmp;
            }
            start = argList + 1;
            ptr = start;
            ++argList;
            continue;
        } else if (strchr(*argList, '>')) {
            printf("---------------------\n");
            char *tmp = *argList;
            int fd;
            if ((fd = open(tmp + 1, O_CREAT | O_TRUNC | O_WRONLY, 0777)) ==
                -1) {
                FPRINTF("redirection open file error");
            }
            struct stat stmp;
            if (stat(tmp + 1, &stmp) == -1) {
                perror("stat");
                return NULL;
            }
            char modestr[11];
            mode_to_letters(stmp.st_mode, modestr);
            printf("mode: %s\n", modestr);
            jobPtr->jstdout = fd;
            ++argList;
        } else if (strchr(*argList, '<')) {
            char *tmp = *argList;
            int fd;
            if ((fd = open(tmp + 1, O_RDONLY)) == -1) {
                FPRINTF("redirection open file error");
            }
            jobPtr->jstdin = fd;
            ++argList;
        }
        *ptr++ = *argList++;
    }
    if (head == NULL) {
        head = CreateProcess(start);
    }
    jobPtr->firstProcess = head;
    return jobPtr;
}
