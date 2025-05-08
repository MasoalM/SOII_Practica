#include "directorios.h"

int main(int argc, char **argv) {
    // Validar sintaxis
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n" WHITE);
        return FALLO;
    }

    // Validar que permisos sean un número entre 0 y 7
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Error: modo inválido: <<%s>>\n" WHITE, argv[2]);
        return FALLO;
    }

    // Montar el disco
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error: No se pudo montar el dispositivo.\n" WHITE);
        return FALLO;
    }

    // Cambiar permisos usando mi_chmod()
    int r = mi_chmod(argv[3], permisos);
    if (r < 0) {
        mostrar_error_buscar_entrada(r);  // error de buscar_entrada
    }

    // Desmontar
    if(bumount()==FALLO) return FALLO;

    return r;
}


