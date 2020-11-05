#ifndef INIT_H
#define INIT_H
#include <termios.h>
extern int shellTerminal;
extern pid_t shellPgid;
extern int shellIsInteractive;
extern struct termios shellTmodes;
void InitShell();
#endif
