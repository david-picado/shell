// Group 15
// David Picado Liñares ---> david.picado - 58021112R
// Aarón Martínez Barreira ---> aaron.martinezb - 32723648F

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/mman.h>
#include <wait.h>
#include "../include/list.h"
#include <signal.h>
#include <sys/resource.h>
#include "../include/background_list.h"

#define LIM 1024
#define BUFFSIZE 4096

// DECLARACION DE LA LISTA

typedef char tCommandName[LIM];

typedef struct tItemL {
    tCommandName commandName[LIM];
    int indice; // Control de los comandos en la lista
    int control; // Variable que nos ayudará a saber cuantos trozos tiene la cadena troceada
} tItemL;

typedef struct tNode * tPosL;
struct tNode {
    tItemL data;
    tPosL next;
};
typedef tPosL tList;

// Creacion de los nodos de la lista
bool createNode(tPosL * p) {
    *p = malloc(sizeof(struct tNode));
    return (*p == NULL?false:true);
}

// Funciones basicas de nuestra lista

void createEmptyList(tList *L) {
    *L = NULL;
}

bool insertItem (tItemL d, tPosL p, tList * L) {
    tPosL q, r;
    if (!createNode(&q)) {
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

void deleteAtPosition (tPosL p, tList * L) {
    tPosL q;
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

bool isEmptyList(tList L) {
    if (L == NULL) {
        return true;
    }
    else {
        return false;
    }
}

tPosL last(tList L) {
    tPosL p;
    for (p = L; p->next != NULL; p = p->next);
    return p;
}

void freeList (tList * L) { // Liberacion de memoria de la lista
    while (!isEmptyList(*L)) {
        deleteAtPosition(*L, L);
    }
}

// FIN DE LA DECLARACION DE LA LISTA

// Funciones auxiliares

int recCount = 0; // Variable global para contar las recursividades
int global1 = 12; // Variable global para el comando memory
int global2 = 13; // Variable global para el comando memory

char letraTF (mode_t m) {
    switch (m & S_IFMT) {
        case S_IFSOCK: return 's'; // socket
        case S_IFLNK: return 'l'; // symbolic link
        case S_IFREG: return '-'; // normal file
        case S_IFBLK: return 'b'; // block device
        case S_IFDIR: return 'd'; // directory
        case S_IFCHR: return 'c'; // char device
        case S_IFIFO: return 'p'; // pipe
        default: return '?'; // desconocido, no debería aparecer
    }
}
char * convierteModo3 (mode_t m) {
    char * permisos;
    permisos = (char *) malloc (12);
    strcpy(permisos, "---------- ");

    permisos[0] = letraTF(m);
    if (m & S_IRUSR) permisos[1] = 'r'; // Propietario
    if (m & S_IWUSR) permisos[2] = 'w';
    if (m & S_IXUSR) permisos[3] = 'x';
    if (m & S_IRGRP) permisos[4] = 'r'; // Grupo
    if (m & S_IWGRP) permisos[5] = 'w';
    if (m & S_IXGRP) permisos[6] = 'x';
    if (m & S_IROTH) permisos[7] = 'r';
    if (m & S_IWOTH) permisos[8] = 'w';
    if (m & S_IXOTH) permisos[9] = 'x';
    if (m & S_ISUID) permisos[3] = 's'; // setuid, setgid y stickybit
    if (m & S_ISGID) permisos[6] = 's';
    if (m & S_ISVTX) permisos[9] = 't';
    return permisos;
}

void listLong (char ruta[], bool dire, bool hid, bool rec) {
    DIR * dir = NULL;
    struct dirent *file = NULL;
    struct stat sbuf; // Para tanto directorios como ficheros
    struct stat sb; // Para solo ficheros
    char buff[128];
    struct passwd pwent, * pwentp;
    struct group grp, *grpt;
    char dateString[250];
    struct tm time;
    char pathname[LIM];
    char linkname[LIM];
    char recursive[100][LIM];
    char * modes;
    int k = 0;
    getcwd(pathname, LIM);
    if (stat(ruta, &sb) == 0) {
        if (!S_ISDIR(sb.st_mode) || (strcmp(ruta, ".") != 0 && !dire)) { // Mostrar fichero o directoio
            if (ruta[0] != '.' || hid) {
                localtime_r((const time_t *) &sb.st_ctim, &time);
                strftime(dateString, sizeof(dateString), "%c", &time);
                printf("%s %10lu", dateString, sb.st_ino);
                if (!getpwuid_r(sb.st_uid, &pwent, buff, sizeof(buff), &pwentp)) printf(" %s", pwent.pw_name);
                else printf(" %d", sb.st_uid);

                if (!getgrgid_r(sb.st_gid, &grp, buff, sizeof(buff), &grpt)) printf(" %s", grp.gr_name);
                else printf(" %d", sb.st_gid);
                modes = convierteModo3(sb.st_mode);
                printf(" %s", modes); // Me muestra los permisos
                printf(" %9d", (int)sb.st_size); // El tamaño del fichero/directorio
                printf(" (%3d)", (int)sb.st_nlink); // Me muestra los links
                printf(" %s", ruta); // El nombre
                ssize_t r = readlink(ruta, linkname, LIM);
                if (r != -1) {
                    linkname[r] = '\0';
                    printf(" -> %s\n", linkname);
                }
                else printf("\n");
            }
        }
        else if (S_ISDIR(sb.st_mode)) { // Vamos a mostrar el contenido del directorio
            if (dire) {
                chdir(ruta);
                dir = opendir(".");
            }
            else {
                dir = opendir(".");
            }
            if (dir) {
                if (recCount == 0) printf("******** %s\n", ruta);
                while ((file = readdir(dir)) != NULL) {
                    if (file->d_name[0] != '.' || hid) {
                        if (stat(file->d_name, &sbuf) == 0) {
                            if ((rec && dire) || (strcmp(ruta, ".") == 0 && rec)) { // Caso recursivo para directorios
                                if (!S_ISDIR(sbuf.st_mode)) {
                                    localtime_r((const time_t *) &sbuf.st_ctim, &time);
                                    strftime(dateString, sizeof(dateString), "%c", &time);
                                    printf("%s %10lu", dateString, sbuf.st_ino);
                                    if (!getpwuid_r(sbuf.st_uid, &pwent, buff, sizeof(buff), &pwentp)) printf(" %s", pwent.pw_name);
                                    else printf(" %d", sbuf.st_uid);

                                    if (!getgrgid_r(sbuf.st_gid, &grp, buff, sizeof(buff), &grpt)) printf(" %s", grp.gr_name);
                                    else printf(" %d", sbuf.st_gid);
                                    modes = convierteModo3(sbuf.st_mode);
                                    printf(" %s", modes); // Me muestra los permisos
                                    printf(" %9d", (int)sbuf.st_size); // El tamaño del fichero/directorio
                                    printf(" (%3d)", (int)sbuf.st_nlink); // Me muestra los links
                                    printf(" %s", file->d_name); // El nombre
                                    ssize_t r = readlink(file->d_name, linkname, LIM);
                                    if (r != -1) {
                                        linkname[r] = '\0';
                                        printf(" -> %s\n", linkname);
                                    }
                                    else printf("\n");
                                }
                                else if (S_ISDIR(sbuf.st_mode)) {
                                    strcpy(recursive[k], file->d_name);
                                    k++;
                                    localtime_r((const time_t *) &sbuf.st_ctim, &time);
                                    strftime(dateString, sizeof(dateString), "%c", &time);
                                    printf("%s %10lu", dateString, sbuf.st_ino);
                                    if (!getpwuid_r(sbuf.st_uid, &pwent, buff, sizeof(buff), &pwentp)) printf(" %s", pwent.pw_name);
                                    else printf(" %d", sbuf.st_uid);

                                    if (!getgrgid_r(sbuf.st_gid, &grp, buff, sizeof(buff), &grpt)) printf(" %s", grp.gr_name);
                                    else printf(" %d", sbuf.st_gid);
                                    modes = convierteModo3(sbuf.st_mode);
                                    printf(" %s", modes); // Me muestra los permisos
                                    printf(" %9d", (int)sbuf.st_size); // El tamaño del fichero/directorio
                                    printf(" (%3d)", (int)sbuf.st_nlink); // Me muestra los links
                                    printf(" %s", file->d_name); // El nombre
                                    ssize_t r = readlink(file->d_name, linkname, LIM);
                                    if (r != -1) {
                                        linkname[r] = '\0';
                                        printf(" -> %s\n", linkname);
                                    }
                                    else printf("\n");
                                }
                            }
                            else {
                                localtime_r((const time_t *) &sbuf.st_ctim, &time);
                                strftime(dateString, sizeof(dateString), "%c", &time);
                                printf("%s %10lu", dateString, sbuf.st_ino);
                                if (!getpwuid_r(sbuf.st_uid, &pwent, buff, sizeof(buff), &pwentp)) printf(" %s", pwent.pw_name);
                                else printf(" %d", sbuf.st_uid);

                                if (!getgrgid_r(sbuf.st_gid, &grp, buff, sizeof(buff), &grpt)) printf(" %s", grp.gr_name);
                                else printf(" %d", sbuf.st_gid);
                                modes = convierteModo3(sbuf.st_mode);
                                printf(" %s", modes); // Me muestra los permisos
                                printf(" %9d", (int)sbuf.st_size); // El tamaño del fichero/directorio
                                printf(" (%3d)", (int)sbuf.st_nlink); // Me muestra los links
                                printf(" %s", file->d_name); // El nombre
                                ssize_t r = readlink(file->d_name, linkname, LIM);
                                if (r != -1) {
                                    linkname[r] = '\0';
                                    printf(" -> %s\n", linkname);
                                }
                                else printf("\n");
                            }
                        }
                        else {
                            if (lstat(file->d_name, &sbuf) == 0) {
                                localtime_r((const time_t *) &sbuf.st_ctim, &time);
                                strftime(dateString, sizeof(dateString), "%c", &time);
                                printf("%s %10lu", dateString, sbuf.st_ino);
                                if (!getpwuid_r(sbuf.st_uid, &pwent, buff, sizeof(buff), &pwentp)) printf(" %s", pwent.pw_name);
                                else printf(" %d", sbuf.st_uid);

                                if (!getgrgid_r(sbuf.st_gid, &grp, buff, sizeof(buff), &grpt)) printf(" %s", grp.gr_name);
                                else printf(" %d", sbuf.st_gid);
                                modes = convierteModo3(sbuf.st_mode);
                                printf(" %s", modes); // Me muestra los permisos
                                printf(" %9d", (int)sbuf.st_size); // El tamaño del fichero/directorio
                                printf(" (%3d)", (int)sbuf.st_nlink); // Me muestra los links
                                printf(" %s", file->d_name); // El nombre
                                ssize_t r = readlink(file->d_name, linkname, LIM);
                                if (r != -1) {
                                    linkname[r] = '\0';
                                    printf(" -> %s\n", linkname);
                                }
                                else printf("\n");
                            }
                            else perror("lstat: ");
                        }
                    }
                }
                for (int i = 0; strcmp(recursive[i], "") != 0; ++i) {
                    if (strcmp(recursive[i], "..") != 0 && strcmp(recursive[i], ".") != 0) {
                        int newpath_length;
                        char newpath[LIM];
                        recCount++;

                        newpath_length = snprintf(newpath, LIM, "%s/%s", ruta, recursive[i]);
                        if (newpath_length >= LIM) perror("path length es demasiado grande: ");
                        printf("******** %s\n", newpath);
                        if (strcmp(ruta, ".") == 0 && rec) dire = true;
                        listLong(recursive[i], dire, hid, rec);
                    }
                }
            }
            else perror("opendir: ");
        }
    }
    else {
        if (lstat(ruta, &sb) == 0) { // Segunda comprobación
            localtime_r((const time_t *) &sb.st_ctim, &time);
            strftime(dateString, sizeof(dateString), "%c", &time);
            printf("%s %10lu", dateString, sb.st_ino);
            if (!getpwuid_r(sb.st_uid, &pwent, buff, sizeof(buff), &pwentp)) printf(" %s", pwent.pw_name);
            else printf(" %d", sb.st_uid);

            if (!getgrgid_r(sb.st_gid, &grp, buff, sizeof(buff), &grpt)) printf(" %s", grp.gr_name);
            else printf(" %d", sb.st_gid);
            modes = convierteModo3(sb.st_mode);
            printf(" %s", modes); // Me muestra los permisos
            printf(" %9d", (int)sb.st_size); // El tamaño del fichero/directorio
            printf(" (%3d)", (int)sb.st_nlink); // Me muestra los links
            printf(" %s", ruta); // El nombre
            ssize_t r = readlink(ruta, linkname, LIM);
            if (r != -1) {
                linkname[r] = '\0';
                printf(" -> %s\n", linkname);
            }
            else printf("\n");
        }
        else perror("lstat: ");
    }
    closedir(dir);
    for (int i = 0; strcmp(recursive[i], "") != 0; ++i) {
        strcpy(recursive[i], "");
    }
    recCount = 0;
    chdir(pathname);
    free(modes);
}

void list (char ruta[], bool dir, bool hid, bool rec) {
    DIR * d = NULL;
    struct dirent *file = NULL;
    struct stat sbuf;
    struct stat sb;
    char pathname[LIM];
    char recursive[100][LIM];
    int k = 0;
    getcwd(pathname, LIM);
    if (stat(ruta, &sb) == 0) {
        if (!S_ISDIR(sb.st_mode) || (strcmp(ruta, ".") != 0 && !dir)) { // Mostrar fichero o directoio
            if (ruta[0] != '.' || hid) {

                printf("%9d %s\n", (int)sb.st_size, ruta); // El nombre y tamaño
            }
        }
        else if (S_ISDIR(sb.st_mode)) { // Vamos a mostrar el contenido del directorio
            if (dir) {
                chdir(ruta);
                d = opendir(".");
            }
            else {
                d = opendir(".");
            }
            if (d) {
                if (recCount == 0) printf("******** %s\n", ruta);
                while ((file = readdir(d)) != NULL) {
                    if (file->d_name[0] != '.' || hid) {
                        if (stat(file->d_name, &sbuf) == 0) {
                            if ((rec && dir) || (strcmp(ruta, ".") == 0 && rec)) { // Caso recursivo para directorios
                                if (!S_ISDIR(sbuf.st_mode)) {
                                    printf("%9d %s\n", (int)sbuf.st_size, file->d_name); // El nombre y tamaño
                                }
                                else if (S_ISDIR(sbuf.st_mode)) {
                                    strcpy(recursive[k], file->d_name);
                                    k++;
                                    printf("%9d %s\n", (int)sbuf.st_size, file->d_name); // El nombre y tamaño
                                }
                            }
                            else {
                                printf("%9d %s\n", (int)sbuf.st_size, file->d_name);
                            }
                        }
                        else {
                            if (lstat(file->d_name, &sbuf) == 0) {
                                printf("%9d %s\n", (int)sbuf.st_size, file->d_name); // El nombre y tamaño
                            }
                            else perror("lstat: ");
                        }
                    }
                }
                for (int i = 0; strcmp(recursive[i], "") != 0; ++i) {
                    if (strcmp(recursive[i], "..") != 0 && strcmp(recursive[i], ".") != 0) {
                        int newpath_length;
                        char newpath[LIM];
                        recCount++;

                        newpath_length = snprintf(newpath, LIM, "%s%s", ruta, recursive[i]);
                        if (newpath_length >= LIM) perror("path length es demasiado grande: ");
                        printf("******** %s\n", newpath);
                        if (strcmp(ruta, ".") == 0 && rec) dir = true;
                        list(recursive[i], dir, hid, rec);
                    }
                }
            }
            else perror("opendir: ");
        }
    }
    else {
        if (lstat(ruta, &sb) == 0) {
            printf("%9d %s\n", (int)sb.st_size, ruta); // El nombre y tamaño
        }
    }
    closedir(d);
    free(file);
    for (int i = 0; strcmp(recursive[i], "") != 0; ++i) {
        strcpy(recursive[i], "");
    }
    recCount = 0;
    chdir(pathname);
}

void list_options(char ruta[], bool longListed, bool dir, bool hid, bool rec) {
    if (longListed) { // -long
        if (dir && hid && !rec) {
            listLong(ruta, dir, hid, rec);
        }
        if (!dir && hid && !rec) {
            listLong(ruta, dir, hid, rec);
        }
        if (dir && !hid && !rec) {
            listLong(ruta, dir, hid, rec);
        }
        if (!dir && !hid && !rec) {
            listLong(ruta, dir, hid, rec);
        }
        if (dir && !hid && rec) {
            listLong(ruta, dir, hid, rec);
        }
        if (dir && hid && rec) {
            listLong(ruta, dir, hid, rec);
        }
        if (!dir && !hid && rec) {
            listLong(ruta, dir, hid, rec);
        }
        if (!dir && hid && rec) {
            listLong(ruta, dir, hid, rec);
        }
    }
    else {
        if (dir && hid && !rec) {
            list(ruta, dir, hid, rec);
        }
        if (!dir && hid && !rec) {
            list(ruta, dir, hid, rec);
        }
        if (dir && !hid && !rec) {
            list(ruta, dir, hid, rec);
        }
        if (!dir && !hid && !rec) {
            list(ruta, dir, hid, rec);
        }
        if (dir && !hid && rec) {
            list(ruta, dir, hid, rec);
        }
        if (dir && hid && rec) {
            list(ruta, dir, hid, rec);
        }
        if (!dir && !hid && rec) {
            list(ruta, dir, hid, rec);
        }
        if (!dir && hid && rec) {
            list(ruta, dir, hid, rec);
        }
    }
}

void remove_dir(const char path[], bool rec) {
    DIR * dir = NULL;
    char fullPath[LIM];
    struct stat stat_path, stat_entrada;
    struct dirent * entrada = NULL;
    if (rec) {
        stat(path, &stat_path);
        dir = opendir(path);
        if (dir) {
            while ((entrada = readdir(dir)) != NULL) {
                if (strcmp(entrada->d_name, ".") != 0 && strcmp(entrada->d_name, "..") != 0) {
                    strcpy(fullPath, path);
                    strcat(fullPath, "/");
                    strcat(fullPath, entrada->d_name);

                    stat(fullPath, &stat_entrada);
                    if (S_ISDIR(stat_entrada.st_mode)) { // Si nos encontramos un directorio
                        remove_dir(fullPath, true);
                        unlink(fullPath);
                    }
                    else remove(fullPath);

                }
            }
            rmdir(path);
            closedir(dir);
        }
        else perror("Delete was not satisfactory: ");
    }
    else {
        if (rmdir(path) != 0) {
            printf("Delete was not satisfactory: %s is not empty\n", path);
        }
    }
}

int castToInt(char *param) {  // Funcion para convertir un String en un entero
    int x;
    sscanf(param, "%d", &x);
    return x;
}

int trocearCadena (char * cadena, char * trozos[]) { // Funcion para trocear una cadena en tantos trozos necesarios
    int i = 1;

    if ((trozos[0] = strtok(cadena, " ")) == NULL) {
        return 0;
    }
    while ((trozos[i] = strtok(NULL, " ")) != NULL) {
        i++;
    }

    return i;
}

void comprobarComando (tItemL item, int control) { // Funcion para comprobar el tipo de salida del comando historic
    if (control == 1) { // Comando historic simple
        if (strcmp(item.commandName[1], "") != 0) {
            printf("%d->", item.indice);
            for (int i = 0; strcmp(item.commandName[i], "") != 0; ++i) {
                printf("%s ", item.commandName[i]);
            }
            printf("\n");
        }
        else if (strcmp(item.commandName[1], "") == 0) {
            printf("%d->%s\n", item.indice, item.commandName[0]);
        }
    }
    else if (control == 2) { // Comando historic -rN
        if (strcmp(item.commandName[1], "") != 0) {
            printf("Ejecutando historic (%d): ", item.indice);
            for (int i = 0; strcmp(item.commandName[i], "") != 0; ++i) {
                printf("%s ", item.commandName[i]);
            }
            printf("\n");
        }
        else if (strcmp(item.commandName[1], "") == 0) {
            printf("Ejecutando historic (%d): %s\n", item.indice, item.commandName[0]);
        }
    }

}

void leerEntrada (tList * list, int * indice) { // Leemos la entrada de nuetro Shell
    char * troceado[LIM];
    char string[LIM];
    struct tItemL newItem;
    printf(">>> ");
    scanf("%[^\n]%*c", string);

    newItem.control = 0;

    if (isEmptyList(*list)) newItem.indice = 0;
    else {
        newItem.indice = last(*list)->data.indice;
        newItem.indice++;
    }
    *indice = newItem.indice;
    newItem.control = trocearCadena(string, troceado); // Se hace el troceado de la entrada del usuario;
    for (int i = 0; i < newItem.control; ++i) {
        strcpy(newItem.commandName[i], troceado[i]);
    }
    insertItem(newItem, NULL, list); // Introducimos el comando en la lista historial
    for (int i = 0; i < LIM; ++i) {
        strcpy(newItem.commandName[i], "");
    }
}

// Funciones auxiliares dedicadas exclusivamente a memoria

void * ObtenerMemoriaShmget (key_t clave, size_t tam, tListM * list_nodes) {
    void * p;
    int aux, id, flags = 0777;
    struct shmid_ds s;
    time_t t;
    struct tm *timeInfo;
    tItemM item;

    if (tam) { // si tam no es 0 la crea en modo exclusivo
        flags = flags | IPC_CREAT | IPC_EXCL;
    }
    if (clave == IPC_PRIVATE) {
        errno = EINVAL;
        return NULL;
    }// no nos vale
    if ((id = shmget(clave, tam, flags)) == -1) {
        return NULL;
    }
    if ((p = shmat(id, NULL, 0)) == (void*) - 1) {
        aux = errno;
        if (tam)
            shmctl(id, IPC_RMID, NULL);
        errno = aux;
        return NULL;
    }
    shmctl(id, IPC_STAT, &s);
    item.tipoNodo = 'S';
    item.ptr = p;
    item.key = clave;
    item.size = s.shm_segsz;
    time(&t);
    timeInfo = localtime(&t);
    item.time = *timeInfo;
    insertItemM(item, NULL, list_nodes);
    return p;
}

void Cmd_AlocateCreateShared (char * arg[], tListM * list_nodes) {
    key_t k;
    size_t tam = 0;
    void * p;
    tPosM pos;
    tItemM iterator;
    char dateString[250];

    if (arg[0] == NULL || arg[1] == NULL) {
        if (!isEmptyListM(*list_nodes)) {
            pos = *list_nodes;
            while (pos != NULL) {
                iterator = pos->data;
                if (iterator.tipoNodo == 'S') {
                    strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                    printf("%p:   size:%d. shared memory (key %d) %s\n", iterator.ptr, (int)iterator.size, iterator.key, dateString);
                }
                pos = pos->next;
            }
        }
        else {
            printf("Empty list\n");
        }
        return;
    }
    k = (key_t) atoi(arg[0]);

    if (arg[1] != NULL)
        tam = (size_t) atoll(arg[1]);
    if ((p = ObtenerMemoriaShmget(k, tam, list_nodes)) == NULL)
        perror ("Imposible obtener memoria shmget");
    else
        printf("Memoria de shmget de clave %d asignada en %p\n", k, p);
}

void * MmapFichero (char * fichero, int protection, tListM * list_nodes) {
    int df, map = MAP_PRIVATE, modo = O_RDONLY;

    struct stat s;
    void * p;
    tItemM item;
    time_t t;
    struct tm *timeInfo;


    if (protection&PROT_WRITE) modo = O_RDWR;

    if (stat(fichero, &s) == -1 || (df = open(fichero, modo)) == -1)
        return NULL;
    if ((p = mmap(NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED)
        return NULL;
    item.size = s.st_size;
    item.ptr = p;
    item.df = df;
    item.fich = fichero;
    time(&t);
    timeInfo = localtime(&t);
    item.time = *timeInfo;
    item.tipoNodo = 'M';
    insertItemM(item, NULL, list_nodes);
    return p;
}

void Cmd_AllocateMmap(char * arg[], tListM * list_nodes) {
    char * perm;
    void * p;
    int protection = 0;
    tPosM pos;
    tItemM iterator;
    char dateString[250];

    if (arg[0] == NULL) {
        if (!isEmptyListM(*list_nodes)) {
            pos = *list_nodes;
            while (pos != NULL) {
                iterator = pos->data;
                if (iterator.tipoNodo == 'M') {
                    strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                    printf("%p:   size:%d. mmap %s (fd:%d) %s\n", iterator.ptr, (int)iterator.size, iterator.fich, iterator.df, dateString);
                }
                pos = pos->next;
            }
        }
        else {
            printf("Empty list\n");
        }
        return;
    }
    if ((perm = arg[1]) != NULL && strlen(perm) < 4) {
        if (strchr(perm, 'r') != NULL) protection |= PROT_READ;
        if (strchr(perm, 'w') != NULL) protection |= PROT_WRITE;
        if (strchr(perm, 'x') != NULL) protection |= PROT_EXEC;
    }
    if ((p = MmapFichero(arg[0], protection, list_nodes)) == NULL)
        perror("Imposible mapear fichero");
    else
        printf("fichero %s mapeado en %p\n", arg[0], p);
}

#define LEERCOMPLETO ((ssize_t) - 1)
ssize_t leerFichero (char * fich, void * p, ssize_t n) {
    ssize_t nleidos, tam = n;
    int df, aux;

    struct stat s;

    if (stat(fich, &s) == -1 || (df = open(fich, O_RDONLY)) == -1)
        return ((ssize_t) - 1);
    if (n == LEERCOMPLETO)
        tam = (ssize_t) s.st_size;
    if ((nleidos = read(df, p, tam)) == -1) {
        aux = errno;
        close(df);
        errno = aux;
        return ((ssize_t) - 1);
    }
    close(df);

    return nleidos;
}

void Cmd_deletekey(char * args[]) {
    key_t clave;
    int id;
    char * key = args[0];

    if (key == NULL || (clave = (key_t) strtoul(key, NULL, 10)) == IPC_PRIVATE) {
        printf("   rmkey  clave_valida\n");
        return;
    }
    if ((id = shmget(clave, 0, 0666)) == -1) {
        perror("shmget: imposible obtener memoria compartida");
        return;
    }
    if (shmctl(id, IPC_RMID, NULL) == -1)
        perror("shmctl: imposible eliminar memoria compartida\n");
    else
        printf("Borrado de la clave %d completado\n", clave);
}

void Cmd_dopmap(char * args[]) {
    pid_t pid;
    char elpid[32];
    char *argv[3] = {
            "pmap", elpid, NULL
    };

    sprintf(elpid, "%d", (int)getpid());
    if ((pid = fork()) == -1) {
        perror("Imposible crear proceso");
        return;
    }
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1)
            perror("Cannot execute pmap");
        exit(1);
    }
    waitpid(pid, NULL, 0);
}

void dealloc(char tipo, tListM * list_nodes, tPosL p) {
    tItemM iterator;
    tPosM pos;
    char dateString[250];
    if (tipo == 'm') { // dealloc malloc
        if (strcmp(p->data.commandName[3], "") != 0) {
            // Vamos a borrar la primera vez que tengamos un bloque de memoria del tamaño dado
            if (!isEmptyListM(*list_nodes)) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    if (iterator.tipoNodo == 'm' && iterator.bloques == castToInt(p->data.commandName[3])) {
                        free(iterator.ptr);
                        deleteAtPositionM(pos, list_nodes);
                        printf("Deallocation of %p was successful (malloc)\n", iterator.ptr);
                        break;
                    }
                    pos = pos->next;
                }
            }
        }
        else { // Mostramos la lista
            if (!isEmptyListM(*list_nodes)) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    if (iterator.tipoNodo == 'm') {
                        strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                        printf("%p:     size:%d malloc %s\n", iterator.ptr, iterator.bloques, dateString);
                    }
                    pos = pos->next;
                }
            }
            else {
                printf("Empty list\n");
            }
        }
    }

    else if (tipo == 'M') { // dealloc mmap
        if (strcmp(p->data.commandName[3], "") != 0) {
            // Borramos la primera iteración encontrada
            if (!isEmptyListM(*list_nodes)) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    if (iterator.tipoNodo == 'M' && strcmp(iterator.fich, p->data.commandName[3]) == 0) {
                        munmap(iterator.ptr, iterator.size);
                        deleteAtPositionM(pos, list_nodes);
                        printf("Unmap successful of %p (mmap)\n", iterator.ptr);
                        break;
                    }
                    pos = pos->next;
                }
            }
        }
        else { // Mostramos la lista
            if (!isEmptyListM(*list_nodes)) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    if (iterator.tipoNodo == 'M') {
                        strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                        printf("%p:   size:%d. mmap %s (fd:%d) %s\n", iterator.ptr, (int)iterator.size, iterator.fich, iterator.df, dateString);
                    }
                    pos = pos->next;
                }
            }
            else {
                printf("Empty list\n");
            }
        }
    }

    else { // dealloc shared
        if (strcmp(p->data.commandName[3], "") != 0) {
            if (!isEmptyListM(*list_nodes)) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    if (iterator.tipoNodo == 'S' && iterator.key == (key_t) castToInt(p->data.commandName[3])) {
                        shmdt(iterator.ptr);
                        printf("Dealloc of %p was successful (shared)\n", iterator.ptr);
                        deleteAtPositionM(pos, list_nodes);
                        break;
                    }
                }
            }
        }
        else { // Mostramos la lista
            if (!isEmptyListM(*list_nodes)) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    if (iterator.tipoNodo == 'S') {
                        strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                        printf("%p:   size:%d. shared memory (key %d) %s\n", iterator.ptr, (int)iterator.size, iterator.key, dateString);
                    }
                    pos = pos->next;
                }
            }
            else {
                printf("Empty list\n");
            }
        }
    }
}

