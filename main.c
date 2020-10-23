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
    /* See if we are running interactively.  */
    shellTerminal = STDIN_FILENO;
    shellIsInteractive = isatty(shellTerminal);

    if (shellIsInteractive) {
        /* Loop until we are in the foreground.  */
        while (tcgetpgrp(shellTerminal) != (shellPgid = getpgrp()))
            kill(-shellPgid, SIGTTIN);

        /* Ignore interactive and job-control signals.  */
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, SigchldHandler);

        /* Put ourselves in our own process group.  */
        shellPgid = getpid();
        if (setpgid(shellPgid, shellPgid) < 0)
            Perror("Couldn't put the shell in its own process group");
                /* Grab control of the terminal.  */
                tcsetpgrp(shellTerminal, shellPgid);

        /* Save default terminal attributes for shell.  */
        tcgetattr(shellTerminal, &shellTmodes);
    }
}

int main(int argc, char *argv[]) {
    InitShell();
    int ret;
    char *cmd;
    char **argList;
    //printf("shell pid%d,pgid%d,foreground pgid%d\n", getpid(),getpgrp(),tcgetpgrp(0));
    while ((cmd = GetInput(stdin)) != NULL) {
        if ((argList = ParseLine(cmd)) != NULL) {
            int foreGround = IfForeGround(argList);
            int jid = GetNextJid(jidList);
            Job *jobPtr = CreateJob(cmd, argList, 0, jid, &shellTmodes);
            ret = Processing(jobPtr, foreGround);
        }
    }
    DeleteJobList(jobList);
    return 0;
}
