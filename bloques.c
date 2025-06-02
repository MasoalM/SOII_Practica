#include "bloques.h"
#include "semaforo_mutex_posix.h"

static sem_t *mutex;

// Variable global que almacena el descriptor de un dispositivo virtual
static int descriptor = 0;

static unsigned int inside_sc = 0;

void mi_waitSem() {
    if (!inside_sc) { // inside_sc==0, no se ha hecho ya un wait
      waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}


// Nombre: bmount
// Utilidad: Función que monta y abre el dispositivo virtual
// Parámetros de entrada: camino (ruta donde crear o acceder al dispositivo virtual)
// Salida: Devuelve -1 (FALLO) en caso de error y el descriptor de la opertura en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
int bmount(const char *camino) {
    
    if(descriptor>0){
        close(descriptor);
    }
    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem(); 
        if (mutex == SEM_FAILED) {
            return FALLO;
        }
    }

    umask(000);
    descriptor = open(camino, O_CREAT | O_RDWR, 0666);
    if(descriptor==FALLO) {
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
    if(cierre == FALLO) {
        perror(RED "Error al cerrar la ruta"); 
        return FALLO;
    }
    deleteSem(); 
    return EXITO;
}

// Nombre: bwrite
// Utilidad: Función que escribe 1 bloque en el dispositivo virtual, en el bloque físico especificado por nbloque
// Parámetros de entrada: nbloque (número de bloque) y buf (puntero al buffer de memoria)
// Salida: Devuelve -1 (FALLO) en caso de error o la cantidad escrita (tamaño de un bloque, BLOCKSIZE) en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
int bwrite(unsigned int nbloque, const void *buf) {
    off_t desplazamiento = lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    if(desplazamiento == FALLO) {
        perror(RED "Error al desplazar el puntero");
        return FALLO;
    }
    
    size_t bytes_escritos = write(descriptor, buf, BLOCKSIZE);
    if(bytes_escritos == FALLO) {
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
    if(desplazamiento == FALLO) {
        perror(RED "Error al desplazar el puntero");
        return FALLO;
    }

    size_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);
    if(bytes_leidos == FALLO) {
        perror(RED "Error al leer un bloque");
        return FALLO;
    }
    return bytes_leidos;
}
