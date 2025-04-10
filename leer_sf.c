#include "directorios.h"

void mostrar_buscar_entrada(char *camino, char reservar) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
    return;
}

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
    
    //printf("sizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    //printf("sizeof struct inodo: %lu\n\n", sizeof(struct inodo));

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
    /*
    printf("INODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");
    unsigned int posInodoReservado = reservar_inodo('f', 6);

    struct inodo in;

    traducir_bloque_inodo(posInodoReservado, 8, 1);
    traducir_bloque_inodo(posInodoReservado, 204, 1);
    traducir_bloque_inodo(posInodoReservado, 30004, 1);
    traducir_bloque_inodo(posInodoReservado, 400004, 1);
    traducir_bloque_inodo(posInodoReservado, 468750, 1);

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
    printf("tamEnBytesLog: %d\n", in.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", in.numBloquesOcupados);

    printf("SB.posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    */
    /*
    // reservar y liberar bloque
    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    unsigned int bloqueReservado = reservar_bloque();
    //volvemos a leer el SB dado que se ha actualizado
    if(bread(posSB, &SB) == FALLO) {
        perror(RED "Error al leer el superbloque");
        return FALLO;
    }
    
    printf("Se ha reservado el bloque físico número %d que era el primero libre indicado por el MB\n", bloqueReservado);
    printf("SB.cantidadBloquesLibres = %d\n", SB.cantBloquesLibres);
    liberar_bloque(bloqueReservado);
    
    //volvemos a leer el SB dado que se ha actualizado
    if(bread(posSB, &SB) == FALLO) {
        perror(RED "Error al leer el superbloque");
        return FALLO;
    }
    
    printf("Liberamos ese bloque y después SB.cantidadBloquesLibres = %d\n", SB.cantBloquesLibres);

    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf("leer_bit(posSB) -> %d\n", leer_bit(posSB));
    printf("leer_bit(SB.posPrimerBloqueMB) -> %d\n", leer_bit(SB.posPrimerBloqueMB));
    printf("leer_bit(SB.posUltimoBloqueMB) -> %d\n", leer_bit(SB.posUltimoBloqueMB));
    printf("leer_bit(SB.posPrimerBloqueAI) -> %d\n", leer_bit(SB.posPrimerBloqueAI));
    printf("leer_bit(SB.posUltimoBloqueAI) -> %d\n", leer_bit(SB.posUltimoBloqueAI));
    printf("leer_bit(SB.posPrimerBloqueDatos) -> %d\n", leer_bit(SB.posPrimerBloqueDatos));
    printf("leer_bit(SB.posUltimoBloqueDatos) -> %d\n", leer_bit(SB.posUltimoBloqueDatos));
    
    // mostrar el inodo raiz
    printf("\nDATOS DEL DIRECTORIO RAÍZ\n");
    printf("tipo: %c\n", inodoRaiz.tipo);
    printf("permisos: %d\n", inodoRaiz.permisos);
    
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];


    struct inodo inodo;
    int ninodo = 0;

    leer_inodo(ninodo, &inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s \nBTIME: %s \n",ninodo,atime,mtime,ctime, btime);

    printf("nlinks: %d\n", inodoRaiz.nlinks);
    printf("tamEnBytesLog: %d\n", inodoRaiz.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodoRaiz.numBloquesOcupados);

    */

    //Mostrar creación directorios y errores
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  
    //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
 
    if (bumount() == FALLO) return FALLO;

    return EXITO;
}