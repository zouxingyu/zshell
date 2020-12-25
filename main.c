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

int varTableSize = 0;
Variable varTable[MAXVARSIZE];
Job *jobList = NULL;
char jidList[MAXSIZE];
int shellTerminal;
pid_t shellPgid;
int shellIsInteractive;
struct termios shellTmodes;

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
    bool stop = false;
    ReadBuf readBuf;
    while (!stop) {
        if(GetInput(stdin, &readBuf) == -1 || ParseLine(cmd, &readBuf) == -1)
            stop = true;
        if(!stop){
            ret = Processing(cmd, &readBuf);
        }
        FreeReadBuf(&readBuf);
    }
    return 0;
}
