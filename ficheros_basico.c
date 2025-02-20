#include "ficheros_basico.h"

struct superbloque SB;  // Superbloque

int tamMB(unsigned int nbloques) {
    int resultado = (nbloques / 8) / BLOCKSIZE;
    // nbloques % 8 ?
    if((nbloques / 8) % BLOCKSIZE > 0) return (resultado+1);
    return resultado;
}

int tamAI(unsigned int ninodos) {
    int resultado = (ninodos * INODOSIZE) / BLOCKSIZE;
    if((ninodos * INODOSIZE) % BLOCKSIZE) return (resultado+1);
    return resultado;
}

int initSB(unsigned int nbloques, unsigned int ninodos) {
    unsigned int bufferSB [BLOCKSIZE];       
    SB.posPrimerBloqueMB = posSB + tamSB;       //Posición del primer bloque del mapa de bits 
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;  //Posición del último bloque del mapa de bits 
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;    //Posición del primer bloque del array de inodos 
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;  //Posición del último bloque del array de inodos   
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;     //Posición del primer bloque de datos    
    SB.posUltimoBloqueDatos = nbloques-1;       //Posición del último bloque de datos 
    SB.posInodoRaiz = 0;                //Posición del inodo del directorio raíz en el array de inodos
    SB.posPrimerInodoLibre = 0;         // Posición del primer inodo libre
    SB.cantBloquesLibres = nbloques;    // Cantidad bloques libres
    SB.cantInodosLibres = ninodos;      // Cantidad inodos libres en el array de inodos    
    SB.totBloques = nbloques;       // Número total de bloques    
    SB.totInodos = ninodos;         // Número total de inodos

    memset(bufferSB, 0, BLOCKSIZE);
    memcpy(bufferSB, &SB, sizeof(struct superbloque));

    if (bwrite(posSB, bufferSB) == -1) {
        perror(RED "Error al escribir el superbloque");
        return FALLO;
    }
    return EXITO;
}

int initMB() {
    unsigned char bufferMB [BLOCKSIZE];
    
    int bloques = tamSB + tamAI(SB.totInodos) + tamMB(SB.totBloques); // bloques
    int bits = (bloques/8) / BLOCKSIZE; // bits
    SB.cantBloquesLibres -= bloques;    // se restan todos los bloques de metadatos de los bloques libres

    for(int i = 0; i < bits; i++) {
        for(int j = 0; j < BLOCKSIZE; j++) {
            
        }
    }

    
    
}

int initAI() {

}