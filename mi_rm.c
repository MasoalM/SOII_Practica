#include "directorios.h"

int main(int argc, char **argv){
    if(argc != 3) {
        fprintf(stderr, RED"Sintaxis: ./mi_rm disco /ruta"WHITE);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error: No se pudo montar el dispositivo.\n" WHITE);
        return FALLO;
    }

    int r;
    r=mi_unlink(argv[2]);
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