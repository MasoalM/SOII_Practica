#include "directorios.h"

int main(int argc, char **argv) {
    struct STAT stat;
    // Validar sintaxis
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: ./mi_stat <disco> </ruta>" WHITE);
        return FALLO;
    }

    // Montar el disco
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error: No se pudo montar el dispositivo.\n" WHITE);
        return FALLO;
    }

    // Cambiar permisos usando mi_chmod()
    int r = mi_stat(argv[2], &stat);
    if (r < 0) {
        mostrar_error_buscar_entrada(r);  // error de buscar_entrada
    }
    

    // Mostrar tipo y permisos
    printf("tipo: %c\n", stat.tipo);
    printf("permisos: %d\n", stat.permisos);

    // Formatear fechas
    char atime[80], mtime[80], ctime[80], btime[80];
    struct tm *tm;

    tm = localtime(&stat.atime);
    strftime(atime, 80, "%a %Y-%m-%d %H:%M:%S", tm);

    tm = localtime(&stat.mtime);
    strftime(mtime, 80, "%a %Y-%m-%d %H:%M:%S", tm);

    tm = localtime(&stat.ctime);
    strftime(ctime, 80, "%a %Y-%m-%d %H:%M:%S", tm);

    tm = localtime(&stat.ctime);  // En sistemas reales sería btime, pero usamos ctime
    strftime(btime, 80, "%a %Y-%m-%d %H:%M:%S", tm);

    // Mostrar metadatos
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("btime: %s\n", btime);
    printf("nlinks: %d\n", stat.nlinks);
    printf("tamEnBytesLog: %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", stat.numBloquesOcupados);

    // Desmontar
    if(bumount()==FALLO) return FALLO;

    return r;
}






