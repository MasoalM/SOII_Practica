#include "ficheros_basico.h"
#include <limits.h>

struct superbloque SB;  // Superbloque
struct inodo inodos[BLOCKSIZE/INODOSIZE]; // inodos

int potencia(int base, int exponente) {
    int resultado = 1;
    for (int i = 0; i < exponente; i++) {
        resultado *= base;
    }
    return resultado;
}

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

    memset(bufferSB, 0, BLOCKSIZE * sizeof(unsigned int));
    memcpy(bufferSB, &SB, sizeof(struct superbloque));

    if (bwrite(posSB, bufferSB) == FALLO) {
        perror(RED "Error al escribir el superbloque");
        return FALLO;
    }
    return EXITO;
}

int initMB() {
    unsigned int bufferSB [BLOCKSIZE]; 
    unsigned char bufferMB [BLOCKSIZE];
    memset(bufferMB, 1, BLOCKSIZE);
    int bloques = tamSB + tamAI(SB.totInodos) + tamMB(SB.totBloques); // cantidad de bloques representados por 1 bit
    
    int nbloques = (bloques/8) / BLOCKSIZE; // nbloques físicos
    SB.cantBloquesLibres -= bloques;    // se restan todos los bloques de metadatos de los bloques libres
    int i = 0;
    for(; i < nbloques; i++) {
        for(int j = 0; j < BLOCKSIZE; j++) {
            if(bwrite(SB.posPrimerBloqueMB+i,bufferMB) == FALLO) {
                perror(RED "Error al escribir el mapa de bits");
                return FALLO;
            }
        }
    }
    int resto = (bloques%8);   //bits sueltos
    memset(bufferMB, 0, BLOCKSIZE);
    bloques/=8;
    //bloques virtuales (1 bit = 1 bloque) que no completan un bloque físico
    int k=0;
    for(;k<bloques;k++){
        bufferMB[k]=255;
    }
    //bits sobrantes
    int valor = 0;
    for(int j = 0; j < resto; j++){
        valor += (int) potencia(2,(7 - j));
    }
    bufferMB[k]=valor;
    if(bwrite(SB.posPrimerBloqueMB+i,bufferMB) == FALLO) {
        perror(RED "Error al escribir el mapa de bits");
        return FALLO;
    }
    memset(bufferSB, 0, BLOCKSIZE * sizeof(unsigned int));
    memcpy(bufferSB, &SB, sizeof(struct superbloque));
    if (bwrite(posSB, bufferSB) == FALLO) {
        perror(RED "Error al escribir el superbloque");
        return FALLO;
    }
    return EXITO;
}

int initAI() {
    printf("SBcantblocslibres3: %d\n",SB.cantBloquesLibres);
    int contInodos = SB.posPrimerInodoLibre + 1;     //si hemos inicializado SB.posPrimerInodoLibre = 0
    for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) { //para cada bloque del AI
        if(bread(i, inodos) != FALLO){  //gestión de errores
            for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {    //para cada inodo del bloque
                inodos[j].tipo = 'l';   //libre
                if(contInodos < SB.totInodos) { //si no hemos llegado al último inodo del AI
                    inodos[j].punterosDirectos[0] = contInodos; //enlazamos con el siguiente
                    contInodos++;
                } else {    //hemos llegado al último inodo
                    inodos[j].punterosDirectos[0] = UINT_MAX;
                    break; 
                }
            }
            //escribimos el inodo en la máquina virtual
            if(bwrite(i, inodos)==FALLO){
                perror(RED "Error al iniciar el array de inodos");
                return FALLO;
            }
        } else{
            //gestión de errores
            perror(RED "Error al iniciar el array de inodos");
            return FALLO;
        }
    }
    return EXITO;
}