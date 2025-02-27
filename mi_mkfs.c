#include "ficheros_basico.h"
#include <string.h>

// Método main
// Utilidad: monta el dispositivo virtual, escribe n bloques y desmonta el dispositivo virtual
// Parámetros de entrada: argc (número de argumentos) y argv (array que contiene los argumentos)
// Salida: Devuelve -1 (FALLO) en caso de error y 0 (EXITO) en caso de ejecución exitosa
int main(int argc, char **argv) {
    unsigned char buffer [BLOCKSIZE];
    memset(buffer, '\0', sizeof(buffer));
    
    if(argc != 3) {
        perror("Error, faltan o sobran argumentos: $ ./mi_mkfs <nombre_dispositivo> <nbloques>");
        return FALLO;
    }

    if(bmount(argv[1])==FALLO) return FALLO;

    int num_bloques = atoi(argv[2]);
    for(int i = 0; i < num_bloques; i++) {
        bwrite(i, buffer);
    }
    
    initSB(num_bloques, num_bloques/4);
    initMB();
    initAI();

    reservar_inodo('d', 7);

    if(bumount() == FALLO) return FALLO;

    return EXITO;
}