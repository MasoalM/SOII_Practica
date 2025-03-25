#include "ficheros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TAMB_BUFFER 1500  // Tamaño del buffer de lectura

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

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        perror("Error al leer el superbloque");
        if(bumount()==FALLO) return FALLO;
        return FALLO;
    }

    // Verificar que el número de inodo es válido
    if (ninodo < 0 || ninodo >= SB.totInodos) {
        fprintf(stderr, "Error: el número de inodo %d no es válido.\n", ninodo);
        if(bumount()==FALLO) return FALLO;
        return FALLO;
    }

    struct inodo in;
    if (leer_inodo(ninodo, &in) == FALLO) {
        perror("Error al leer el inodo");
        if(bumount()==FALLO) return FALLO;
        return FALLO;
    }

    // Verificar permisos de lectura
    if ((in.permisos & 4) != 4) {
        fprintf(stderr, "Error: No tiene permisos de lectura.\n");
        if(bumount()==FALLO) return FALLO;
        return FALLO;
    }

    

    // Leer el contenido del inodo bloque a bloque
    char buffer_texto[TAMB_BUFFER];
    int offset = 0, leidos = 0, total_leidos = 0;

    do {
        memset(buffer_texto, 0, TAMB_BUFFER);  // Limpiar buffer
        leidos = mi_read_f(ninodo, buffer_texto, offset, TAMB_BUFFER);
    
        printf("DEBUG: leidos=%d, offset=%d, total_leidos=%d, tamEnBytesLog=%d\n",
               leidos, offset, total_leidos, in.tamEnBytesLog);
    
        if (leidos == FALLO) {
            perror("Error al leer el archivo");
            if (bumount() == FALLO) return FALLO;
            return FALLO;
        }
        if (leidos > 0) {
            write(1, buffer_texto, leidos);  // Mostrar contenido en salida estándar
            total_leidos += leidos;
            offset += leidos;
        }
    } while (leidos > 0 && offset < in.tamEnBytesLog);
    

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
