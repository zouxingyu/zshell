#include <string.h>
#include "headers/function.h"
#include "headers/job.h"
int Process(char **argList){
    int ret = 0;
    if(!IsBuildIn(argList, &ret)){
        ret = Execute(argList, foreGround);
    } 
    return ret;
}
int Execute(char **argList, int foreGround){

}
int IsBuildIn(char **argList, int *ret){
    if (!strcmp(argList[0], "quit")) {
        exit(0);
    } else if (!strcmp(argList[0], "jobs")) {
        ListJobs(jobs);
        return 1;
    } else if (!strcmp(argList[0], "bg") || !strcmp(argList[0], "fg")) {
        DoBgFg(argList);
        return 1;
    }
    return 0; 
}
