#include "bloques.h"
#include "bloques.c"
#include <string.h>

// Método main
// Utilidad: monta el dispositivo virtual, escribe n bloques y desmonta el dispositivo virtual
// Parámetros de entrada: argc (número de argumentos) y argv (array que contiene los argumentos)
// Salida: Devuelve -1 (FALLO) en caso de error y 0 (EXITO) en caso de ejecución exitosa
int main(int argc, char **argv) {
    unsigned char *buffer [BLOCKSIZE];
    memset(buffer, '\0', sizeof(buffer));

    if(argc < 3) {
        perror("Error, faltan argumentos: $ ./mi_mkfs <nombre_dispositivo> <nbloques>");
        return FALLO;
    } 
    if(strcmp(argv[0], "./mi_mkfs") != 0) {
        perror("Error de sintaxis");
        return FALLO;
    }
    bmount(argv[1]);
    
    int num_bloques = atoi(argv[2]);
    for(int i = 0; i < num_bloques; i++) {
        bwrite(i, buffer);
    }

    bumount();

    return EXITO;
}