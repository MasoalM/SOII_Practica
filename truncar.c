#include "ficheros.h"

int main(int argc, char **argv) {
    /*
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        perror(RED "Error al leer el superbloque");
        return FALLO;  // Usar un código de error adecuado
    }
    */

    // Comprobar la sintaxis ./truncar <nombre_dispositivo> <ninodo> <nbytes>
    if(argc != 4) {
        fprintf(stderr, "Sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes> \nSi diferentes_inodos=0 se reserva un solo inodo para todos los offsets");
        return FALLO;
    }

    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);

    struct inodo inodo;
    //if (leer_inodo(ninodo, &inodo) == FALLO) {
    //    perror("Error al leer inodo");
    //    return FALLO;
    //}

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

    
    if(nbytes == 0) {
        if(liberar_inodo(ninodo)==FALLO) return FALLO;
    } else {
        if(mi_truncar_f(ninodo, nbytes)==FALLO) return FALLO;
    }
    
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        perror("Error al leer inodo");
        return FALLO;
    }
    // mostrar el inodo reservado
    printf("\nDATOS DEL INODO RESERVADO %d\n", ninodo);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %d\n", inodo.permisos);
    
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];
    
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ATIME: %s \nMTIME: %s \nCTIME: %s \nBTIME: %s \n",atime,mtime,ctime, btime);

    printf("nlinks: %d\n", inodo.nlinks);
    printf("inodo.tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    //printf("nbytes: %d\n", nbytes);
    printf("bloques ocupados: %d\n\n\n\n\n\n", inodo.numBloquesOcupados);

    /*
    if (bwrite(posSB, &SB) == FALLO) {
        perror(RED "Error al escribir el superbloque");
        return FALLO;
    }
    */
    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}