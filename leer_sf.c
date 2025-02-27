#include "ficheros_basico.h"

int main(int argc, char **argv) {   
    struct superbloque SB;
    
    struct inodo inodoRaiz;

    char *nombreArchivo = argv[1];

    if(argc != 2) {
        perror("Error, faltan o sobran argumentos: $ ./leer_sf <nombre_dispositivo>");
        return FALLO;
    } 

    if(bmount(nombreArchivo) == FALLO) return FALLO;

    if(bread(posSB, &SB) == FALLO) {
        perror(RED "Error al leer el superbloque");
        return FALLO;
    }

    if(leer_inodo(SB.posInodoRaiz, &inodoRaiz) == FALLO) {
        perror(RED "Error al leer el inodo raíz");
        return FALLO;
    }

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n",SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n",SB.cantInodosLibres);
    printf("totBloques = %d\n", SB.totBloques);
    printf("totInodos = %d\n\n", SB.totInodos);
    
    printf("sizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n\n", sizeof(struct inodo));

    /*
    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        if (bread(i, inodos) == FALLO){
            perror(RED "Error al leer el array de inodos");
            return FALLO;
        }
        for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            printf("%d ", inodos[j].punterosDirectos[0]);
        }    
    }
    printf("\n");
    */

    // reservar y liberar bloque
    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    unsigned int bloqueReservado = reservar_bloque();
    printf("Se ha reservado el bloque físico número %d que era el primero libre indicado por el MB\n", bloqueReservado);
    printf("SB.cantidadBloquesLibres = %d\n", SB.cantBloquesLibres);
    liberar_bloque(bloqueReservado);
    printf("Liberamos ese bloque y después SB.cantidadBloquesLibres = %d\n", SB.cantBloquesLibres);

    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf("leer_bit(posSB) -> %d\n", leer_bit(posSB));
    printf("leer_bit(SB.posPrimerBloqueMB) -> %d\n", leer_bit(SB.posPrimerBloqueMB));
    printf("leer_bit(SB.posUltimoBloqueMB) -> %d\n", leer_bit(SB.posUltimoBloqueMB));
    printf("leer_bit(SB.posPrimerBloqueAI) -> %d\n", leer_bit(SB.posPrimerBloqueAI));
    printf("leer_bit(SB.posUltimoBloqueAI) -> %d\n", leer_bit(SB.posUltimoBloqueAI));
    printf("leer_bit(SB.posPrimerBloqueDatos) -> %d\n", leer_bit(SB.posPrimerBloqueDatos));
    printf("leer_bit(SB.posUltimoBloqueDatos) -> %d\n", leer_bit(SB.posUltimoBloqueDatos));
    
    // mostrar el inodo raiz
    printf("DATOS DEL DIRECTORIO RAÍZ\n");
    printf("tipo: %c\n", inodoRaiz.tipo);
    printf("permisos: %d\n", inodoRaiz.permisos);
    
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];


    struct inodo inodo;
    int ninodo;

    leer_inodo(ninodo, &inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.btime);
    strftime(ctime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ID: %d ATIME: %s MTIME: %s CTIME: %s BTIME: %s\\n",ninodo,atime,mtime,ctime, btime);

    printf("nlinks: %d\n", inodoRaiz.nlinks);
    printf("tamEnBytesLog: %d\n", inodoRaiz.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodoRaiz.numBloquesOcupados);
 
    if (bumount() == FALLO) return FALLO;

    return EXITO;
}