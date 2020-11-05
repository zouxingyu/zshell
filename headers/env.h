#ifndef ENV_H
#define ENV_H
#define MAXVARSIZE 512
typedef enum Scope { GLOBAL, LOCAL } Scope;
typedef struct Variable {
    char *str;
    Scope scope;
} Variable;
extern int varTableSize;
extern Variable varTable[MAXVARSIZE];
extern char **environ;
int IsValidAssignStr(char *str);
int VEnviron2Table(char *env[]);
char **VTable2Environ();
void VList();
void VListEnv(); 
Variable *FindVariable(char *name, int returnEmpty);
char *Concat(char *name, char *value);
char *VLookUp(char *name);
int VExport(char *str);
int VStore(char *name, char *value);
int Assign(char *str);

#endif 
