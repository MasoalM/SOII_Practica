#include "bloques.h"

// Variable global que almacena el descriptor de un dispositivo virtual
static int descriptor = 0;

// Nombre: bmount
// Utilidad: Función que monta y abre el dispositivo virtual
// Parámetros de entrada: camino (ruta donde crear o acceder al dispositivo virtual)
// Salida: Devuelve -1 (FALLO) en caso de error y el descriptor de la opertura en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
int bmount(const char *camino) {
    umask(000);
    descriptor = open(camino, O_CREAT | O_RDWR, 0666);
    if(descriptor == -1) {
        perror(RED "Error al abrir la ruta");
        return FALLO;
    }
    return descriptor;
}

// Nombre: bumount
// Utilidad: Función que desmonta el dispositivo virtual llamando a la función close para liberar el descriptor del fichero
// Parámetros de entrada: Ninguno
// Salida: Devuelve -1 (FALLO) en caso de error y 0 (EXITO) en caso de cierre exitoso
// Dónde se utiliza: Main de mi_mkfs
int bumount() {
    int cierre = close(descriptor);
    if(cierre == -1) {
        perror(RED "Error al cerrar la ruta");
        return FALLO;
    }
    return EXITO;
}

// Nombre: bwrite
// Utilidad: Función que escribe 1 bloque en el dispositivo virtual, en el bloque físico especificado por nbloque
// Parámetros de entrada: nbloque (número de bloque) y buf (puntero al buffer de memoria)
// Salida: Devuelve -1 (FALLO) en caso de error o la cantidad escrita (tamaño de un bloque, BLOCKSIZE) en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
int bwrite(unsigned int nbloque, const void *buf) {
    off_t desplazamiento = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    if(desplazamiento == -1) {
        perror(RED "Error al desplazar el puntero");
        return FALLO;
    }
    
    size_t bytes_escritos = write(descriptor, buf, BLOCKSIZE);
    if(bytes_escritos == -1) {
        perror(RED "Error al escribir un bloque");
        return FALLO;
    }
    return bytes_escritos;
}

// Nombre: bread
// Utilidad: Función que lee 1 bloque del dispositivo virtual, que se corresponde con el bloque físico especificado por nbloque
// Parámetros de entrada: nbloque (número de bloque) y buf (puntero al buffer de memoria)
// Salida: Devuelve -1 (FALLO) en caso de error o la cantidad leída (tamaño de un bloque, BLOCKSIZE) en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
int bread(unsigned int nbloque, void *buf) {
    off_t desplazamiento = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    if(desplazamiento == -1) {
        perror(RED "Error al desplazar el puntero");
        return FALLO;
    }

    size_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);
    if(bytes_leidos == -1) {
        perror(RED "Error al leer un bloque");
        return FALLO;
    }
    return bytes_leidos;
}
