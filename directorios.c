#include "directorios.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NUM_ENTRADAS_POR_BLOQUE (BLOCKSIZE / sizeof(struct entrada)) 

// Nombre: extraer_camino
// Utilidad: Esta función toma una ruta absoluta (comenzando con '/'), la divide en dos partes:
// - inicial: el primer componente de la ruta (el nombre de un directorio o fichero).
// - final: el resto de la ruta después de 'inicial'.
// - tipo: determina si lo que encontramos es un directorio ('d') o un fichero ('f').
// Parámetros de entrada:
// - camino (const char *): La ruta absoluta que se quiere dividir. Ejemplo: "/dir1/dir2/fich".
// - inicial (char *): El primer componente de la ruta (nombre del directorio o fichero).
// - final (char *): El resto de la ruta después de 'inicial'.
// - tipo (char *): Determina si es un directorio ('d') o un fichero ('f').
// Salida: Devuelve EXITO si la operación es exitosa. Si la ruta no empieza con '/', devuelve FALLO.
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

// Nombre: buscar_entrada
// Utilidad: Función que busca una entrada dentro de un directorio y, si no existe, la crea. 
// Esta función también se encarga de separar el camino en partes (inicial, final) y realizar búsquedas recursivas.
// Parámetros de entrada:
// - camino_parcial (const char *): La ruta del archivo o directorio a buscar.
// - p_inodo_dir (unsigned int *): Dirección del inodo del directorio donde se busca la entrada.
// - p_inodo (unsigned int *): Dirección del inodo del archivo o directorio encontrado o creado.
// - p_entrada (unsigned int *): Dirección del número de entrada dentro del directorio.
// - reservar (char): 0 para solo consultar, 1 para crear una nueva entrada si no existe.
// - permisos (unsigned char): Permisos de acceso para la nueva entrada (si se está creando).
// Salida: Devuelve EXITO (0) si la operación fue exitosa o un código de error negativo en caso de fallo.
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
        } else {
            printf(GRAY "[buscar_entrada()→ creada entrada: %s, %d]\n" WHITE , inicial, entrada.ninodo);
        }

        //num_entrada_inodo = cant_entradas_inodo; // última entrada añadida
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
    *p_inodo = 0;
    *p_entrada = 0;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
} 

// Nombre: leer_entrada
// Utilidad: Esta función lee una entrada de un directorio especificado por el número de inodo del directorio y el número de entrada dentro del directorio.
// Parámetros de entrada:
// - ninodo_dir (int): El número de inodo del directorio donde se encuentra la entrada.
// - ent (struct entrada *): Puntero a la estructura de entrada donde se guardará la información de la entrada leída.
// - num_entrada (int): El número de entrada dentro del directorio que se desea leer.
// Salida: Devuelve 0 si la operación es exitosa, o un código de error en caso de fallo
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

// Nombre: calcular_num_entradas
// Utilidad: Esta función calcula el número total de entradas dentro de un directorio,
// basado en el tamaño total de las entradas en el directorio y el tamaño de una entrada.
// Parámetros de entrada:
// - ninodo_dir (int): El número de inodo del directorio donde se quiere calcular el número de entradas.
// - n_entradas (int *): Puntero a la variable donde se almacenará el número de entradas calculado.
// Salida: Devuelve EXITO (0) si la operación es exitosa, o FALLO en caso de error.

int calcular_num_entradas(int ninodo_dir, int *n_entradas) {
    struct inodo inodo;
    if(leer_inodo(ninodo_dir, &inodo)==FALLO) return FALLO;

    *n_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    return EXITO;
}

// Nombre: mostrar_error_buscar_entrada
// Utilidad: Esta función se encarga de mostrar el mensaje de error correspondiente según el código de error recibido,
// proporcionando una explicación legible para el usuario sobre qué ha fallado durante la operación de búsqueda de entradas.
// Parámetros de entrada:
// - error (int): El código de error devuelto por la función que llamó a `buscar_entrada`.
// Salida: No tiene valor de retorno. Muestra un mensaje de error en `stderr`.

