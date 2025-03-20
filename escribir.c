#include "ficheros.h"

int main(int argc, char **argv) {
    //char buffer[tamanyo];

    // Comprobar la sintaxis ./escribir <nombre_dispositivo> <"$(cat fichero)"> <diferentes_inodos>
    if(argc != 4) {
        perror("Error, faltan o sobran argumentos: $ ./escribir <nombre_dispositivo> <""$(cat fichero)""> <diferentes_inodos>");
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    char *texto = argv[2];
    int ninodo = atoi(argv[3]);
    int offset = atoi(argv[4]);

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

    // código

    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}