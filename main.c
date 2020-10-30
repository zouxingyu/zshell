#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "headers/function.h"
#include "headers/job.h"
pid_t shellPgid;
struct termios shellTmodes;
int shellTerminal;
int shellIsInteractive;
Job *jobList = NULL;
char jidList[MAXSIZE];
void InitShell() {
    shellTerminal = STDIN_FILENO;
    shellIsInteractive = isatty(shellTerminal);

    if (shellIsInteractive) {
        while (tcgetpgrp(shellTerminal) != (shellPgid = getpgrp()))
            kill(-shellPgid, SIGTTIN);

        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, SigchldHandler);

        shellPgid = getpid();
        if (setpgid(shellPgid, shellPgid) < 0)
            Perror("Couldn't put the shell in its own process group");
                tcsetpgrp(shellTerminal, shellPgid);

        tcgetattr(shellTerminal, &shellTmodes);
    }
}

int main(int argc, char *argv[]) {
    InitShell();
    int ret;
    char *cmd;
    char **argList;
    while ((cmd = GetInput(stdin)) != NULL) {
        if ((argList = ParseLine(cmd)) != NULL) {
            int jid = GetNextJid(jidList);
            int foreGround = IfForeGround(argList);
            Job *jobPtr = ComposeJob(cmd, argList, 0, jid, &shellTmodes);
            if(jobPtr == NULL) continue;
            Process *ptr = jobPtr->firstProcess;
            ret = Processing(jobPtr, foreGround);
        }
    }
    DeleteJobList(jobList);
    return 0;
}
