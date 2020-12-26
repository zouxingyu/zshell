#ifndef PARSE_H
#define PARSE_H
#include <stdio.h>
#define MAX_CMD_BUFFER 1024
#define MAX_ARGS_BUFFER 8096
#define MAX_FILE_BUFFER 1024
typedef struct ReadBuf{
    char cmd[MAX_CMD_BUFFER];
    char *argList[MAX_ARGS_BUFFER];
    char in[MAX_FILE_BUFFER];
    char out[MAX_FILE_BUFFER];
    char err[MAX_FILE_BUFFER];
    int argLen;
    int cmdLen;
    int processNum;
    int foreGround;
}ReadBuf;
int GetInput(FILE *fp, ReadBuf *readBuf);
int ParseLine(ReadBuf *readBuf);
char *GetString(char *str, int len);
int IsDelim(int c);
int IsForeGround(char *start, int len, ReadBuf *readBuf);
int ParseRedirect(char *str, int len, ReadBuf *readBuf);
int IsValidRedirect(char *str);
char **Copy(char **start, char **end);
#endif