/** Print and change the uids **/
char * nombreUsuario(uid_t uid) {
    struct passwd * p;
    if ((p = getpwuid(uid)) == NULL)
        return (" ??????");
    return p->pw_name;
}

uid_t uidUsuario(char * nombre) {
    struct passwd * p;
    if ((p = getpwnam(nombre)) == NULL)
        return (uid_t) -1;
    return p->pw_uid;
}

void Cmd_getuid(char * tr[]) {
    uid_t real = getuid(), efec = geteuid();

    printf("Credencial real: %d, (%s)\n", real, nombreUsuario(real));
    printf("Credencial efectiva: %d, (%s)\n", efec, nombreUsuario(efec));
}

int Cmd_setuid(char * tr[]) {
    uid_t uid;
    int u;
    if (tr[0] == NULL || (!strcmp(tr[0], "-l") && tr[1] == NULL)) {
        Cmd_getuid(tr);
        return 0;
    }
    if (!strcmp(tr[0], "-l")) {
        if ((uid = uidUsuario(tr[1])) == (uid_t) -1) {
            printf("Usuario no existente %s\n", tr[1]);
            return -1;
        }
    }
    else if ((uid = (uid_t) ((u = atoi(tr[0])) < 0) ? -1: u) == (uid_t) - 1) {
        printf("Valor no valido de la credencial %s\n", tr[0]);
        return -1;
    }
    if (setuid(uid) == -1) {
        printf("Imposible cambiar la credencial: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

/********* SEÑALES ********/

struct SEN {
    char * nombre;
    int senal;
};
static struct SEN sigstrnum[] = {
        "HUP", SIGHUP,
        "INT", SIGINT,
        "QUIT", SIGQUIT,
        "ILL", SIGILL,
        "TRAP", SIGTRAP,
        "ABRT", SIGABRT,
        "IOT", SIGIOT,
        "BUS", SIGBUS,
        "FPE", SIGFPE,
        "KILL", SIGKILL,
        "USR1", SIGUSR1,
        "USR2", SIGUSR2,
        "PIPE", SIGPIPE,
        "ALRM", SIGALRM,
        "TERM", SIGTERM,
        "CHLD", SIGCHLD,
        "CONT", SIGCONT,
        "STOP", SIGSTOP,
        "TSTP", SIGTSTP,
        "TTIN", SIGTTIN,
        "TTOU", SIGTTOU,
        "URG", SIGURG,
        "XCPU", SIGXCPU,
        "XFSZ", SIGXFSZ,
        "VTALRM", SIGVTALRM,
        "PROF", SIGPROF,
        "WINCH", SIGWINCH,
        "IO", SIGIO,
        "SYS", SIGSYS,
#ifdef SIGPOLL
       "POLL", SIGPOLL,
#endif
#ifdef SIGPWR
       "PWR", SIGPWR,
#endif
#ifdef SIGEMT
       "EMT", SIGEMT,
#endif
#ifdef SIGINFO
        "INFO", SIGINFO,
#endif
#ifdef SIGSTKFLT
        "STKFLT", SIGSTKFLT,
#endif
#ifdef SIGCLD
        "CLD", SIGCLD,
#endif
#ifdef SIGLOST
        "LOST", SIGLOST,
#endif
#ifdef SIGCANCEL
       "CANCEL", SIGCANCEL,
#endif
#ifdef SIGTHAW
       "THAW", SIGTHAW,
#endif
#ifdef SIGFREEZE
        "FREEZE", SIGFREEZE,
#endif
#ifdef SIGLWP
       "LWP", SIGLWP,
#endif
#ifdef SIGWAITING
       "WAITING", SIGWAITING,
#endif
        NULL, -1
};

int senal(char * sen) { // Devuelve el numero de la señal a partir del nombre
    int i;
    for (i = 0; sigstrnum[i].nombre != NULL; i++)
        if (!strcmp(sen, sigstrnum[i].nombre))
            return sigstrnum[i].senal;
    return -1;
}

char * nombreSenal(int sen) { // Devuelve el nomre de la señal a partir de la misma señal
    int i;
    for (i = 0; sigstrnum[i].nombre != NULL; i++)
        if (sen == sigstrnum[i].senal)
            return sigstrnum[i].nombre;
    return ("SIGUNKNOW");
}

/********* FIN DE LAS SEÑALES ***************/

// COMIENZO DE LAS DECLARACIONES DE LOS COMANDOS

void comandoTime() { // Implementacion del comando "time"
    time_t s;
    struct tm* horario;

    s = time(NULL);
    horario = localtime(&s);

    printf("%02d:%02d:%02d\n", horario->tm_hour, horario->tm_min, horario->tm_sec);
}

void comandoDate() { // Implementacion del comando "date"
    time_t s;
    struct tm* fecha;

    s = time(NULL);
    fecha = localtime(&s);

    printf("%02d/%02d/%02d\n", fecha->tm_mday, fecha->tm_mon + 1, fecha->tm_year + 1900);

}

void comandoPwd(char string[LIM]) { // Implementacion del comando "pwd"
    char * cwd;

    cwd = getcwd(string, LIM);

    if (cwd != NULL) {
        printf("%s\n", cwd);
    }
    else {
        perror("getcwd() error");
    }
}

void comandoChdir(char string[], int control) { // Implementacion del comando "chdir"
    if (control == 1) {
        comandoPwd(string);
    }
    else {
        if (chdir(string) != 0) {
            perror("Imposible cambiar directorio");
        }
    }
}

void comandoAuthors(char string[], int control) { // Implementacion del comando "authors"
    if (control == 1) {
        printf("David Picado Liñares: david.picado\n");
        printf("Aaron Martinez Barreira: aaron.martinezb\n");
    }
    else {
        if (strcmp(string, "-l") == 0) {
            printf("david.picado\n");
            printf("aaron.martinezb\n");
        }
        else if (strcmp(string, "-n") == 0) {
            printf("David Picado Liñares\n");
            printf("Aaron Martinez Barreira\n");
        }
    }
}

void comandoList(tPosL p) { // Implementación del comando "list"
    char parameters[100][LIM];
    bool isLong = false; // Referirnos a long listing
    bool isDir = false; // Referirnos a dir
    bool isHid = false; // Referirnos a hid
    bool isRec = false; // Referirnos a rec
    int j = 0;
    if (strcmp(p->data.commandName[1], "") == 0) strcpy(parameters[0], "");
    for (int i = 1; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (strcmp(p->data.commandName[i], "-long") == 0) isLong = true;
        if (strcmp(p->data.commandName[i], "-dir") == 0) isDir = true;
        if (strcmp(p->data.commandName[i], "-hid") == 0) isHid = true;
        if (strcmp(p->data.commandName[i], "-rec") == 0) isRec = true;
        else if (p->data.commandName[i][0] != '-'){
            strcpy(parameters[j], p->data.commandName[i]);
            j++;
        }
    }
    if (strcmp(parameters[0], "") == 0) {
        list_options(".", isLong, isDir, isHid, isRec);
    }
    for (int i = 0; strcmp(parameters[i], "") != 0; ++i) {
        list_options(parameters[i], isLong, isDir, isHid, isRec);
    }
    for (int i = 0; i < 100; ++i) {
        strcpy(parameters[i], "");
    }
}

void comandoCreate (tPosL p) { // Implementación del comando "create"
    struct stat st = {0};
    mode_t m = 0755;
    FILE * f;
    if ((strcmp(p->data.commandName[1], "") == 0 && strcmp(p->data.commandName[2], "") == 0) || ((strcmp(p->data.commandName[1], "-dir") == 0 && strcmp(p->data.commandName[2], "") == 0))) {
        strcpy(p->data.commandName[0], "list");
        comandoList(p);
    }
    if (strcmp(p->data.commandName[1], "-dir") == 0 && strcmp(p->data.commandName[2], "") != 0) { // Creamos un directorio
        if (stat(p->data.commandName[2], &st) == -1) {
            if (mkdir(p->data.commandName[2], m) == -1) perror("mkdir: ");
        }
    }
    if (strcmp(p->data.commandName[1], "") != 0 && strcmp(p->data.commandName[2], "") == 0) { // Creamos un fichero
        f = fopen(p->data.commandName[1], "w+");
        free(f);
    }
    strcpy(p->data.commandName[0], "create");
}

void comandoDelete(tPosL p) { // Implementación del comando "delete"
    int del;
    struct stat sbuf;
    if (strcmp(p->data.commandName[1], "") == 0 || (strcmp(p->data.commandName[1], "-rec") == 0 && strcmp(p->data.commandName[2], "") == 0)) {
        strcpy(p->data.commandName[0], "list");
        strcpy(p->data.commandName[1], "");
        comandoList(p);
    }
    if (strcmp(p->data.commandName[1], "") != 0) {
        if (strcmp(p->data.commandName[1], "-rec") != 0) {
            for (int i = 1; strcmp(p->data.commandName[i], "") != 0; ++i) {
                if (stat(p->data.commandName[i], &sbuf) == 0) {
                    if (S_ISDIR(sbuf.st_mode)) {
                        remove_dir(p->data.commandName[i], false);
                    }
                    else {
                        del = remove(p->data.commandName[i]);
                        if (del == -1) perror("delete was not satisfactory: ");
                    }
                }
                else perror("stat: ");

            }
        }
        else if (strcmp(p->data.commandName[1], "-rec") == 0) {
            for (int i = 2; strcmp(p->data.commandName[i], "") != 0; ++i) {
                if (stat(p->data.commandName[i], &sbuf) == 0) {
                    if (S_ISDIR(sbuf.st_mode)) {
                        remove_dir(p->data.commandName[i], true);
                    }
                    else {
                        del = remove(p->data.commandName[i]);
                        if (del == -1) perror("delete was not satisfactory: ");
                    }
                }
            }
        }
    }
    strcpy(p->data.commandName[0], "delete");
}

void comandoMemory(tPosL p, tListM * list_nodes) {
    // -allocate
    // -dealloc
    // -deletekey cl
    // -show
    // -show-vars
    // -show-funcs
    // -pmap

    char temp[100][LIM];
    void * ptr;
    time_t t;
    struct tm * timeInfo;
    char dateString[250];
    tItemM item;
    tItemM iterator;
    tPosM pos;

    if (strcmp(p->data.commandName[1], "-allocate") == 0) { // Mostrar listado de todas las asignaciones si no se da [2]
        if (strcmp(p->data.commandName[2], "-malloc") == 0) { // Memoria dinámica malloc
            if (strcmp(p->data.commandName[3], "") != 0) {
                ptr = malloc(castToInt(p->data.commandName[3]) * sizeof(void*));
                if (ptr == NULL)
                    printf("Allocation failed\n");
                else {
                    //memset(ptr, 0, sizeof(ptr[0]));
                    item.ptr = ptr;
                    time(&t);
                    timeInfo = localtime(&t);
                    item.time = *timeInfo;
                    item.bloques = castToInt(p->data.commandName[3]);
                    item.tipoNodo = 'm';
                    insertItemM(item, NULL, list_nodes);
                    printf("Asignados %d bytes en %p\n", castToInt(p->data.commandName[3]), ptr);
                }
            }
            else { // Mostramos la lista de asignaciones malloc
                if (!isEmptyListM(*list_nodes)) {
                    pos = *list_nodes;
                    while (pos != NULL) {
                        iterator = pos->data;
                        if (iterator.tipoNodo == 'm') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:     size:%d malloc %s\n", iterator.ptr, iterator.bloques, dateString);
                        }
                        pos = pos->next;
                    }
                }
                else {
                    printf("Empty list\n");
                }
            }
        }
        else if (strcmp(p->data.commandName[2], "-mmap") == 0) {
            char * argument1[LIM];
            if (strcmp(p->data.commandName[3], "") != 0) {
                *argument1 = p->data.commandName[3];
                argument1[1] = p->data.commandName[4];
            }
            Cmd_AllocateMmap(argument1, list_nodes);
            *argument1 = NULL;
            argument1[1] = NULL;
        }
        else if (strcmp(p->data.commandName[2], "-createshared") == 0) {
            char * arguments2[LIM];
            if (strcmp(p->data.commandName[3], "") != 0) {
                *arguments2 = p->data.commandName[3];
                arguments2[1] = p->data.commandName[4];
            }
            Cmd_AlocateCreateShared(arguments2, list_nodes);
            *arguments2 = NULL;
            arguments2[1] = NULL;
        }
        else if (strcmp(p->data.commandName[2], "-shared") == 0) {
            char * arguments3[LIM];
            if (strcmp(p->data.commandName[3], "") != 0) {
                *arguments3 = p->data.commandName[3];
                arguments3[1] = p->data.commandName[4];
            }
            Cmd_AlocateCreateShared(arguments3, list_nodes);
            *arguments3 = NULL;
            arguments3[1] = NULL;
        }
        else { // Mostramos la lista de todas
            if (!isEmptyListM(*list_nodes)) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    if (iterator.tipoNodo == 'S') {
                        strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                        printf("%p:   size:%d. shared memory (key %d) %s\n", iterator.ptr, (int)iterator.size, iterator.key, dateString);
                    }
                    else if (iterator.tipoNodo == 'm') {
                        strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                        printf("%p:     size:%d malloc %s\n", iterator.ptr, iterator.bloques, dateString);
                    }
                    else if (iterator.tipoNodo == 'M') {
                        strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                        printf("%p:   size:%d. mmap %s (fd:%d) %s\n", iterator.ptr, (int)iterator.size, iterator.fich, iterator.df, dateString);
                    }
                    pos = pos->next;
                }
            }
        }
    }
    else if (strcmp(p->data.commandName[1], "-dealloc") == 0) {
        if (strcmp(p->data.commandName[2], "-malloc") == 0) {
            dealloc('m', list_nodes, p);
        }
        else if (strcmp(p->data.commandName[2], "-mmap") == 0) {
            dealloc('M', list_nodes, p);
        }
        else if (strcmp(p->data.commandName[2], "-shared") == 0) {
            dealloc('S', list_nodes, p);
        }
        else { // Borrado dada la dirección
            if (!isEmptyListM(*list_nodes) && strcmp(p->data.commandName[2], "") != 0) {
                pos = *list_nodes;
                while (pos != NULL) {
                    iterator = pos->data;
                    char aux[LIM];
                    sprintf(aux, "%p", iterator.ptr);
                    if (strcmp(p->data.commandName[2], aux) == 0) {
                        if (iterator.tipoNodo == 'm') {
                            free(iterator.ptr);
                            deleteAtPositionM(pos, list_nodes);
                            printf("Deallocation of %p was successful (malloc)\n", iterator.ptr);
                        }
                        else if (iterator.tipoNodo == 'M') {
                            munmap(iterator.ptr, iterator.size);
                            deleteAtPositionM(pos, list_nodes);
                            printf("Unmap successful of %p (mmap)\n", iterator.ptr);
                        }
                        else if (iterator.tipoNodo == 'S') {
                            shmdt(iterator.ptr);
                            printf("Dealloc of %p was successful (shared)\n", iterator.ptr);
                            deleteAtPositionM(pos, list_nodes);
                        }
                        break;
                    }
                    pos = pos->next;
                }
                if (pos == NULL) { // Llegamos al final sin encontrar nada
                    if (!isEmptyListM(*list_nodes)) {
                        pos = *list_nodes;
                        while (pos != NULL) {
                            iterator = pos->data;
                            if (iterator.tipoNodo == 'S') {
                                strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                                printf("%p:   size:%d. shared memory (key %d) %s\n", iterator.ptr, (int)iterator.size, iterator.key, dateString);
                            }
                            else if (iterator.tipoNodo == 'm') {
                                strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                                printf("%p:     size:%d malloc %s\n", iterator.ptr, iterator.bloques, dateString);
                            }
                            else if (iterator.tipoNodo == 'M') {
                                strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                                printf("%p:   size:%d. mmap %s (fd:%d) %s\n", iterator.ptr, (int)iterator.size, iterator.fich, iterator.df, dateString);
                            }
                            pos = pos->next;
                        }
                    }
                }
            }
            else {
                if (!isEmptyListM(*list_nodes)) {
                    pos = *list_nodes;
                    while (pos != NULL) {
                        iterator = pos->data;
                        if (iterator.tipoNodo == 'S') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:   size:%d. shared memory (key %d) %s\n", iterator.ptr, (int)iterator.size, iterator.key, dateString);
                        }
                        else if (iterator.tipoNodo == 'm') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:     size:%d malloc %s\n", iterator.ptr, iterator.bloques, dateString);
                        }
                        else if (iterator.tipoNodo == 'M') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:   size:%d. mmap %s (fd:%d) %s\n", iterator.ptr, (int)iterator.size, iterator.fich, iterator.df, dateString);
                        }
                        pos = pos->next;
                    }
                }
                else {
                    printf("Empty list\n");
                }
            }
        }
    }
    else if (strcmp(p->data.commandName[1], "-show") == 0) {
        if (strcmp(p->data.commandName[2], "") == 0) {
            printf("Variables locales    %p,  %p,  %p\n", &temp, &t, &dateString);
            printf("Variables globales   %p,  %p,  %p\n", &recCount, &global1, &global2);
            printf("Funciones programa   %p,  %p,  %p\n", comandoDate, comandoTime, castToInt);
        }
        else {
            if (strcmp(p->data.commandName[2], "-malloc") == 0) {
                if (!isEmptyListM(*list_nodes)) {
                    pos = *list_nodes;
                    while (pos != NULL) {
                        iterator = pos->data;
                        if (iterator.tipoNodo == 'm') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:     size:%d malloc %s\n", iterator.ptr, iterator.bloques, dateString);
                        }
                        pos = pos->next;
                    }
                }
                else {
                    printf("Empty list\n");
                }
            }
            else if (strcmp(p->data.commandName[2], "-mmap") == 0) {
                if (!isEmptyListM(*list_nodes)) {
                    pos = *list_nodes;
                    while (pos != NULL) {
                        iterator = pos->data;
                        if (iterator.tipoNodo == 'M') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:   size:%d. mmap %s (fd:%d) %s\n", iterator.ptr, (int)iterator.size, iterator.fich, iterator.df, dateString);
                        }
                        pos = pos->next;
                    }
                }
                else {
                    printf("Empty list\n");
                }
            }
            else if (strcmp(p->data.commandName[2], "-shared") == 0) {
                if (!isEmptyListM(*list_nodes)) {
                    pos = *list_nodes;
                    while (pos != NULL) {
                        iterator = pos->data;
                        if (iterator.tipoNodo == 'S') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:   size:%d. shared memory (key %d) %s\n", iterator.ptr, (int)iterator.size, iterator.key, dateString);
                        }
                        pos = pos->next;
                    }
                }
                else {
                    printf("Empty list\n");
                }
            }
            else if (strcmp(p->data.commandName[2], "-all") == 0) {
                if (!isEmptyListM(*list_nodes)) {
                    pos = *list_nodes;
                    while (pos != NULL) {
                        iterator = pos->data;
                        if (iterator.tipoNodo == 'S') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:   size:%d. shared memory (key %d) %s\n", iterator.ptr, (int)iterator.size, iterator.key, dateString);
                        }
                        else if (iterator.tipoNodo == 'm') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:     size:%d malloc %s\n", iterator.ptr, iterator.bloques, dateString);
                        }
                        else if (iterator.tipoNodo == 'M') {
                            strftime(dateString, sizeof(dateString), "%c", &iterator.time);
                            printf("%p:   size:%d. mmap %s (fd:%d) %s\n", iterator.ptr, (int)iterator.size, iterator.fich, iterator.df, dateString);
                        }
                        pos = pos->next;
                    }
                }
                else {
                    printf("Empty list\n");
                }
            }
        }
    }
    else if (strcmp(p->data.commandName[1], "-show-vars") == 0) {
        printf("Variables locales    %p,  %p,  %p\n", &temp, &t, &dateString);
        printf("Variables globales   %p,  %p,  %p\n", &recCount, &global1, &global2);
    }
    else if (strcmp(p->data.commandName[1], "-show-funcs") == 0) {
        printf("Funciones programa   %p,  %p,  %p\n", comandoDate, comandoTime, castToInt);
        printf("Funciones librería   %p,  %p,  %p\n", strcmp, perror, stat);
    }
    else if (strcmp(p->data.commandName[1], "-dopmap") == 0) {
        Cmd_dopmap(NULL);
    }
    else if (strcmp(p->data.commandName[1], "-deletekey") == 0) {
        if (strcmp(p->data.commandName[2], "") == 0) {
            printf("   deletekey necesita una clave_valida\n");
        }
        else {
            char * clave[LIM];
            *clave = p->data.commandName[2];
            Cmd_deletekey(clave);
        }
    }
}

