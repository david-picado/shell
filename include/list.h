// Group 15
// David Picado Liñares ---> david.picado - 58021112R
// Aarón Martínez Barreira ---> aaron.martinezb - 32723648F

#ifndef P2_LIST_H
#define P2_LIST_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct tItemM {
    char tipoNodo; // Caracter que nos indica el tipo de memoria referida 'm'alloc 'M'map 'S'hared
    void * ptr;
    int bloques;
    struct tm time;
    key_t key;
    int df;
    ssize_t size;
    char * fich;
}tItemM;

typedef struct tNodeM * tPosM;
struct tNodeM {
    tItemM data;
    tPosM next;
};
typedef tPosM tListM;


// Funciones basicas de nuestra lista

void createEmptyListM(tListM *L);

bool insertItemM (tItemM d, tPosM p, tListM * L);

void deleteAtPositionM (tPosM p, tListM * L);

bool isEmptyListM(tListM L);

tPosM lastM(tListM L);

void freeListM (tListM * L);

#endif //P2_LIST_H
