#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, RED "Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n" WHITE);
        return FALLO;
    }

    // Comprobar que los permisos están entre 0 y 7
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Error: modo inválido: <<%s>>\n" WHITE, argv[2]);
        return FALLO;
    }
    
    // Montar sistema de ficheros
    if(bmount(argv[1])==FALLO) return FALLO;

    // Crear el directorio
    int error = mi_creat(argv[3], permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
    }

    // Desmontar sistema
    if(bumount() == FALLO) return FALLO;

    return error;
}