void mostrar_error_buscar_entrada(int error) {
    // fprintf(stderr, "Error: %d\n", error);
    switch (error) {
        //case -1: fprintf(stderr, RED "ERROR GENÉRICO POR IMPLEMENTAR\n"WHITE); break;
        case -2: fprintf(stderr, RED "Error: Camino incorrecto.\n" WHITE); break;
        case -3: fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" WHITE); break;
        case -4: fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" WHITE); break;
        case -5: fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" WHITE); break;
        case -6: fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" WHITE); break;
        case -7: fprintf(stderr, RED "Error: El archivo ya existe.\n" WHITE); break;
        case -8: fprintf(stderr, RED "Error: No es un directorio.\n" WHITE); break;
    }
}

// Nombre: mi_creat
// Utilidad: Esta función crea un fichero o directorio en el sistema de ficheros.
// Parámetros de entrada:
// - camino (const char *): La ruta del fichero o directorio a crear.
// - permisos (unsigned char): Los permisos con los que se crea el fichero o directorio (en formato octal).
// Salida: Devuelve el valor devuelto por `buscar_entrada`, que puede ser EXITO (0) o un código de error.

int mi_creat(const char *camino, unsigned char permisos){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    
    return buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
}

// Nombre: mi_dir
// Utilidad: Esta función lista el contenido de un directorio, llenando un buffer con los nombres de las entradas del directorio.
// Parámetros de entrada:
// - camino (const char *): La ruta del directorio que se desea listar.
// - buffer (char *): El buffer donde se almacenarán los nombres de las entradas del directorio.
// Salida: Devuelve el número de entradas encontradas si la operación es exitosa, o un código de error en caso de fallo.
int mi_dir(const char *camino, char *buffer) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    struct inodo inodo;
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    int offset = 0, total = 0, leidos;

    // Buscar la entrada y obtener el inodo
    int error;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
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

// Nombre: mi_chmod
// Utilidad: Esta función cambia los permisos de un fichero o directorio especificado por la ruta.
// Parámetros de entrada:
// - camino (const char *): La ruta del fichero o directorio cuyo permiso se desea cambiar.
// - permisos (unsigned char): Los permisos que se asignarán al fichero o directorio (en formato octal).
// Salida: Devuelve el resultado de llamar a `mi_chmod_f` para cambiar los permisos del inodo, o FALLO en caso de error.

int mi_chmod(const char *camino, unsigned char permisos) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

    // Buscar la entrada y obtener el inodo
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (r < 0) return FALLO;
    return mi_chmod_f(p_inodo, permisos);
}

// Nombre: mi_stat
// Utilidad: Esta función obtiene los metadatos (información del inodo) de un fichero o directorio especificado por la ruta.
// Parámetros de entrada:
// - camino (const char *): La ruta del fichero o directorio cuyo metadato se desea obtener.
// - p_stat (struct STAT *): Puntero a la estructura donde se almacenarán los metadatos del inodo.
// Salida: Devuelve el resultado de llamar a `mi_stat_f` para obtener los metadatos del inodo, o FALLO en caso de error.

int mi_stat(const char *camino, struct STAT *p_stat){
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    
    // Buscar la entrada y obtener el inodo
    int r = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (r < 0) return FALLO;
    printf( BLUE"Nº de inodo: %d\n"WHITE , p_inodo);
    return mi_stat_f(p_inodo, p_stat);
}

//NIVEL 9

// Nombre: mi_write
// Utilidad: Esta función escribe contenido en un fichero especificado por la ruta, comenzando desde un offset determinado.
// Parámetros de entrada:
// - camino (const char *): La ruta del fichero en el que se desea escribir.
// - buf (const void *): Puntero al buffer que contiene los datos a escribir.
// - offset (unsigned int): El desplazamiento dentro del fichero desde el cual comenzar a escribir.
// - nbytes (unsigned int): El número de bytes a escribir desde el buffer.
// Salida: Devuelve el resultado de la función `mi_write_f` para escribir los datos, o FALLO en caso de error.
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

    // Buscar la entrada
    int error;
    if ((error=buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)   {
        mostrar_error_buscar_entrada(error);
        return FALLO;
   }


    // Escribir en el fichero
    return mi_write_f(p_inodo, buf, offset, nbytes);
}