void comandoReadfile(tPosL p) {
    void * ptr = NULL;
    unsigned long ul;
    if (strcmp(p->data.commandName[1], "") != 0 && strcmp(p->data.commandName[2], "") != 0) {
        sscanf(p->data.commandName[2], "%lx", &ul);
        ptr = (void*) ul;

        if (strcmp(p->data.commandName[3], "") == 0) {
            leerFichero(p->data.commandName[1], ptr, -1);
        }
        else {
            leerFichero(p->data.commandName[1], ptr, castToInt(p->data.commandName[3]));
        }
    }
    else printf("readfile fich addr [cont]\n");
    leerFichero(p->data.commandName[1], p->data.commandName[2], -1);
}

void comandoWriteFile(tPosL p) {
    void * ptr;
    unsigned long ul;
    int fd;
    ssize_t out;

    if (strcmp(p->data.commandName[1], "") == 0 || strcmp(p->data.commandName[2], "") == 0 || strcmp(p->data.commandName[3], "") == 0) {
        printf("writefile [-o] fich addr cont\n");
    }
    else {
        if (strcmp(p->data.commandName[1], "-o") == 0) { // Sobrescibimos el posible fichero ya creado
            fd = open(p->data.commandName[2], O_RDWR);
            sscanf(p->data.commandName[3], "%lx", &ul);
            ptr = (void*) ul;
            out = write(fd, ptr, castToInt(p->data.commandName[4]));

            if (out == -1)
                perror("Imposible escribir fichero: ");
            else
                printf("Se han escrito %d bytes en %s desde %p\n", (int)out, p->data.commandName[2], ptr);
        }
        else { // No hay sobrescritura, se sobreentiende que se debe crear un nuevo fichero con el nombre dado
            fd = open(p->data.commandName[1], O_RDWR | O_CREAT | O_TRUNC, 0644);
            sscanf(p->data.commandName[2], "%lx", &ul);
            ptr = (void*) ul;
            out = write(fd, ptr, castToInt(p->data.commandName[3]));

            if (out == -1)
                perror("Imposible escribir fichero: ");
            else
                printf("Se han escrito %d bytes en %s desde %p\n", (int)out, p->data.commandName[1], ptr);
        }
        close(fd);
    }
}

