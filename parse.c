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
    printf("%s", PROMPT);
    if(fgets(readBuf->cmd, MAX_CMD_BUFFER, fp) == NULL)
        return -1;
    readBuf->cmdLen = strlen(readBuf->cmd) - 1;
    readBuf->cmd[readBuf->cmdLen] = '\0';
    return 0;
}
int ParseLine(ReadBuf *readBuf) {
    char *str = readBuf->cmd;
    if (str == NULL || *str == '\0') return -1;
    char **argList = readBuf->argList;
    int slot = 0;
    int validStr = 1;
    while (*str != '\0') {
        while (IsDelim(*str)) ++str;
        if (*str == '\0'){
            validStr = 0;
            break;
        } 
        int len = 0;
        char *start = str;
        while (*str != 0 && !IsDelim(*str)) ++str, ++len;
        int ret =  ParseRedirect(start, len, readBuf);
        if(ret == -1){
            validStr = 0;
            break;
        }else if(ret == 0){
            if(*str == 0 && IsForeGround(start, len, readBuf) == 0)break;
            if(len == 1 && *start == '|'){
                argList[slot++] = NULL;
                readBuf->processNum++;
            }else{
                char *tmp = GetString(start, len);
                argList[slot++] = tmp;
            }
        }
    }
    if (!validStr) {
        return -1;
    }
    argList[slot++] = NULL;
    readBuf->argLen = slot;
    return 0;
}
int ParseRedirect(char *str, int len, ReadBuf *readBuf){
    int find = 0;
    for(int i = 0; i < len; i++, str++){
        if(*str == '>'){
            if(i == 0){
                strncpy(readBuf->out, str + 1, len - 1);
                find = 1;
            }else if(i == 1){
                if(*(str - 1) == '1'){
                    strncpy(readBuf->out, str + 1, len - 2);
                    find = 1;
                }else if(*(str - 1) == '2'){
                    strncpy(readBuf->err, str + 1, len - 2);
                    find = 1;
                }else{
                    return -1;
                }
            }else{
                return -1;
            } 
        }else if(*str == '<'){
            if(i == 0){
                strncpy(readBuf->in, str + 1, len - 1);
                find = 1;
            } else if(i == 1 && *(str - 1) == '0'){
                strncpy(readBuf->in, str + 1, len - 2);
                find = 1;
            }else {
                return -1;
            }       
        }
    }
    return find;
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
int IsForeGround(char *start, int len, ReadBuf *readBuf){
    if(len == 1 && *start == '&'){
        readBuf->foreGround = 0;
        return 0;
    }else{
        readBuf->foreGround = 1;
        return 1;
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
