#ifndef PARSE_H
#define PARSE_H
#include <stdio.h>
char *GetInput(FILE *fp);
char **ParseLine(char *str);
char *GetString(char *str, int len);
int IsDelim(int c);
int IsForeGround(char *cmd);
char *Redirect(char *str, int *which);
int IsValidRedirect(char *str);
#endif
