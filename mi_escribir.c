#include "directorios.h"

int main(int argc, char **argv) {
    // Comprobar la sintaxis ./escribir <nombre_dispositivo> <"$(cat fichero)"> <diferentes_inodos>
    if(argc != 5) {
        fprintf(stderr, RED"Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>"WHITE);
        return FALLO;
    }

    char *camino = argv[2];
      //obtenemos la ruta y comprobamos que no se refiera a un directorio
    if (argv[2][strlen(argv[2])-1]=='/') {
        fprintf(stderr, RED "Error: la ruta se corresponde a un directorio.\n" RESET);
        exit(FALLO);
    }
    char *texto = argv[3];
    int offset = atoi(argv[4]);
    
    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;
    printf("Longitud texto: %ld \n", strlen(texto));
    int escritos = mi_write(camino, texto, offset, strlen(texto));
    if(escritos < 0){
        printf("Bytes escritos: 0 \n");
        return FALLO;    
    } 
    printf("Bytes escritos: %d \n", escritos);
    
    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}