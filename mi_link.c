#include "directorios.h"

int main(int argc, char **argv){

    if(argc != 4) {
        fprintf(stderr, RED"Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace"WHITE);
        return FALLO;
    }

    // Montar el dispositivo virtual
    if (bmount(argv[0]) == FALLO) {
        perror("Error al montar el dispositivo");
        return FALLO;
    }

    int r = mi_link(argv[2], argv[3]);

    if(r<0){
        mostrar_error_buscar_entrada(r);
        if (bumount() == FALLO) {
            perror("Error al desmontar el dispositivo");
            return FALLO;
        }
        return FALLO;
    }

    // Desmontar el dispositivo virtual
    if (bumount() == FALLO) {
        perror("Error al desmontar el dispositivo");
        return FALLO;
    }
    return EXITO;
}