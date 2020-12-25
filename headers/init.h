#ifndef INIT_H
#define INIT_H
#include <termios.h>
#define BUFFER_SIZE 8096
extern int shellTerminal;
extern pid_t shellPgid;
extern int shellIsInteractive;
extern struct termios shellTmodes;
#endif
