// Group 15
// David Picado Liñares ---> david.picado - 58021112R
// Aarón Martínez Barreira ---> aaron.martinezb - 32723648F

#include "../include/background_list.h"

void createEmptyListB(tListB *B) {
    B->lastPos = BNULL;
}

bool isEmptyListB(tListB B) {
    return (B.lastPos == BNULL);
}

bool insertItemB(tItemB d, tPosB p, tListB *L) {
    tPosB i;
    if (L->lastPos == MAX - 1) { // Lista completa
        return false;
    }
    else {
        L->lastPos++;
        if (p == BNULL) {
            L->data[L->lastPos] = d;
        }
        else {
            for (i = L->lastPos;i >= p + 1; --i) {
                L->data[i] = L->data[i-1];
            }
            L->data[p] = d;
        }
    }
    return true;
}

void deleteAtPositionB(tPosB p, tListB * B) {
    tPosB i;
    B->lastPos--;
    for (i = p; i <= B->lastPos; ++i) {
        B->data[i] = B->data[i + 1];
    }
}

void deleteListB(tListB * B) {
    while (B->lastPos != BNULL) {
        B->lastPos = BNULL; // El array seguirá estando en memoria
    }
}

