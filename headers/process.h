#ifndef PROCESS_H
#define PROCESS_H 
#include <termios.h>
void SigchldHandler(int signum);
int Processing(char *cmd, char **argList); 
int LaunchJob(char *cmd, char **argList, pid_t pgid, int foreGround); 
int IsBuildIn(char **argList, int *ret);
int DoBgFg(char **argList);
#endif