void comandoMemdump(tPosL p) {
    int i;
    int len;
    void * ptr = NULL;
    unsigned char buff[17];
    unsigned char * pc;
    unsigned long ul;

    if (strcmp(p->data.commandName[1], "") == 0) printf("pocos argumentos recibidos\n");
    else {
        if (strcmp(p->data.commandName[2], "") == 0) {
            len = 25; // Miramos solo los primeros 25 bytes de la dirección
        }
        else len = castToInt(p->data.commandName[2]);

        sscanf(p->data.commandName[1], "%lx", &ul);
        ptr = (void*) ul;
        pc = (unsigned char*)ptr;

        for (i = 0; i < len; ++i) {
            if ((i % 16) == 0) {
                if (i != 0)
                    printf("   %s\n", buff);
                printf("   %04x ", i);
            }

            printf(" %02x", pc[i]);

            if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
                buff[i % 16] = '.';
            }
            else {
                buff[i % 16] = pc[i];
            }

            buff[(i % 16) + 1] = '\0';
        }

        while ((i % 16) != 0) {
            printf("   ");
            i++;
        }
        printf("   %s\n", buff);
    }
}

void comandoMemfill(tPosL p) {
    void * ptr = NULL;
    unsigned long ul;
    int aux;

    if (strcmp(p->data.commandName[1], "") == 0) printf("//-> memfill addr [cont] [byte]\n");
    else {
        sscanf(p->data.commandName[1], "%lx", &ul);
        ptr = (void*) ul;
        if (strcmp(p->data.commandName[2], "") == 0) {
            memset(ptr, 0x41, 128*sizeof(char));
        }
        else {
            if (strcmp(p->data.commandName[3], "") == 0) {
                memset(ptr, 0x41, (castToInt(p->data.commandName[2])) * sizeof(char));
            }
            else {
                if (p->data.commandName[3][1] == 'x') {
                    sscanf(p->data.commandName[3], "%x", &aux);
                    memset(ptr, aux, (castToInt(p->data.commandName[2])) * sizeof(char));
                }
                else {
                    memset(ptr, castToInt(p->data.commandName[3]), (castToInt(p->data.commandName[2])) * sizeof(char));
                }
            }
        }
    }
}

