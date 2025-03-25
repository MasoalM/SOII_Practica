#include "ficheros.h"

int main(int argc, char **argv) {
    // comprobar sintaxis ./permitir <nombre_dispositivo> <ninodo> <permisos>
    if(argc != 4) {
        perror("Error, faltan o sobran argumentos: $ ./permitir <nombre_dispositivo> <ninodo> <permisos>");
        return FALLO;
    }

    int ninodo = atoi(argv[2]);
    unsigned char permisos = (unsigned char) atoi(argv[3]);

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

   int resultado = mi_chmod_f(ninodo,permisos);
   if (resultado == -1) {
        fprintf(stderr, "Error al cambiar los permisos del inodo %d\n", ninodo);
        bumount();
        exit(EXIT_FAILURE);
    }
    
    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}