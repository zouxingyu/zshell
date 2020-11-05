#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#include "headers/parse.h"
#include "headers/job.h"
#include "headers/util.h"
#include "headers/init.h"
int GetNextJid() {
    for (int i = 0; i < MAXSIZE; ++i) {
        if (jidList[i] == 0) {
            jidList[i] = 1;
            return i;
        }
    }
    return -1;
}
void InsertJobList(Job *jobPtr) {
    if (jobList == NULL) {
        jobList = jobPtr;
    } else {
        Job *tmp = jobList;
        while (tmp->next) tmp = tmp->next;
        tmp->next = jobPtr;
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
Job *GetJobJid(int jid) {
    Job *jobPtr = jobList;
    while (jobPtr) {
        if (jobPtr->jid == jid) return jobPtr;
    }
    return NULL;
}
void ListJobs() {
    Job *jobPtr = jobList;
    while (jobPtr) {
        if (IsJobStopped(jobPtr)) {
            printf("job[%d] stopped %s\n", jobPtr->jid, jobPtr->command);
        } else {
            printf("job[%d] runnning %s\n", jobPtr->jid, jobPtr->command);
        }
        jobPtr = jobPtr->next;
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
Job *FindJob(pid_t pgid) {
    Job *jobPtr = jobList;
    while (jobPtr != NULL) {
        if (jobPtr->pgid == pgid) return jobPtr;
    }
    return NULL;
}
int IsJobStopped(Job *jobPtr) {
    for (Process *ptr = jobPtr->firstProcess; ptr != NULL; ptr = ptr->next) {
        if (!ptr->stopped && !ptr->completed) return 0;
    }
    return 1;
}
int IsJobCompleted(Job *jobPtr) {
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
    int jid = jobPtr->jid;
    while (1) {
        if (jidList[jid] == 0) return;
        if (IsJobStopped(jobPtr)) return;
        sleep(0.1);
    }
}
Job *ComposeJob(char *cmd, char **argList, pid_t pgid,
                struct termios *tmodesPtr) {
    int jid = GetNextJid();
    Job *jobPtr = CreateJob(cmd, pgid, jid, tmodesPtr);
    char **start = argList;
    char **ptr = argList;
    Process *head = NULL, *tail;
    int success = 1;
    while (*argList && success) {
        if (!strcmp(*argList, "|")) {
            free(*argList);
            *ptr = NULL;
            if (head == NULL) {
                head = CreateProcess(start);
                tail = head;
            } else {
                Process *tmp = CreateProcess(start);
                tail->next = tmp;
                tail = tmp;
            }
            start = ++argList;
            ptr = start;
            continue;
        } else if (strchr(*argList, '>')) {
            int which = 1;
            char *tmp = Redirect(*argList, &which);
            // fprintf(stderr, "write file name %s which %d\n", tmp, which);
            int fd;
            if ((fd = open(tmp, O_CREAT | O_TRUNC | O_WRONLY, 0777)) == -1) {
                fprintf(stderr, "output redirection open file error\n");
                success = 0;
            }
            if (which == 1) {
                jobPtr->jstdout = fd;
            } else if(which == 2){
                jobPtr->jstderr = fd;
            }
            free(*argList);
            ++argList;
        } else if (strchr(*argList, '<')) {
            int which = -1;
            char *tmp = Redirect(*argList, &which);
            // fprintf(stderr, "read file name %s\n", tmp);
            int fd;
            if ((fd = open(tmp, O_RDONLY)) == -1) {
                fprintf(stderr, "input redirection open file error\n");
                success = 0;
            }
            jobPtr->jstdin = fd;
            free(*argList);
            ++argList;
        } else {
            *ptr++ = *argList++;
        }
    }
    if (!success) {
        free(jobPtr);
        return NULL;
    }
    *ptr = NULL;
    if (head == NULL) {
        head = CreateProcess(start);
        tail = head;
    } else {
        Process *tmp = CreateProcess(start);
        tail->next = tmp;
        tail = tmp;
    }
    jobPtr->firstProcess = head;
    return jobPtr;
}
