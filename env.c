#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "headers/env.h"
#include "headers/parse.h"
#include "headers/util.h"
int IsValidAssignStr(char *str) {
    if (!str || !*str) return 0;
    if (isdigit(*str++)) return 0;
    while (*str) {
        if (!(isalnum(*str)) || *str == '_') {
            return 0;
        }
        ++str;
    }
    return 1;
}
int VEnviron2Table(char *env[]) {
    varTableSize = 0;
    while (*env) {
        if (varTableSize == MAXVARSIZE) return -1;
        Variable *var = &varTable[varTableSize];
        var->scope = GLOBAL;
        char *tmp = GetString(*env, strlen(*env));
        if (tmp)
            var->str = tmp;
        else
            return -1;
         ++varTableSize;
        ++env;
    }
    return 0;
}
char **VTable2Environ() {
    char **env = Malloc(sizeof(Variable) * (varTableSize + 1));
    int i, j;
    for (i = 0, j = 0; i < varTableSize; ++i) {
        if (varTable[i].scope == GLOBAL) {
            env[j++] = varTable[i].str;
        }
    }
    env[j] = NULL;
    return env;
}
void VList() {
    for (int i = 0; i < varTableSize; ++i) {
        printf("%s\n", varTable[i].str);
    }
}

void VListEnv() {
    for (int i = 0; i < varTableSize; ++i) {
        if (varTable[i].scope == GLOBAL) printf("%s\n", varTable[i].str);
    }
}
Variable *FindVariable(char *name, int returnEmpty) {
    int len = strlen(name);
    for (int i = 0; i < varTableSize; ++i) {
        if (!strncmp(varTable[i].str, name, len) &&
            varTable[i].str[len] == '=') {
            return &varTable[i];
        }
    }
    if (varTableSize != MAXVARSIZE && returnEmpty) {
        return &varTable[varTableSize];
    } else {
        return NULL;
    }
}
char *Concat(char *name, char *value) {
    char *ptr = Malloc(strlen(name) + strlen(value) + 2);
    if (ptr) {
        sprintf(ptr, "%s=%s", name, value);
        return ptr;
    } else {
        return NULL;
    }
}
char *VLookUp(char *name) {
    Variable *var;
    if (var = FindVariable(name, 0)) {
        return var->str + 1 + strlen(name);
    } else {
        return NULL;
    }
}
int VExport(char *str) {
    if (!IsValidAssignStr(str)) return -1;
    Variable *var;
    if (var = FindVariable(str, 1)) {
        if (var->str) {
            var->scope = GLOBAL;
        } else {
            char *ptr = Concat(str, "");
            if (ptr) {
                var->str = ptr;
                var->scope = GLOBAL;
                ++varTableSize;
            } else {
                return -1;
            }
        }
        return 0;
    } else {
        return -1;
    }
}
int VStore(char *name, char *value) {
    Variable *var;
    char *str;
    if ((var = FindVariable(name, 1)) && (str = Concat(name, value))) {
        if (var->str) {
            free(var->str);
        } else {
            ++varTableSize;
        }
        var->str = str;
        var->scope = LOCAL;
        return 0;
    }
    return -1;
}

int Assign(char *str) {
    int ret;
    char *tmp = str;
    while (*tmp) {
        if (*tmp == '=') {
            *tmp = '\0';
            break;
        }
        ++tmp;
    }
    ret = (IsValidAssignStr(str) ? VStore(str, tmp + 1) : -1);
    *tmp = '=';
    return ret;
}
