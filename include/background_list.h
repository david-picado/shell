// Group 15
// David Picado Liñares ---> david.picado - 58021112R
// Aarón Martínez Barreira ---> aaron.martinezb - 32723648F

#ifndef P3_BACKGROUND_LIST_H
#define P3_BACKGROUND_LIST_H

#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define BNULL (-1)
#define MAX 150

typedef char tCommandName[1024];

typedef struct tItemB {
    pid_t pid;
    int priority;
    tCommandName commandLine;
    struct tm time;
    char state[100];
    int terminated;
}tItemB;

typedef int tPosB;

typedef struct {
    tItemB data[MAX];
    tPosB lastPos;
}tListB;

void createEmptyListB(tListB * B);

bool insertItemB(tItemB d, tPosB, tListB * list);

void deleteAtPositionB(tPosB p, tListB * B);

void deleteListB(tListB * B);

bool isEmptyListB(tListB B);

#endif //P3_BACKGROUND_LIST_H
