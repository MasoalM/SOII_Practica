#include "ficheros.h"

int main(int argc, char **argv) {
    // Comprobar la sintaxis ./truncar <nombre_dispositivo> <ninodo> <nbytes>
    if(argc != 4) {
        fprintf(stderr, "Sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes> \nSi diferentes_inodos=0 se reserva un solo inodo para todos los offsets");
        return FALLO;
    }

    unsigned int ninodo = atoi(argv[2]);
    unsigned int nbytes = atoi(argv[3]);

    // Montar dispositivo virtual
    if(bmount(argv[1])==FALLO) return FALLO;

    if(nbytes == 0) {
        if(liberar_inodo(ninodo)==FALLO) return FALLO;
    } else {
        if(mi_truncar_f(ninodo, nbytes)==FALLO) return FALLO;
    }

    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == FALLO){
        return FALLO;
    }

    printf("INODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");
    unsigned int posInodoReservado = reservar_inodo('f', 6);

    struct inodo in;

    leer_inodo(posInodoReservado, &in);

    // mostrar el inodo reservado 1
    printf("\nDATOS DEL INODO RESERVADO 1\n");
    printf("tipo: %c\n", in.tipo);
    printf("permisos: %d\n", in.permisos);
    
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];
    
    ts = localtime(&in.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&in.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&in.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&in.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ATIME: %s \nMTIME: %s \nCTIME: %s \nBTIME: %s \n",atime,mtime,ctime, btime);

    printf("nlinks: %d\n", in.nlinks);

    printf("inodo.tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("nbytes: %d\n", nbytes);
    printf("nBloquesOcupados: %d\n", stat.);


    // Desmontar dispositivo virtual
    if(bumount() == FALLO) return FALLO;

    return EXITO;
}