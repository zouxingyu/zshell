#ifndef FUNCTION_H
#define FUNCTION_H
#include <stdio.h>
#include "job.h"
char *GetInput(FILE *fp);
char **ParseLine(char *str);
char *GetString(char *str, int len);
int IsDelim(int c);
int IfForeGround(char **argList);
char *Redirect(char *str, int *which);

int IsBuildIn(Job *jobPtr, int *ret);
int Processing(Job *jobPtr, int foreGround);
int Execute(Job *jobPtr, int foreGround); 

void SigchldHandler(int signum);

void Perror(char *str);
void Ferror(char *str, int n);
void *Malloc(size_t size);
void *Realloc(void *str, size_t size);
void FreeArgList(char **argList);
void PrintArgList(char **argList);

#endif
