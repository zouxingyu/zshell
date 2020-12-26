#include <stdio.h>
#include <stdlib.h>
#include "headers/util.h"
void Perror(char *str) {
    perror(str);
    exit(1);
}
void Ferror(char *str, int n) {
    fprintf(stderr, "%s\n", str);
    exit(n);
}
void *Malloc(size_t size) {
    char *p = malloc(size);
    if (p == NULL) Ferror("malloc", 1);
    return p;
}
void *Realloc(void *str, size_t size) {
    char *p = realloc(str, size);
    if (p == NULL) Ferror("realloc", 1);
    return p;
}
