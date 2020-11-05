#include <signal.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <termios.h>
#include <unistd.h>

#include "headers/env.h"
#include "headers/init.h"
#include "headers/parse.h"
#include "headers/process.h"
#include "headers/util.h"
#include "headers/job.h"
int shellTerminal;
int shellIsInteractive;
struct termios shellTmodes;
pid_t shellPgid;
Job *jobList = NULL;
char jidList[MAXSIZE];
int varTableSize;
Variable varTable[MAXVARSIZE];

void InitShell() {
    int shellTerminal = STDIN_FILENO;
    int shellIsInteractive = isatty(shellTerminal);
    if (shellIsInteractive) {
        while (tcgetpgrp(shellTerminal) != (shellPgid = getpgrp()))
            kill(-shellPgid, SIGTTIN);

        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGCHLD, SigchldHandler);

        if (setpgid(shellPgid, shellPgid) < 0)
            Perror("Couldn't put the shell in its own process group");
        tcsetpgrp(shellTerminal, shellPgid);

        tcgetattr(shellTerminal, &shellTmodes);
    }
}
int main(int argc, char *argv[]) {
    InitShell();
    if (VEnviron2Table(environ)) Ferror("VEnviron2Table", 1);
    int ret;
    char *cmd;
    char **argList;
    while ((cmd = GetInput(stdin)) != NULL) {
        if ((argList = ParseLine(cmd)) != NULL) {
            ret = Processing(cmd, argList);
            FreeArgList(argList);
            free(argList);
            continue;
  n       }
        free(cmd);
    }
    return 0;
}
