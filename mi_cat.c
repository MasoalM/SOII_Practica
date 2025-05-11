#include "directorios.h"

int main(int argc, char **argv) {
    // Comprobar la sintaxis ./mi_cat <disco> </ruta_fichero>
    
    if(argc != 3) {
        fprintf(stderr, RED"Sintaxis: ./mi_cat <disco> </ruta_fichero>"WHITE);
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    char *camino = argv[2];
    
    // Montar el dispositivo virtual
    if (bmount(nombre_dispositivo) == FALLO) {
        perror("Error al montar el dispositivo");
        return FALLO;
    }
    
    // Leer el contenido del inodo bloque a bloque
    char buffer_texto[TAMBUFFER];
    int offset = 0, leidos, total_leidos = 0;
    
    memset(buffer_texto, 0, TAMBUFFER);
    leidos=mi_read(camino, buffer_texto, offset, TAMBUFFER);
    while(leidos > 0){
        write(1, buffer_texto, leidos); 
        total_leidos+=leidos;
        offset+=TAMBUFFER;
        memset(buffer_texto, 0, TAMBUFFER);
        leidos=mi_read(camino, buffer_texto, offset, TAMBUFFER);
    }
    
    // Mostrar cantidad total de bytes leídos
    fprintf(stderr, "\nBytes leídos: %d\n", total_leidos);

    // Desmontar el dispositivo virtual
    if (bumount() == FALLO) {
        perror("Error al desmontar el dispositivo");
        return FALLO;
    }

    return EXITO;
} 