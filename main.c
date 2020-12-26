#include <signal.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
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
void FreeReadBuf(ReadBuf *readBuf){
    for(int i = 0; i < readBuf->argLen; i++){
        free(readBuf->argList[i]); 
    }
    memset(readBuf, 0, sizeof(ReadBuf));
}

int main(int argc, char *argv[]) {
    InitShell();
    if (VEnviron2Table(environ)) Ferror("VEnviron2Table", 1);
    int ret;
    ReadBuf readBuf;
    memset(&readBuf, 0, sizeof(ReadBuf));
    while (1) {
        if(GetInput(stdin, &readBuf) == -1)
           break;
        if(ParseLine(&readBuf) != -1) 
            ret = Processing(&readBuf);
        FreeReadBuf(&readBuf);
    }
    return 0;
}
