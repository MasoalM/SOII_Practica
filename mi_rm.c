#include "directorios.h"

int main(int argc, char **argv){
    if(argc != 3) {
        fprintf(stderr, RED"Sintaxis: ./mi_rm disco /ruta"WHITE);
        return FALLO;
    }

    int r=mi_unlink(argv[2]);
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