#include "ficheros_basico.h"

int main(int argc, char **argv) {   
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
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
    printf("totInodos = %d\n", SB.totInodos);
    
    printf("sizeof struct superbloque is: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo is: %lu\n", sizeof(struct inodo));

    printf("RECORRIDO LISTA ENLAZADA INODOS LIBRES");
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        if (bread(i, inodos) == FALLO){
            perror(RED "Error al leer el array de inodos");
            return FALLO;
        }
        for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            printf("%d ", inodos[i].punterosDirectos[0]);
            fflush(stdout);
        }    
    }
    printf("\n");

    /*
    for(unsigned int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
        if (bread(i, AI) == FALLO){
            fprintf(stderr, RED "ERROR: no se ha podido leer el superbloque"RESET);
            return FALLO;
        }
        for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++){
            printf("%d ", AI[j].punterosDirectos[0]);
            fflush(stdout);
        }
    }
    printf("\n");

    */

    if (bumount() == FALLO) return FALLO;

    return EXITO;
}