void doRecursiva(int n) {
    char automatico[BUFFSIZE];
    static char estatico[BUFFSIZE];

    printf("Parámetro n: %d en %p || Array automático en %p || Array estático en %p\n", n, &n, automatico, estatico);

    n--;
    if (n > 0)
        doRecursiva(n);
}

void comandoRecurse(tPosL p) {
    if (strcmp(p->data.commandName[1], "") == 0) printf("//-> recurse n\n");
    else {
        if (strcmp(p->data.commandName[2], "") != 0) printf("//-> recurse n");
        else {
            doRecursiva(castToInt(p->data.commandName[1]));
        }
    }
}

void comandoGetpriority(tPosL p) {
    int ret = 0;
    id_t pid;

    if (strcmp(p->data.commandName[1], "") == 0)
        pid = getpid();
    else
        pid = castToInt(p->data.commandName[1]);

    ret = getpriority(PRIO_PROCESS, pid);
    if (ret == -1) {
        perror("Imposible getpriority: ");
        return;
    }
    else
        printf("Prioridad del proceso %d es %d\n", pid, ret);
}

void comandoSetpriority(tPosL p) {
    int ret = 0;
    id_t pid;
    pid = getpid();

    if (strcmp(p->data.commandName[1], "") == 0) {
        ret = getpriority(PRIO_PROCESS, pid);
        if (ret == -1) {
            perror("Imposible getpriority: ");
            return;
        }
        else
            printf("Prioridad del proceso %d es %d\n", pid, ret);
    }
    else {
        if (strcmp(p->data.commandName[2], "") == 0) {
            ret = setpriority(PRIO_PROCESS, pid, castToInt(p->data.commandName[1]));
            if (ret == -1) {
                perror("Imposible setpriority: ");
                return;
            }
            else {
                printf("Cambio de prioridad del proceso %d ---> Nueva Prioridad = %d\n", pid, castToInt(p->data.commandName[1]));
            }
        }
        else {
            ret = setpriority(PRIO_PROCESS, castToInt(p->data.commandName[1]), castToInt(p->data.commandName[2]));
            if (ret == -1) {
                perror("Imposible setpriority: ");
                return;
            }
            else {
                printf("Cambio de prioridad del proceso %d ---> Nueva Prioridad = %d\n", castToInt(p->data.commandName[1]), castToInt(p->data.commandName[2]));
            }
        }
    }
}

