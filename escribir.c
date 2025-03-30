#include "ficheros.h"

int main(int argc, char **argv) {
    //char buffer[tamanyo];

    // Comprobar la sintaxis ./escribir <nombre_dispositivo> <"$(cat fichero)"> <diferentes_inodos>
    if(argc != 4) {
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <""$(cat fichero)""> <diferentes_inodos> \nOffsets: 9000, 209000, 30725000, 409605000, 480000000 \nSi diferentes_inodos=0 se reserva un solo inodo para todos los offsets");
        return FALLO;
    }

    //printf("longitud texto: %ld\n\n", strlen(argv[2]));
    char *texto = argv[2];
    int cantidadInodos = atoi(argv[3]);
    //int offset = atoi(argv[4]);

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

    // código
    unsigned int posIn = reservar_inodo('f', 6);
    unsigned int escritos = mi_write_f(posIn, texto, 9000, strlen(texto));
    if(escritos==FALLO) return FALLO;
    if(cantidadInodos==0){
        leamosInodo(posIn, 9000, escritos);
        escritos=mi_write_f(posIn, texto, 209000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 209000, escritos);
        escritos=mi_write_f(posIn, texto, 30725000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 30725000, escritos);
        escritos=mi_write_f(posIn, texto, 409605000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 409605000, escritos);
        escritos=mi_write_f(posIn, texto, 480000000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 480000000, escritos);
    } else if (cantidadInodos == 1) {
        leamosInodo(posIn, 9000, escritos);
        posIn = reservar_inodo('f', 6);
        escritos=mi_write_f(posIn, texto, 209000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 209000, escritos);
        posIn = reservar_inodo('f', 6);
        escritos=mi_write_f(posIn, texto, 30725000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 30725000, escritos);
        posIn = reservar_inodo('f', 6);
        escritos=mi_write_f(posIn, texto, 409605000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 409605000, escritos);
        posIn = reservar_inodo('f', 6);
        escritos=mi_write_f(posIn, texto, 480000000, strlen(texto));
        if(escritos==FALLO) return FALLO;
        leamosInodo(posIn, 480000000, escritos);
    }

    struct inodo inodo;
    if(leer_inodo(posIn, &inodo)==FALLO){
        printf(RED "ERROR");
        return FALLO;
    }
    
    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}

int leamosInodo(int ninodo, int offset, int bytesEscritos){
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo)==FALLO){
        printf(RED "ERROR");
        return FALLO;
    }

    printf("Bytes escritos %d\n\n", bytesEscritos);

    printf("Nº inodo reservado %d\n", ninodo);
    printf("offset %d\n", offset);
    printf("Bytes escritos %d\n", bytesEscritos);

    struct STAT stat;
    if(mi_stat_f(ninodo, &stat)==FALLO) return FALLO;
    //printf("stat.tamEnBytesLog=%d\n", stat.tamEnBytesLog);
    //printf("stat.numBloquesOcupados=%d\n\n", stat.numBloquesOcupados);

    printf("tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("bloques ocupados: %d\n\n\n\n\n\n", inodo.numBloquesOcupados);

    return EXITO;
}