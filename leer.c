#include "ficheros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAMBUFFER 1500  // Tamaño del buffer de lectura

int main(int argc, char **argv) {
    // Verificar sintaxis del comando
    if (argc != 3) {
        fprintf(stderr, "Uso incorrecto: ./leer <nombre_dispositivo> <ninodo>\n");
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);

    // Montar el dispositivo virtual
    if (bmount(nombre_dispositivo) == FALLO) {
        perror("Error al montar el dispositivo");
        return FALLO;
    }

    struct inodo in;
    if (leer_inodo(ninodo, &in) == FALLO) {
        perror("Error al leer el inodo");
        if(bumount()==FALLO) return FALLO;
        return FALLO;
    }

    // Leer el contenido del inodo bloque a bloque
    char buffer_texto[TAMBUFFER];
    int offset = 0, leidos, total_leidos = 0;

    memset(buffer_texto, 0, TAMBUFFER);
    leidos=mi_read_f(ninodo, buffer_texto, offset, TAMBUFFER);

    while(leidos > 0){
        write(1, buffer_texto, leidos); //leidos no tendría pq completar el buffer
        total_leidos+=leidos;
        offset+=TAMBUFFER;
        memset(buffer_texto, 0, TAMBUFFER);
        leidos=mi_read_f(ninodo, buffer_texto, offset, TAMBUFFER);
    }

    // Leer el inodo nuevamente para obtener su tamaño lógico actualizado
    if (leer_inodo(ninodo, &in) == FALLO) {
        perror("Error al leer el inodo tras la lectura del archivo");
        if(bumount()==FALLO) return FALLO;
        return FALLO;
    }

    // Mostrar cantidad total de bytes leídos
    printf("\nBytes leídos: %d\nTamaño lógico del inodo: %d\n", total_leidos, in.tamEnBytesLog);

    // Desmontar el dispositivo virtual
    if (bumount() == FALLO) {
        perror("Error al desmontar el dispositivo");
        return FALLO;
    }

    return EXITO;
}