// Nombre: mi_read
// Utilidad: Esta función lee contenido de un fichero especificado por la ruta, comenzando desde un offset determinado.
// Parámetros de entrada:
// - camino (const char *): La ruta del fichero desde el cual se desea leer.
// - buf (void *): Puntero al buffer donde se almacenarán los datos leídos.
// - offset (unsigned int): El desplazamiento dentro del fichero desde el cual comenzar a leer.
// - nbytes (unsigned int): El número de bytes a leer del fichero.
// Salida: Devuelve el resultado de la función `mi_read_f` para leer los datos, o FALLO en caso de error.
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

    // Buscar la entrada
    int error;
    if ((error=buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0)   {
         mostrar_error_buscar_entrada(error);
         return FALLO;
    }
    
    // Leer del fichero
    return mi_read_f(p_inodo, buf, offset, nbytes);
}

int mi_link(const char *camino1, const char *camino2) {
    unsigned int p_inodo_dir1 = 0, p_inodo1 = 0, p_entrada1 = 0;
    unsigned int p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;

    // Buscar la entrada                                               
    int error;

    // comprobamos que camino1 existe
    // fprintf(stderr, MAGENTA "camino1: %s\n" RESET, camino1);
    if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 0)) < 0) {
        mostrar_error_buscar_entrada(error);
                return FALLO;
    }
    // Comprobar que es un fichero
    struct inodo in1;
    if (leer_inodo(p_inodo1, &in1) == FALLO) return FALLO;
    if (in1.tipo != 'f') {
        fprintf(stderr, RED "ERROR: SE ESPERABA UN FICHERO." WHITE); 
        return FALLO;
    } 
    if ((in1.permisos & 4) != 4) { 
        #if DEBUGN7    
        fprintf(stderr, GREEN "[buscar_entrada()→ El inodo %d no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
        #endif
        return ERROR_PERMISO_LECTURA;
    }
    
    // creamos camino2
    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    struct entrada entrada;
    // leemos la entrada de p_inodo_dir2 y le asociamos p_inodo1
    mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));
    entrada.ninodo = p_inodo1;
    //printf("ENLACE entrada.nombre: %s, entrada.ninodo: %d\n", entrada.nombre, entrada.ninodo);
    mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada));

    liberar_inodo(p_inodo2);
    
    //Actualizar número de links y tiempo del inodo
    in1.nlinks++;
    in1.ctime = time(NULL);
    if(escribir_inodo(p_inodo1, &in1) < 0) return FALLO;

    // Leer del fichero
    return EXITO;
}



int mi_unlink(const char *camino) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;
    // Buscar la entrada    
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    printf("p-entradaprimeravez= %d", p_entrada);

    // Comprobar que es un fichero
    struct inodo in;
    if (leer_inodo(p_inodo, &in) == FALLO) return FALLO;

    printf("NLINKS INPRIMERO: %d", in.nlinks);
    if ((in.tipo == 'd') && (in.tamEnBytesLog>0)) {
        fprintf(stderr, RED "ERROR: EL DIRECTORIO %s NO ESTÁ VACÍO." WHITE, camino); 
        return FALLO;
    } 
    
    struct inodo inDir;
    if (leer_inodo(p_inodo_dir, &inDir) == FALLO) return FALLO;


    unsigned int nEntradas=(inDir.tamEnBytesLog/sizeof(struct entrada));
    
    printf("p-entradasegundavez= %d", p_entrada);
    printf("nEntradas= %d", nEntradas);

    if((nEntradas-1) == p_entrada) {
        //Es la última entrada
        if(mi_truncar_f(p_inodo_dir, (inDir.tamEnBytesLog-sizeof(struct entrada))) == FALLO) return FALLO;
    } else {
        //Si no es la última entrada, generamos una nueva
        struct entrada entrada;
        //La leemos
        if(leer_entrada(p_inodo_dir, &entrada, nEntradas-1)<EXITO) return FALLO;
        //escribir entrada
        if(mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada))<0) return FALLO; 
        //borramos la última (ya está guardada)
        if(mi_truncar_f(p_inodo_dir, (inDir.tamEnBytesLog-sizeof(struct entrada))) == FALLO) return FALLO;
    }
    in.nlinks--;
    if(in.nlinks==0){
        if(liberar_inodo(p_inodo) == FALLO) return FALLO;
        return EXITO;
    }
    in.ctime=time(NULL);
    if(escribir_inodo(p_inodo, &in) == FALLO) return FALLO;
    return EXITO;
}
