#ifndef PARSE_H
#define PARSE_H
#include <stdio.h>
typedef struct ReadBuf{
    char *cmd;
    char **argList;
    char *in;
    char *out;
    char *err;
    int cmdLen;
    int processNum;
};
int GetInput(FILE *fp, ReadBuf *readBuf);
int ParseLine(char *str, ReadBuf *readBuf);
char *GetString(char *str, int len);
int IsDelim(int c);
int IsForeGround(char *cmd);
char *Redirect(char *str, int *which);
int IsValidRedirect(char *str);
char **Copy(char **start, char **end);
#endif
