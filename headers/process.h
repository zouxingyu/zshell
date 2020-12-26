#ifndef PROCESS_H
#define PROCESS_H 
#include <termios.h>
void SigchldHandler(int signum);
int Processing(ReadBuf *readBuf); 
int LaunchJob(ReadBuf *readBuf, pid_t pgid); 
int IsBuildIn(char **argList, int *ret);
int DoBgFg(char **argList);
#endif