void comandoGetuid() {
    char * pr[LIM];
    Cmd_getuid(pr);
}

void comandoSetuid(tPosL p) {
    char * tr[1024];
    if (strcmp(p->data.commandName[1], "") == 0) {
        Cmd_setuid(tr);
    }
    else {
        *tr = p->data.commandName[1];
        if (strcmp(p->data.commandName[2], "") != 0)
            tr[1] = p->data.commandName[2];
        Cmd_setuid(tr);
    }
}

void comandoFork(tPosL p) {
    pid_t pid;
    if ((pid = fork()) == 0) {
        exit(0);
    }
    printf("ejecutando el proceso %d\n", pid);
}

void comandoExecute(tPosL p, bool pri) {
    char * temp[1024];
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
    char * aux[100];
    for (int i = 0; aux[i] != NULL; ++i) {
        aux[i] = NULL;
    }
    pid_t pid;
    int ret;
    for (int i = 1; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (pri && p->data.commandName[i][0] == '@')
            *aux = strtok(p->data.commandName[i], "@");
        else
            temp[i - 1] = p->data.commandName[i];
    }
    if (pri) {
        pid = getpid();
        ret = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
        if (ret == -1)
            perror("Error en el setpriority: ");
        else
            printf("Cambio de prioridad de %d --> Nueva prioridad = %d\n", pid, castToInt(*aux));
    }
    if (execvp(temp[0], temp) == -1) {
        perror("cannot execute: ");
    }
    exit(255);
}

void comandoForeground(tPosL p, bool pri) {
    char * temp[1024];
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
    char * aux[100];
    for (int i = 0; aux[i] != NULL; ++i) {
        aux[i] = NULL;
    }
    pid_t pid;
    int ret;
    for (int i = 1; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (pri && p->data.commandName[i][0] == '@')
            *aux = strtok(p->data.commandName[i], "@");
        else
            temp[i - 1] = p->data.commandName[i];
    }
    pid = fork();
    if (pid == 0) {
        if (pri) {
            ret = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
            if (ret == -1)
                perror("Error en el setpriority: ");
            else
                printf("Cambio de prioridad (%d)\n", castToInt(*aux));
        }
        if (execvp(temp[0], temp) == -1) {
            perror("cannot execute: ");
        }
        exit(255);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
    }
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
}

void comandoBackground(tPosL p, bool pri, tListB * list) {
    char * temp[1024];
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
    char * aux[100];
    for (int i = 0; aux[i] != NULL; ++i) {
        aux[i] = NULL;
    }
    bool save = true;
    char * estado = "ACTIVO";
    pid_t pid;
    int ret = 0;
    int status;
    tItemB item;
    time_t t;
    struct tm * timeInfo;
    time(&t);
    timeInfo = localtime(&t);
    item.time = *timeInfo;

    for (int i = 1; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (pri && p->data.commandName[i][0] == '@')
            *aux = strtok(p->data.commandName[i], "@");
        else
            temp[i - 1] = p->data.commandName[i];
    }

    if ((pid = fork()) == 0) {
        if (pri) {
            ret = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
            if (ret == -1)
                perror("Error en el setpriority: ");
            else {
                printf("Cambio de prioridad (%d)\n", getpriority(PRIO_PROCESS, pid));
            }
        }
        if (execvp(temp[0], temp) == -1) {
            perror("cannot execute: ");
        }
        exit(255);
    }

    if (waitpid(pid, &status, WNOHANG | WUNTRACED | WCONTINUED) == pid) {
        if (WIFEXITED(status)) {
            estado = "TERMINADO";
            item.priority = -1;
            item.terminated = WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status)) {
            estado = "SEÑALADO";
            item.priority = -1;
            item.terminated = WTERMSIG(status);
        }
        else if (WIFSTOPPED(status)) {
            estado = "PARADO";
            item.terminated = WSTOPSIG(status);
        }
        else if (WIFCONTINUED(status)) {
            estado = "CONTINUADO";
            item.terminated = 0;
        }
    }
    strcpy(item.state, estado);
    strcpy(item.commandLine, *temp);
    if (pri)
        item.priority = castToInt(*aux);
    else
        item.priority = getpriority(PRIO_PROCESS, pid);
    item.pid = pid;
    if (save)
        insertItemB(item, BNULL, list);

    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
}

void updateProcessList(tListB *lista) {
    int state = 0;
    pid_t pid;
    tPosB pos;
    tItemB item; // Solo lectura

    if (!isEmptyListB(*lista)) {
        pos = 0;
        while (pos != BNULL) {
            item = lista->data[pos];
            pid = item.pid;
            lista->data[pos].priority = getpriority(PRIO_PROCESS, pid);
            if (waitpid(pid, &state, WNOHANG | WUNTRACED | WCONTINUED) == pid) {
                if (WIFCONTINUED(state)) {
                    strcpy(lista->data[pos].state, "CONTINUADO");
                    lista->data[pos].terminated = 000;
                }
                else if (WIFSTOPPED(state)) {
                    strcpy(lista->data[pos].state, "PARADO");
                    lista->data[pos].terminated = WSTOPSIG(state);
                }
                else if (WIFSIGNALED(state)) {
                    strcpy(lista->data[pos].state, "SEÑALADO");
                    lista->data[pos].priority = -1;
                    lista->data[pos].terminated = WTERMSIG(state);
                }
                else if (WIFEXITED(state)) {
                    strcpy(lista->data[pos].state, "TERMINADO");
                    lista->data[pos].priority = -1;
                    lista->data[pos].terminated = WEXITSTATUS(state);
                }
            }
            if (pos == lista->lastPos)
                pos = BNULL;
            else
                pos++;
        }
    }
}

void comandoListprocs(tListB * list) {
    tPosB pos;
    tItemB item;
    char dateString[250];

    if (!isEmptyListB(*list)) {
        pos = 0;
        updateProcessList(list);
        while (pos != BNULL) {
            item = list->data[pos];
            strftime(dateString, sizeof(dateString), "%c", &item.time);
            if (item.terminated == 0 || strcmp(item.state, "SEÑALADO") != 0)
                printf("%d p = %d %s %s (%d) %s\n", item.pid, item.priority, dateString, item.state, item.terminated, item.commandLine);
            else
                printf("%d p = %d %s %s (%s) %s\n", item.pid, item.priority, dateString, item.state, nombreSenal(item.terminated), item.commandLine);
            if (pos == list->lastPos)
                pos = BNULL;
            else
                pos++;
        }
    }
    else
        printf("Process list is empty\n");
}

void lanzarProg(tPosL p, tListB * list) {
    bool back = false;
    bool pri = false;
    id_t pid;
    char * temp[150];
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
    char * aux[100];
    for (int i = 0; aux[i] != NULL; ++i) {
        aux[i] = NULL;
    }
    int set;
    tItemB item;
    time_t t;
    struct tm * timeInfo;
    time(&t);
    timeInfo = localtime(&t);
    item.time = *timeInfo;
    int status;
    char * estado = "ACTIVO";

    for (int i = 0; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (strcmp(p->data.commandName[i], "&") == 0)
            back = true;
    }
    if (back) { // Background
        for (int i = 0; strcmp(p->data.commandName[i], "") != 0; ++i) {
            if (p->data.commandName[i][0] == '@') {
                *aux = strtok(p->data.commandName[i], "@");
                pri = true;
            }
            else if (strcmp(p->data.commandName[i], "&") != 0)
                temp[i] = p->data.commandName[i];
        }

        if ((pid = fork()) == 0) {
            if (pri) {
                set = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
                if (set == -1)
                    perror("Error en el setpriority: ");
                else
                    printf("Cambio de prioridad (%d)\n", getpriority(PRIO_PROCESS, pid));
            }
            if (execvp(temp[0], temp) == -1) {
                perror("cannot execute: ");
            }
            exit(255);
        }
        if (waitpid(pid, &status, WNOHANG | WUNTRACED | WCONTINUED) == pid) {
            if (WIFEXITED(status)) {
                estado = "TERMINADO";
                item.priority = -1;
                item.terminated = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status)) {
                estado = "SEÑALADO";
                item.priority = -1;
                item.terminated = WTERMSIG(status);
            }
            else if (WIFSTOPPED(status)) {
                estado = "PARADO";
                item.terminated = WSTOPSIG(status);
            }
            else if (WIFCONTINUED(status)) {
                estado = "CONTINUADO";
                item.terminated = 0;
            }
        }
        strcpy(item.state, estado);
        strcpy(item.commandLine, *temp);
        if (pri)
            item.priority = castToInt(*aux);
        else
            item.priority = getpriority(PRIO_PROCESS, pid);
        item.pid = pid;
        insertItemB(item, BNULL, list);
    }
    else { // Foreground
        for (int i = 0; strcmp(p->data.commandName[i], "") != 0; ++i) {
            if (p->data.commandName[i][0] == '@') {
                *aux = strtok(p->data.commandName[i], "@");
                pri = true;
            }
            else
                temp[i] = p->data.commandName[i];
        }
        pid = fork();
        if (pid < 0)
            return ;
        else if (pid == 0) {
            if (pri) {
                set = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
                if (set == -1)
                    perror("Error en el setpriority: ");
                else
                    printf("Cambio de prioridad de %d --> Nueva prioridad = %d\n", pid, castToInt(*aux));
            }
            int ret = execvp(temp[0], temp);
            if (ret == -1) {
                printf("Comando no encontrado\n");
                _exit(EXIT_FAILURE);
            }
            _exit(EXIT_SUCCESS);
        }
        else {
            int fg_status;
            waitpid(pid, &fg_status, 0);
        }
    }
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
}

