#include <stdlib.h>
#include <string.h> 
#include <stdio.h> 
#include "headers/util.h"
#include "headers/parse.h"
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
    int whitespace = 0;
    printf("%s", PROMPT);
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\n') break;
        if(IsDelim(c)){
            if(whitespace)continue;
            whitespace = 1;
        }else{
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
        char *tmp = GetString(start, len);
        if(strchr(tmp, '>') || strchr(tmp, '<')){
            if(!IsValidRedirect(tmp)){
                fprintf(stderr, "invalid redirection syntax\n");
                free(argList);
                return NULL;
            }
        }
        argList[slot++] = tmp;
    }
    if (slot == 0) {
        free(argList);
        return NULL;
    }
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
int IsForeGround(char *cmd){
    char *pos = strchr(cmd, '&');
    if(pos == NULL){
        return 1;
    }else{
        ++pos;
        while(IsDelim(*pos))++pos;
        return *pos == '\0' ? 0 : 1;
    }
}
int IsValidRedirect(char *str){
    char *pos = str;
    while(*pos != '<' || *pos != '>') ++str;
    if(*pos == '<'){
        if(str != pos) return 0;
        if(*(pos + 1) == '\0') return 0;
    }else{
        if(pos != str + 1 || *str != '2' || *str != '1')
            return 0;
        if(*(pos + 1) == 0) return 0;
    } 
    return 1;
}
char *Redirect(char *str, int *which){
    if(*which == -1){
        return str + 1;
    }else if(*which == 1){
        *which = *str - '0';
        return str + 1;
    }
}
