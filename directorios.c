#include "directorios.h" // directorios.h?
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    if (camino[0] != '/') {
        return FALLO; // Error: el camino no comienza con '/'
    }
 
    char *copia= malloc(strlen(camino) + 1);
 
    if (copia == NULL) {
      perror("Error al reservar memoria");
       return FALLO;
    }
 
    // Copiar el contenido
    strcpy(copia, camino);
 
    char *token = strtok(copia, "/");
    inicial=token;
    *tipo='f';
    final = "";
    while (token != NULL) {
        *tipo='d';
        printf("Token: %s\n", token);
        token = strtok(NULL, "/");
        strcat(final, token);
    }
    return EXITO;
}    

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo = 0;
    struct superbloque SB;

    if(bread(posSB, &SB)) return FALLO;

    // Caso base: raíz
    if (strcmp(camino_parcial, "/") == 0) {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    // Separar el camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0) {
        return ERROR_CAMINO_INCORRECTO;
    }

    // Leer el inodo del directorio actual
    if (leer_inodo(*p_inodo_dir, &inodo_dir) < 0) return FALLO;
    if ((inodo_dir.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;

    // Calcular entradas del inodo
    if (calcular_num_entradas(*p_inodo_dir, &cant_entradas_inodo) < 0) return FALLO;

    // Buscar entrada con nombre igual a 'inicial'
    while (num_entrada_inodo < cant_entradas_inodo) {
        if (leer_entrada(*p_inodo_dir, &entrada, num_entrada_inodo) < 0) return FALLO;

        if (strcmp(entrada.nombre, inicial) == 0) break;
        num_entrada_inodo++;
    }

    // Entrada no encontrada
    if ((strcmp(entrada.nombre, inicial) != 0) && (num_entrada_inodo == cant_entradas_inodo)) {
        if (!reservar) return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

        // Crear nueva entrada
        if (inodo_dir.tipo == 'f') return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
        if ((inodo_dir.permisos & 2) != 2) return ERROR_PERMISO_ESCRITURA;

        strcpy(entrada.nombre, inicial);
        if (tipo == 'd') {
            if (strcmp(final, "/") == 0) {
                entrada.ninodo = reservar_inodo('d', permisos);
            } else {
                return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
            }
        } else {
            entrada.ninodo = reservar_inodo('f', permisos);
        }

        if (entrada.ninodo < 0) return FALLO;

        // Escribir la entrada en el directorio padre
        if (escribir_entrada(*p_inodo_dir, &entrada, cant_entradas_inodo) < 0) {
            liberar_inodo(entrada.ninodo);
            return FALLO;
        }

        num_entrada_inodo = cant_entradas_inodo; // última entrada añadida
    }

    // Si hemos llegado al final del camino
    if (strcmp(final, "/") == 0 || strcmp(final, "") == 0) {
        if ((num_entrada_inodo < cant_entradas_inodo) && reservar) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    }

    // Continuamos la búsqueda recursiva
    *p_inodo_dir = entrada.ninodo;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}



void mostrar_error_buscar_entrada(int error) {
    // fprintf(stderr, "Error: %d\n", error);
    switch (error) {
        case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
        case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
        case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
        case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
        case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
        case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
        case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
    }
}
 