void comandoRun_as(tPosL p, tListB * list) {
    // Control de prioridad y del tipo de ejecución
    bool back;
    bool pri;
    id_t pid;
    char * temp[150];
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
    char * aux[100];
    for (int i = 0; aux[i] != NULL; ++i) {
        aux[i] = NULL;
    }
    char * login[100];
    char * estado = "ACTIVO";
    int set;
    int status;
    tItemB item;
    time_t t;
    struct tm * timeInfo;
    time(&t);
    timeInfo = localtime(&t);
    item.time = *timeInfo;

    for (int i = 0; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (strcmp(p->data.commandName[i], "&") == 0)
            back = true;
        else if (p->data.commandName[i][0] == '@') {
            pri = true;
            *aux = strtok(p->data.commandName[i], "@");
        }
    }
    *login = "-l";
    login[1] = p->data.commandName[1];
    for (int i = 2; strcmp(p->data.commandName[i], "") != 0; ++i) {
        temp[i - 2] = p->data.commandName[i];
    }

    if (back) { // Background
        if ((pid = fork()) == 0) {
            if (Cmd_setuid(login) != -1) {
                if (pri) {
                    set = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
                    if (set == -1)
                        perror("Error en el setpriority: ");
                    else
                        printf("Cambio de prioridad (%d)\n", castToInt(*aux));
                }
                if (execvp(temp[0], temp) == -1) {
                    perror("cannot execute: ");
                }
                exit(255);
            }
            else
                printf("No ejecutado: Ejecutable debe ser setuid (rwsr-xr-x)");
        }
        if (waitpid(pid, &status, WNOHANG | WUNTRACED | WCONTINUED) == pid) {
            if (WIFEXITED(status)) {
                estado = "TERMINADO";
                item.priority = -1;
                item.terminated = WEXITSTATUS(status);
            }
            else if (WIFSIGNALED(status)) {
                estado = "SEÑALADO";
                item.priority = -1;
                item.terminated = WTERMSIG(status);
            }
            else if (WIFSTOPPED(status)) {
                estado = "PARADO";
                item.terminated = WSTOPSIG(status);
            }
            else if (WIFCONTINUED(status)) {
                estado = "CONTINUADO";
                item.terminated = 0;
            }
        }
        strcpy(item.state, estado);
        strcpy(item.commandLine, *temp);
        if (pri)
            item.priority = castToInt(*aux);
        else
            item.priority = getpriority(PRIO_PROCESS, pid);
        item.pid = pid;
        insertItemB(item, BNULL, list);
    }
    else { // Foreground
        pid = fork();
        if (pid == 0) {
            if (Cmd_setuid(login) != -1) {
                if (pri) {
                    set = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
                    if (set == -1)
                        perror("Error en el setpriority: ");
                    else
                        printf("Cambio de prioridad de %d ---> Nueva prioridad = %d\n", pid, castToInt(*aux));
                }
                execvp(temp[0], temp);
                _exit(EXIT_SUCCESS);
            }
            else
                printf("No ejecutado: Ejecutable debe ser setuid (rwsr-xr-x)\n");
        }
        else {
            int fg_status;
            waitpid(pid, &fg_status, 0);
        }
    }
    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
}

void comandoExexute_as(tPosL p) {
    bool pri;
    id_t pid;
    char * temp[150];
    char * aux[100];
    char * login[100];
    int set;

    for (int i = 0; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (p->data.commandName[i][0] == '@') {
            pri = true;
            *aux = strtok(p->data.commandName[i], "@");
        }
    }
    *login = "-l";
    login[1] = p->data.commandName[1];
    for (int i = 2; strcmp(p->data.commandName[i], "") != 0; ++i) {
        temp[i - 2] = p->data.commandName[i];
    }
    if (Cmd_setuid(login) != -1) {
        if (pri) {
            pid = getpid();
            set = setpriority(PRIO_PROCESS, pid, castToInt(*aux));
            if (set == -1)
                perror("Error en el setpriority: ");
            else
                printf("Cambio de prioridad de %d ---> Nueva prioridad = %d\n", pid, castToInt(*aux));
        }
        execvp(temp[0], temp);
    }

    for (int i = 0; temp[i] != NULL; ++i) {
        temp[i] = NULL;
    }
}

void comandoProc(tPosL p, tListB * list) {
    tPosB pos;
    tItemB item;
    bool fg = false;
    pid_t pid;
    char dateString[100];
    int status;
    bool encontrado = false;

    for (int i = 0; strcmp(p->data.commandName[i], "") != 0; ++i) {
        if (strcmp(p->data.commandName[i], "-fg") == 0)
            fg = true;
        else
            pid = atoi(p->data.commandName[i]);
    }

    if (!isEmptyListB(*list)) {
        pos = 0;
        while (pos != BNULL) {
            item = list->data[pos];
            if (item.pid == pid) {
                encontrado = true;
                if (fg) { // Traemos el proceso a primer plano
                    waitpid(item.pid, &status, WUNTRACED | WCONTINUED);
                    if (WIFEXITED(status)) {
                        printf("Exited, status=%d\n", WEXITSTATUS(status));
                    } else if (WIFSIGNALED(status)) {
                        printf("Killed by signal %d -> %s\n", WTERMSIG(status), nombreSenal(status));
                    } else if (WIFSTOPPED(status)) {
                        printf("Stopped by signal %d\n", WSTOPSIG(status));
                    } else if (WIFCONTINUED(status)) {
                        printf("Continued\n");
                    }
                    deleteAtPositionB(pos, list);
                    pos = list->lastPos;
                }
                else { // Mostramamos la información del proceso con el pid asociado en nuestra lista
                    strftime(dateString, sizeof(dateString), "%c", &item.time);
                    printf("%d p = %d %s %s (%d) %s\n", item.pid, item.priority, dateString, item.state, item.terminated, item.commandLine);
                    pos = list->lastPos;
                }
            }
            if (pos == list->lastPos)
                pos = BNULL;
            else
                pos++;
        }
        if (!encontrado)
            comandoListprocs(list);
    }
    else
        printf("Background list is empty\n");
}

void comandoDeleteTerm(tListB * lista) {
    tPosB pos;
    if (!isEmptyListB(*lista)) {
        pos = 0;
        while (pos != BNULL) {
            if (strcmp(lista->data[pos].state, "TERMINADO") == 0) {
                deleteAtPositionB(pos, lista);
                if (lista->lastPos != BNULL)
                    pos = 0;
                else
                    pos = BNULL;
            }
            else {
                if (pos == lista->lastPos)
                    pos = BNULL;
                else
                    pos++;
            }
        }
        comandoListprocs(lista);
    }
    else
        printf("Background list is empty\n");
}

void comandoDeleteSig(tListB * lista) {
    tPosB pos;
    if (!isEmptyListB(*lista)) {
        pos = 0;
        while (pos != BNULL) {
            if (strcmp(lista->data[pos].state, "SEÑALADO") == 0) {
                deleteAtPositionB(pos, lista);
                if (lista->lastPos != BNULL)
                    pos = 0;
                else
                    pos = BNULL;
            }
            else {
                if (pos == lista->lastPos)
                    pos = BNULL;
                else
                    pos++;
            }
        }
        comandoListprocs(lista);
    }
    else
        printf("background list is empty\n");
}

