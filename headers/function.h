#include <stdio.h>
char *GetInput(FILE *fp);
char **ParseLine(char *str);
char *GetString(char *str, int len);
int IsDelim(int c);

int IsBuildIn(char **argList, int *ret);
int Execute(char **argList, int foreground);
int Process(char **argList);
 
void SigchldHandler(int signum);

void Perror(char *str);
void Ferror(char *str, int n);
void *Malloc(size_t size);
void *Realloc(void *str, size_t size);
void FreeArgList(char **argList); 
