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
int GetInput(FILE *fp, ReadBuf *readBuf) {
    if (fp == NULL) return -1;
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
    if (c == EOF || pos == 0) return -1;
    buf[pos] = '\0';
    readBuf->cmd = buf; 
    readBuf->cmdLen = pos;
    return 0;
}
int ParseLine(char *str, ReadBuf *readBuf) {
    in = out = err = NULL;
    if (str == NULL || *str == '\0') return -1;
    char **p;
    int slot = 0, listSize = 0;
    bool validStr = true;
    while (*str != '\0') {
        while (IsDelim(*str)) ++str;
        if (*str == '\0'){
            validStr = false;
            break;
        } 
        if (slot == 0) {
            p = Malloc(ARGSIZE);
            listSize += ARGSIZE;
        } else if (slot + 1 >= listSize / sizeof(char *)) {
            p = Realloc(p, listSize + ARGSIZE);
            listSize += ARGSIZE;
        }
        int len = 0;
        char *start = str;
        while (*str != '\0' && !IsDelim(*str)) ++str, ++len;
        if(ParseRedirect(start, len, readBuf) == -1){
            validStr = false;
            break;
        }else if(len == 1 && *str == '|'){
            p[slot++] = NULL;
            readBuf->process++;
        }else{
            char *tmp = GetString(start, len);
            p[slot++] = tmp;
        }
    }
    if (!validStr) {
        return -1;
    }
    p[slot] = NULL;
    readBuf->argList = p;
    return 0;
}
int ParseRedirect(char *str, int len, ReadBuf *readBuf){
    for(int i = 0; i < len; i++, str++){
        if(*str == '>'){
            if(i == 0){
                readBuf->out = GetString(str + 1, len - 1); 
            }else if(i == 1){
                if(*(str - 1) == '1')
                    readBuf->out = GetString(str + 1, len - 2);
                else if(*(str - 1) == '2')
                    readBuf->err = GetString(str + 1, len - 2);
                else return -1;
            }else{
                return =-1;
            } 
        }else if(*str == '<'){
            if(i == 0)
                readBuf->in = GetString(str + 1, len - 1);
            else if(i == 1 && *(str - 1) == '0')
                readBuf->in = GetString(str + 1, len - 2);
        }
    }
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
char **Copy(char **start, char **end){
    char **newArgList = Malloc(sizeof(char *) * (end - start + 1));
    char **temp = newArgList;
    while(start != end){
        *temp++ = GetString(*start, strlen(*start)); 
        start++;
    }
    *temp = NULL;
    return newArgList;
}
