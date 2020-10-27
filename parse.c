#include <string.h> 
#include <stdio.h> 
#include "headers/function.h" 
#define BUFSIZE 128 
#define ARGSIZE 64 
#define PROMPT "$"
#define FPRINTF(str) \
    do{ \
        fprintf(stderr, "%s\n", str); \
        return NULL; \
    }while(0)
char *GetInput(FILE *fp) {
    if (fp == NULL) return NULL;
    char *buf;
    int c;
    int bufSize = 0, pos = 0;
    int rein = 0, reout = 0;
    int whitespace = 0;
    printf("%s", PROMPT);
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') break;
        if(IsDelim(c)){
            if(whitespace)continue;
            whitespace = 1;
        }else{
            if(c == '>'){
                if(reout) FPRINTF("redirection syntax error");
                reout = 1;
            }else if(c == '<'){
                if(rein) FPRINTF("redirection syntax error");
                rein = 1;
            }
            whitespace = 0;
        }
        if (pos == 0) {
            buf = Malloc(BUFSIZE);
            bufSize += BUFSIZE;
        } else if (pos + 1 >= bufSize) {
            buf = Realloc(buf, bufSize + BUFSIZE);
            bufSize += BUFSIZE;
        }
        buf[pos++] = c;
    }
    if (c == EOF || pos == 0) return NULL;
    buf[pos] = '\0';
    return buf;
}
char **ParseLine(char *str) {
    if (str == NULL || *str == '\0') return NULL;
    char **argList;
    int slot = 0, listSize = 0;
    while (*str != '\0') {
        while (IsDelim(*str)) ++str;
        if (*str == '\0') break;
        if (slot == 0) {
            argList = Malloc(ARGSIZE);
            listSize += ARGSIZE;
        } else if (slot + 1 >= listSize / sizeof(char *)) {
            argList = Realloc(argList, listSize + ARGSIZE);
            listSize += ARGSIZE;
        }
        int len = 0;
        char *start = str;
        while (*str != '\0' && !IsDelim(*str)) ++str, ++len;
        argList[slot++] = GetString(start, len);
    }
    if (slot == 0) return NULL;
    argList[slot] = NULL;
    return argList;
}
char *GetString(char *str, int len) {
    if (str == NULL) return NULL;
    if (len == -1) len = strlen(str);
    char *p = Malloc(len + 1);
    p[len] = '\0';
    strncpy(p, str, len);
    return p;
}
int IsDelim(int c) { return c == 0x20 || c == 0x09; }
int IfForeGround(char **argList){
   while(*(argList + 1) != NULL){
        ++argList; 
   } 
   if(!strcmp(*argList, "&")) return 0;
   else return 1;
}
