#include "ficheros.h"

int main(int argc, char **argv) {
    //char buffer[tamanyo];

    // Comprobar la sintaxis ./truncar <nombre_dispositivo> <ninodo> <nbytes>
    if(argc != 4) {
        fprintf(stderr, "Sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes> \nSi diferentes_inodos=0 se reserva un solo inodo para todos los offsets");
        return FALLO;
    }

    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

    if(nbytes == 0) {
        liberar_inodo(ninodo);
    } else {
        mi_truncar_f(ninodo, nbytes);
    }

    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == FALLO){
        return FALLO;
    }

    printf("inodo.tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("nbytes: %d\n", nbytes);

    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}