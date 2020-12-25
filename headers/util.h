#ifndef UTIL_H
#define UTIL_H
void Perror(char *str);
void Ferror(char *str, int n);
void *Malloc(size_t size);
void *Realloc(void *str, size_t size);
void FreeArgList(char **argList);
void PrintArgList(char **argList);
void FreeAll(char *cmd, char **argList, char *input, char *output);
#endif
