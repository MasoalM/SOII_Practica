#include "bloques.h"

static int descriptor = 0;

int bmount(const char *camino) {
    umask(000);
    descriptor = open(camino, O_CREAT | O_RDWR, 0666);
    if(descriptor == -1) {
        perror(RED "Error al abrir la ruta");
        return FALLO;
    }
    return descriptor;
}


int bumount() {
    int cierre = close(descriptor);
    if(cierre == -1) {
        perror(RED "Error al cerrar la ruta");
        return FALLO;
    }
    return EXITO;
}


int bwrite(unsigned int nbloque, const void *buf) {
    off_t desplazamiento = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    if(desplazamiento == -1) {
        perror(RED "Error al desplazar el puntero");
        return FALLO;
    }
    
    size_t bytes_escritos = write(descriptor, &buf, desplazamiento);
    if(bytes_escritos == -1) {
        perror(RED "Error al escribir un bloque");
        return FALLO;
    }
    return BLOCKSIZE;
}

int bread(unsigned int nbloque, void *buf) {
    off_t desplazamiento = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    if(desplazamiento == -1) {
        perror(RED "Error al desplazar el puntero");
        return FALLO;
    }

    size_t bytes_leidos = read(descriptor, &buf, desplazamiento);
    if(bytes_leidos == -1) {
        perror(RED "Error al leer un bloque");
        return FALLO;
    }
    return BLOCKSIZE;
}
