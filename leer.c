#include "ficheros.h"

int main(int argc, char **argv) {
    int tambuffer;
    struct inodo in;
    struct superbloque SB;
    // comprobar sintaxis ./leer <nombre_dispositivo> <ninodo> 
    if(argc != 3) {
        perror("Error, faltan o sobran argumentos: $ ./leer <nombre_dispositivo> <ninodo>");
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

    if(bread(posSB, &SB) == FALLO) {
        perror(RED "Error al leer el superbloque");
        return FALLO;
    }

    if(!(0 <= *argv[2]) && !(*argv[2] < SB.totInodos)) {
        perror(RED "Error: no existe el número de inodo pasado por parámetro.");
        return FALLO;
    }

    leer_inodo(argv[2], &in);

    // código

    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}