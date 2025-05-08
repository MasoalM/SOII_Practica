#include "directorios.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NUM_ENTRADAS_POR_BLOQUE (BLOCKSIZE / sizeof(struct entrada)) 

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    if (camino[0] != '/') {
        return FALLO;
    }

    const char *segundo_trozo = strchr(camino + 1, '/'); // Busca el segundo '/'
    if (segundo_trozo == NULL) {
        // Es un fichero
        strcpy(inicial, camino + 1); // omite el primer '/'
        final[0] = '\0'; // cadena vacía
        *tipo = 'f';
    } else {
        // Es un directorio
        size_t len = segundo_trozo - (camino + 1); // longitud del nombre de directorio
        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0'; // Aseguramos terminación
        strcpy(final, segundo_trozo); // el resto del camino, con '/' incluido
        *tipo = 'd';
    }

    return EXITO;
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)+1];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo = 0;
    struct superbloque SB;
    if(bread(posSB, &SB)==FALLO) return FALLO;
    // Caso base: raíz
    if (strcmp(camino_parcial, "/") == 0) {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }
    
    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial)+1);
    // Separar el camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0) {
        return ERROR_CAMINO_INCORRECTO;
    }
    
    printf(GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" WHITE , inicial, final, reservar);

    // Leer el inodo del directorio actual
    if (leer_inodo(*p_inodo_dir, &inodo_dir) < 0) {
        printf("ERROR AL LEER INODO");
        return FALLO;
    }    

    if ((inodo_dir.permisos & 4) != 4) { 
        #if DEBUGN7    
        fprintf(stderr, GREEN "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
        #endif
        return ERROR_PERMISO_LECTURA;
    }

    if ((inodo_dir.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;

    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    // Calcular entradas del inodo
    if (calcular_num_entradas(*p_inodo_dir, &cant_entradas_inodo) < 0) return FALLO;

    if (cant_entradas_inodo > 0) {
        
        //bucle leyendo entrada a entrada del disco
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == -1) { //Se lee la primera entrada
            fprintf(stderr, "Error: directorios.c → buscar_entrada() → mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada)).\n");
            return -1;
        }

        // buscamos la entrada cuyo nombre se encuentra en inicial
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(entrada.nombre, inicial) != 0)) {
            num_entrada_inodo++;
            //Leer siguiente entrada.
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            if (mi_read_f(*p_inodo_dir, &entrada, (num_entrada_inodo * sizeof(struct entrada)), sizeof(struct entrada)) == -1) {
                fprintf(stderr, "Error: directorios.c → buscar_entrada() → mi_read_f(*p_inodo_dir, &entrada, (num_entrada_inodo * sizeof(struct entrada)), sizeof(struct entrada))\n");
                return -1;
            }
            *p_entrada=num_entrada_inodo;
        }  
        //fin bucle leyendo entrada a entrada del disco
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
                printf(GRAY "[buscar_entrada()→ reservado inodo %d tipo d con permisos %d para %s]\n" WHITE , entrada.ninodo, permisos, inicial);
            } else {
                return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
            }
        } else {
            entrada.ninodo = reservar_inodo('f', permisos);
            printf(GRAY "[buscar_entrada()→ reservado inodo %d tipo f con permisos %d para %s]\n" WHITE , entrada.ninodo, permisos, inicial);
        }

        if (entrada.ninodo < 0) return FALLO;

        // Escribir la entrada en el directorio padre
        if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
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
        printf(GRAY "[buscar_entrada()→ creada entrada: %s, %d]\n" WHITE , inicial, entrada.ninodo);
        return EXITO;
    }

    // Continuamos la búsqueda recursiva
    *p_inodo_dir = entrada.ninodo;
    *p_inodo = 0;
    *p_entrada = 0;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}

int leer_entrada(int ninodo_dir, struct entrada *ent, int num_entrada) {
    struct inodo inodo_dir;
    leer_inodo(ninodo_dir, &inodo_dir);

    //int entrada_por_bloque = sizeof(struct entrada);
    int bloque = num_entrada / NUM_ENTRADAS_POR_BLOQUE;
    int offset = num_entrada % NUM_ENTRADAS_POR_BLOQUE;

    struct entrada buffer[NUM_ENTRADAS_POR_BLOQUE];
    bread(inodo_dir.punterosDirectos[bloque], buffer); // Función ficticia

    *ent = buffer[offset];
    return 0;
}

/* int escribir_entrada(int ninodo_dir, struct entrada *ent, int num_entrada) {
    struct inodo inodo_dir;
    leer_inodo(ninodo_dir, &inodo_dir);

    int bloque = num_entrada / NUM_ENTRADAS_POR_BLOQUE;
    int offset = num_entrada % NUM_ENTRADAS_POR_BLOQUE;

    struct entrada buffer[NUM_ENTRADAS_POR_BLOQUE];
    bread(inodo_dir.punterosDirectos[bloque], buffer); // cargar el bloque
    buffer[offset] = *ent;
    bwrite(inodo_dir.punterosDirectos[bloque], buffer); // guardar

    return 0;
} */

int calcular_num_entradas(int ninodo_dir, int *n_entradas) {
    struct inodo inodo;
    if(leer_inodo(ninodo_dir, &inodo)==FALLO) return FALLO;

    *n_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    return EXITO;
}


void mostrar_error_buscar_entrada(int error) {
    // fprintf(stderr, "Error: %d\n", error);
    switch (error) {
        case -2: fprintf(stderr, RED "Error: Camino incorrecto.\n" WHITE); break;
        case -3: fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" WHITE); break;
        case -4: fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" WHITE); break;
        case -5: fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" WHITE); break;
        case -6: fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" WHITE); break;
        case -7: fprintf(stderr, RED "Error: El archivo ya existe.\n" WHITE); break;
        case -8: fprintf(stderr, RED "Error: No es un directorio.\n" WHITE); break;
    }
}

int mi_creat(const char *camino, unsigned char permisos){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    
    return buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
}

int mi_dir(const char *camino, char *buffer) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    struct inodo inodo;
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    int offset = 0, total = 0, leidos;

    // Buscar la entrada y obtener el inodo
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (r < 0) return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

    // Leer el inodo del camino
    if (leer_inodo(p_inodo, &inodo) < 0) return FALLO;

    // Verificar que sea un directorio
    if (inodo.tipo != 'd') return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;

    // Verificar permiso de lectura
    if (!(inodo.permisos & 4)) return ERROR_PERMISO_LECTURA;

    // Leer todas las entradas del directorio
    int n_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    while (total < n_entradas) {
        leidos = mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
        if (leidos < 0) return FALLO;

        int num = leidos / sizeof(struct entrada);
        for (int i = 0; i < num && total < n_entradas; i++, total++) {
            strcat(buffer, entradas[i].nombre);
            strcat(buffer, "\t");
        }

        offset += leidos;
    }

    return total;
}

int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

    // Buscar la entrada y obtener el inodo
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (r < 0) return FALLO;
    return mi_chmod_f(p_inodo, permisos);
}

int mi_stat(const char *camino, struct STAT *p_stat){
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    
    // Buscar la entrada y obtener el inodo
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (r < 0) return FALLO;
    printf( BLUE"Nº de inodo: %d\n"WHITE , p_inodo);
    return mi_stat_f(p_inodo, p_stat);
}

//NIVEL 9

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    
    // Buscar la entrada y obtener el inodo
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (r < 0) return FALLO;

    return mi_write_f(p_inodo, buf, offset, nbytes);
}

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    
    // Buscar la entrada y obtener el inodo
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (r < 0) return FALLO;

    return mi_read_f(p_inodo, buf, offset, nbytes);
}