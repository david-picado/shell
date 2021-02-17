// Group 15
// David Picado Liñares ---> david.picado - 58021112R
// Aarón Martínez Barreira ---> aaron.martinezb - 32723648F

#include "../include/list.h"


// Creacion de los nodos de la lista
bool createNodeM(tPosM * p) {
    *p = malloc(sizeof(struct tNodeM));
    return (*p == NULL?false:true);
}

// Funciones basicas de nuestra lista

void createEmptyListM(tListM *L) {
    *L = NULL;
}

bool insertItemM (tItemM d, tPosM p, tListM * L) {
    tPosM q, r;
    if (!createNodeM(&q)) {
        return false;
    }
    else {
        q->data = d;
        q->next = NULL;
        if (*L == NULL) {
            *L = q;
        }
        else if (p == NULL) {
            for (r = *L; r->next != NULL; r = r->next);
            r->next = q;
        }
        else if (p == *L) {
            q->next = *L;
            *L = q;
        }
        else {
            q->data = p->data;
            p->data = d;
            q->next = p->next;
            p->next = q;
        }
        return true;
    }
}

void deleteAtPositionM (tPosM p, tListM * L) {
    tPosM q;
    if (p == *L) {
        *L = (*L)->next;
    }
    else if (p->next == NULL) {
        for (q = *L; q->next != p; q = q->next);
        q->next = NULL;
    }
    else {
        q = p->next;
        p->data = q->data;
        p->next = q->next;
        p = q;
    }
    free(p);
}

bool isEmptyListM(tListM L) {
    if (L == NULL) {
        return true;
    }
    else {
        return false;
    }
}

tPosM lastM(tListM L) {
    tPosM p;
    for (p = L; p->next != NULL; p = p->next);
    return p;
}

void freeListM (tListM * L) { // Liberacion de memoria de la lista
    while (!isEmptyListM(*L)) {
        deleteAtPositionM(*L, L);
    }
}

