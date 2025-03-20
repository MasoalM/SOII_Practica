#include "ficheros.h"

int main(int argc, char **argv) {
    //char buffer[tamanyo];

    // Comprobar la sintaxis ./escribir <nombre_dispositivo> <"$(cat fichero)"> <diferentes_inodos>
    if(argc != 4) {
        perror("Error, faltan o sobran argumentos: $ ./escribir <nombre_dispositivo> <""$(cat fichero)""> <diferentes_inodos>");
        return FALLO;
    }

    //char *nombre_dispositivo = argv[1];
    char *texto = argv[2];
    int ninodo = atoi(argv[3]);
    //int offset = atoi(argv[4]);

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

    // código
    unsigned int posIn = reservar_inodo('f', 6);
    if(ninodo==0){
        mi_write_f(posIn, texto,9000,strlen(texto));
        mi_write_f(posIn, texto,209000,strlen(texto));
        mi_write_f(posIn, texto,30725000,strlen(texto));
        mi_write_f(posIn, texto,409605000,strlen(texto));
        mi_write_f(posIn, texto,480000000,strlen(texto));

    } else if (ninodo == 1) {
        mi_write_f(posIn, texto,9000,strlen(texto));
        posIn = reservar_inodo('f', 6);
        mi_write_f(posIn, texto,209000,strlen(texto));
        posIn = reservar_inodo('f', 6);
        mi_write_f(posIn, texto,30725000,strlen(texto));
        posIn = reservar_inodo('f', 6);
        mi_write_f(posIn, texto,409605000,strlen(texto));
        posIn = reservar_inodo('f', 6);
        mi_write_f(posIn, texto,480000000,strlen(texto));

    } else {
        printf("THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVER THE END IS NEVERTHE END IS NEVER");
    }

    struct inodo inodo;
    if(leer_inodo(posIn, &inodo)==FALLO){
        printf(RED "ERROR");
        return FALLO;
    }
    
    printf("tamEnBytesLog: %d", inodo.tamEnBytesLog);
    printf("bloques ocupados: %d", inodo.numBloquesOcupados);
    
    
    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}