void comandoHistoric(char stringA[], int control, tList * list, tListM * list_nodes, tListB * background_list) { // Implementacion del comando "historic", stringA representa al parametro extra del comando
    tPosL pos; // Variable para iterar la lista y obtener su data
    tItemL item; // Item de la lista
    char * troceado[LIM];
    char * enteroNegativo[LIM];
    char stringB[LIM];
    int i;
    int a; // Variable para convertir determinado parametro del comando
    int b; // Variable para convertir determinado parametro del comando
    bool encontrado = false; // Boolean para indicar si se ha encontrado el elemento en la lista

    if (control == 1) { // Imprimimos la lista a base de iteraciones
        if (!isEmptyList(*list)) {
            pos = *list;
            while (pos != NULL) {
                item = pos->data;
                comprobarComando(item, control);
                pos = pos->next;
            }
        }
    }
    else if (strcmp(stringA, "-r") != 0){ // Tenemos tres opciones ---> -rN, -N y -c
        strcpy(stringB, stringA);
        if (strcmp(stringA, "-c") == 0) {
            freeList(list);
        }
        else if (stringB[0] == '-') { // Me deja con dos opciones ---> -rN y -N
            if (stringB[1] == 'r'){ // Estamos en la opción de rN
                *troceado = strtok(stringA, "-");
                if (stringB[2] == '-') { // Si hay un elemento negativo
                    *enteroNegativo = strtok(stringB, "-r-");
                    printf("No hay elemento -%s en el historico\n", *enteroNegativo);
                }
                else { // Procesamiento del comando de nuevo
                    *troceado = strtok(*troceado, "r");
                    b = castToInt(*troceado);
                    pos = *list;
                    while (pos != NULL) {
                        item = pos->data;
                        if (item.indice == b) {
                            encontrado = true;
                            comprobarComando(item, control);
                            if (strcmp(item.commandName[0], "time") == 0) {
                                comandoTime();
                            }
                            else if (strcmp(item.commandName[0], "getpid") == 0) {
                                printf("Pid de shell: %d\n", getpid());
                            }
                            else if (strcmp(item.commandName[0], "getppid") == 0) {
                                printf("Pid del padre del shell: %d\n", getppid());
                            }
                            else if (strcmp(item.commandName[0], "date") == 0) {
                                comandoDate();
                            }
                            else if (strcmp(item.commandName[0], "pwd") == 0) {
                                comandoPwd(item.commandName[0]);
                            }
                            else if (strcmp(item.commandName[0], "chdir") == 0) {
                                comandoChdir(item.commandName[1], item.control);
                            }
                            else if (strcmp(item.commandName[0], "authors") == 0) {
                                comandoAuthors(item.commandName[1], item.control);
                            }
                            else if (strcmp(item.commandName[0], "historic") == 0) {
                                comandoHistoric(item.commandName[1], item.control, list, list_nodes, background_list);
                            }
                            else if (strcmp(item.commandName[0], "create") == 0) {
                                comandoCreate(pos);
                            }
                            else if (strcmp(item.commandName[0], "list") == 0) {
                                comandoList(pos);
                            }
                            else if (strcmp(item.commandName[0], "delete") == 0) {
                                comandoDelete(pos);
                            }
                            else if (strcmp(item.commandName[0], "memory") == 0) {
                                comandoMemory(pos, list_nodes);
                            }
                            else if (strcmp(item.commandName[0], "readfile") == 0) {
                                comandoReadfile(pos);
                            }
                            else if (strcmp(item.commandName[0], "memdump") == 0) {
                                comandoMemdump(pos);
                            }
                            else if (strcmp(item.commandName[0], "memfilll") == 0) {
                                comandoMemfill(pos);
                            }
                            else if (strcmp(item.commandName[0], "recurse") == 0) {
                                comandoRecurse(pos);
                            }
                            else if (strcmp(item.commandName[0], "writefile") == 0) {
                                comandoWriteFile(pos);
                            }
                            else if (strcmp(item.commandName[0], "getpriority") == 0) {
                                comandoGetpriority(pos);
                            }
                            else if (strcmp(item.commandName[0], "setpriority") == 0) {
                                comandoSetpriority(pos);
                            }
                            else if (strcmp(item.commandName[0], "getuid") == 0) {
                                comandoGetuid();
                            }
                            else if (strcmp(item.commandName[0], "setuid") == 0) {
                                comandoSetuid(pos);
                            }
                            else if (strcmp(item.commandName[0], "fork") == 0) {
                                comandoFork(pos);
                            }
                            else if (strcmp(item.commandName[0], "execute") == 0) {
                                for (i = 0; strcmp(item.commandName[i], "") != 0; ++i);
                                if (item.commandName[i - 1][0] == '@')
                                    comandoExecute(pos, 1);
                                else
                                    comandoExecute(pos, 0);
                            }
                            else if (strcmp(item.commandName[0], "foreground") == 0) {
                                for (i = 0; strcmp(item.commandName[i], "") != 0; ++i);
                                if (item.commandName[i - 1][0] == '@')
                                    comandoForeground(pos, 1);
                                else
                                    comandoForeground(pos, 0);
                            }
                            else if (strcmp(item.commandName[0], "background") == 0) {
                                for (i = 0; strcmp(item.commandName[i], "") != 0; ++i);
                                if (item.commandName[i - 1][0] == '@')
                                    comandoBackground(pos, 1, background_list);
                                else
                                    comandoBackground(pos, 0, background_list);
                            }
                            else if (strcmp(item.commandName[0], "listprocs") == 0) {
                                comandoListprocs(background_list);
                            }
                            else if (strcmp(item.commandName[0], "run-as") == 0) {
                                comandoRun_as(pos, background_list);
                            }
                            else if (strcmp(item.commandName[0], "execute-as") == 0) {
                                comandoExexute_as(pos);
                            }
                            else if (strcmp(item.commandName[0], "proc") == 0) {
                                comandoProc(pos, background_list);
                            }
                            else if (strcmp(item.commandName[0], "deleteprocs") == 0 && strcmp(item.commandName[1], "") == 0) {
                                comandoListprocs(background_list);
                            }
                            else if (strcmp(item.commandName[0], "deleteprocs") == 0 && strcmp(item.commandName[1], "-term") == 0) {
                                comandoDeleteTerm(background_list);
                            }
                            else if (strcmp(item.commandName[0], "deleteprocs") == 0 && strcmp(item.commandName[1], "-sig") == 0) {
                                comandoDeleteSig(background_list);
                            }
                            else {
                                lanzarProg(pos, background_list);
                            }
                        }
                        pos = pos->next;
                    }
                    if (encontrado == false) {
                        printf("No hay elemento %s en el historico\n", *troceado);
                    }
                }
            }
            else if (stringB[1] != '-') { // Estamos en la opción de N
                *troceado = strtok(stringA, "-");
                a = castToInt(*troceado);
                if (!isEmptyList(*list)) { // Imprimimos la lista hasta la posición donde el indice sea 'a'
                    pos = *list;
                    while (pos->data.indice != a && last(*list) != pos) {
                        item = pos->data;
                        comprobarComando(item, 1); // Aqui el control siempre sera 1 pues imprimimos el historic
                        pos = pos->next;
                    }
                }
            }
        }
    }
}

void procesarEntrada (tList * list, bool * salir, int indice, tListM * list_nodes, tListB * background_list) { // Procesamiento de la entrada
    tPosL pos = NULL; // Posiciones para iterar la lista
    char stringA[LIM]; // String provisional
    char stringB[LIM]; // String provisional
    char stringC[LIM];
    int i;

    if (!isEmptyList(*list)) { // Recorremos la lista
        pos = *list;
        while (pos != NULL && pos->data.indice != indice) {
            if (indice != pos->data.indice) {
                pos = pos->next;
            }
        }
    }

    if (pos == NULL) {
        printf("Error fatal, no se ha encontrado el elemento en la lista\n");
    }
    else { // Procesamiento del comando
        strcpy(stringA, pos->data.commandName[0]); // Para evitar problemas al acceder a los comandos pwd y chdir
        strcpy(stringB, pos->data.commandName[1]); // usamos dos strings provisionales
        strcpy(stringC, *pos->data.commandName);
        if (strcmp(pos->data.commandName[0], "time") == 0) {
            comandoTime();
        }
        else if (strcmp(pos->data.commandName[0], "getpid") == 0) {
            printf("Pid de shell: %d\n", getpid());
        }
        else if (strcmp(pos->data.commandName[0], "getppid") == 0) {
            printf("Pid del padre del shell: %d\n", getppid());
        }
        else if (strcmp(pos->data.commandName[0], "date") == 0) {
            comandoDate();
        }
        else if (strcmp(pos->data.commandName[0], "pwd") == 0) {
            comandoPwd(stringA);
        }
        else if (strcmp(pos->data.commandName[0], "chdir") == 0) {
            comandoChdir(stringB, pos->data.control);
        }
        else if (strcmp(pos->data.commandName[0], "authors") == 0) {
            comandoAuthors(pos->data.commandName[1], pos->data.control);
        }
        else if (strcmp(pos->data.commandName[0], "historic") == 0) {
            comandoHistoric(pos->data.commandName[1], pos->data.control, list, list_nodes, background_list);
        }
        else if (strcmp(pos->data.commandName[0], "exit") == 0 || strcmp(pos->data.commandName[0], "end") == 0 || strcmp(pos->data.commandName[0], "quit") == 0) { // Orden de salir del shell
            *salir = true;
        }
        else if (strcmp(pos->data.commandName[0], "create") == 0) {
            comandoCreate(pos);
        }
        else if (strcmp(pos->data.commandName[0], "list") == 0) {
            comandoList(pos);
        }
        else if (strcmp(pos->data.commandName[0], "delete") == 0) {
            comandoDelete(pos);
        }
        else if (strcmp(pos->data.commandName[0], "memory") == 0) {
            comandoMemory(pos, list_nodes);
        }
        else if (strcmp(pos->data.commandName[0], "readfile") == 0) {
            comandoReadfile(pos);
        }
        else if (strcmp(pos->data.commandName[0], "memdump") == 0) {
            comandoMemdump(pos);
        }
        else if (strcmp(pos->data.commandName[0], "memfill") == 0) {
            comandoMemfill(pos);
        }
        else if (strcmp(pos->data.commandName[0], "recurse") == 0) {
            comandoRecurse(pos);
        }
        else if (strcmp(pos->data.commandName[0], "writefile") == 0) {
            comandoWriteFile(pos);
        }
        else if (strcmp(pos->data.commandName[0], "getpriority") == 0) {
            comandoGetpriority(pos);
        }
        else if (strcmp(pos->data.commandName[0], "setpriority") == 0) {
            comandoSetpriority(pos);
        }
        else if (strcmp(pos->data.commandName[0], "getuid") == 0) {
            comandoGetuid();
        }
        else if (strcmp(pos->data.commandName[0], "setuid") == 0) {
            comandoSetuid(pos);
        }
        else if (strcmp(pos->data.commandName[0], "fork") == 0) {
            comandoFork(pos);
        }
        else if (strcmp(pos->data.commandName[0], "execute") == 0) {
            for (i = 0; strcmp(pos->data.commandName[i], "") != 0; ++i);
            if (pos->data.commandName[i - 1][0] == '@')
                comandoExecute(pos, 1);
            else
                comandoExecute(pos, 0);
        }
        else if (strcmp(pos->data.commandName[0], "foreground") == 0) {
            for (i = 0; strcmp(pos->data.commandName[i], "") != 0; ++i);
            if (pos->data.commandName[i - 1][0] == '@')
                comandoForeground(pos, 1);
            else
                comandoForeground(pos, 0);
        }
        else if (strcmp(pos->data.commandName[0], "background") == 0) {
            for (i = 0; strcmp(pos->data.commandName[i], "") != 0; ++i);
            if (pos->data.commandName[i - 1][0] == '@')
                comandoBackground(pos, 1, background_list);
            else
                comandoBackground(pos, 0, background_list);
        }
        else if (strcmp(pos->data.commandName[0], "listprocs") == 0) {
            comandoListprocs(background_list);
        }
        else if (strcmp(pos->data.commandName[0], "run-as") == 0) {
            comandoRun_as(pos, background_list);
        }
        else if (strcmp(pos->data.commandName[0], "execute-as") == 0) {
            comandoExexute_as(pos);
        }
        else if (strcmp(pos->data.commandName[0], "proc") == 0) {
            comandoProc(pos, background_list);
        }
        else if (strcmp(pos->data.commandName[0], "deleteprocs") == 0 && strcmp(pos->data.commandName[1], "") == 0) {
            comandoListprocs(background_list);
        }
        else if (strcmp(pos->data.commandName[0], "deleteprocs") == 0 && strcmp(pos->data.commandName[1], "-term") == 0) {
            comandoDeleteTerm(background_list);
        }
        else if (strcmp(pos->data.commandName[0], "deleteprocs") == 0 && strcmp(pos->data.commandName[1], "-sig") == 0) {
            comandoDeleteSig(background_list);
        }
        else {
            lanzarProg(pos, background_list);
        }
    }
}



int main() {

    bool terminado = false;
    int indice; // Indice del comando en la lista
    tList L; // Creacion de la lista
    createEmptyList(&L); // Inicializacion de la lista
    tListM list_nodes;
    createEmptyListM(&list_nodes);
    tListB background_list;
    createEmptyListB(&background_list);
    while (terminado == false) {  // Comienzo del shell

        leerEntrada(&L, &indice);
        procesarEntrada(&L, &terminado, indice, &list_nodes, &background_list);
    }

    freeList(&L); // Liberamos la lista de memoria tras terminar el proceso
    freeListM(&list_nodes); // Liberamos la lista de nodos de allocations de memoria
    deleteListB(&background_list);


    return 0;
}
