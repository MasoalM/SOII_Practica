#include "ficheros_basico.h"
#include <limits.h>

int potencia(int base, int exponente) {
    int resultado = 1;
    for (int i = 0; i < exponente; i++) {
        resultado *= base;
    }
    return resultado;
}

int tamMB(unsigned int nbloques) {
    int resultado = (nbloques / 8) / BLOCKSIZE;
    if((nbloques / 8) % BLOCKSIZE > 0) return (resultado+1);
    return resultado;
}

int tamAI(unsigned int ninodos) {
    int resultado = (ninodos * INODOSIZE) / BLOCKSIZE;
    if((ninodos * INODOSIZE) % BLOCKSIZE) return (resultado+1);
    return resultado;
}

int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;  // Superbloque
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
    struct superbloque SB;  // Superbloque
    unsigned char bufferMB[BLOCKSIZE];

    if (bread(posSB, &SB) == FALLO) return FALLO; //gestión de errores

    // Cantidad de bloques ocupados por metadatos (representados por bits)
    unsigned int bloquesMetadatos = tamSB + tamAI(SB.totInodos) + tamMB(SB.totBloques);

    // Convertimos los bits en bloques físicos
    unsigned int bloquesOcupados = bloquesMetadatos / 8 / BLOCKSIZE;
    unsigned int bytesOcupados = bloquesMetadatos / 8;
    unsigned int bitsRestantes = bloquesMetadatos % 8;

    // Restar bloques ocupados del superbloque
    SB.cantBloquesLibres -= bloquesMetadatos;

    // Escribir bloques completos del mapa de bits
    for (unsigned int i = 0; i < bloquesOcupados; i++) {
        memset(bufferMB, 255, BLOCKSIZE);  // 11111111 en cada byte
        if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == FALLO) {
            perror(RED "Error al escribir el mapa de bits");
            return FALLO;
        }
    }

    // Preparar el bloque parcial
    memset(bufferMB, 0, BLOCKSIZE);
    unsigned int bytesParciales = bytesOcupados - (bloquesOcupados * BLOCKSIZE);

    // Rellenar los bytes completos del bloque parcial
    for (unsigned int i = 0; i < bytesParciales; i++) {
        bufferMB[i] = 255;
    }

    unsigned int valor = 0;
    for(unsigned int j = 0; j < bitsRestantes; j++){
        valor += (int) potencia(2,(7 - j));
    }
    bufferMB[bytesParciales]=valor;

    // Escribir el bloque parcial si hay datos que guardar
    if (bwrite(SB.posPrimerBloqueMB + bloquesOcupados, bufferMB) == FALLO) {
        perror(RED "Error al escribir el bloque parcial del mapa de bits");
        return FALLO;
    }

    // Escribir el superbloque actualizado
    if (bwrite(posSB, &SB) == FALLO) {
        perror(RED "Error al escribir el superbloque");
        return FALLO;
    }

    return EXITO;
}

int initAI() {
    struct superbloque SB;  // Superbloque
    if (bread(posSB, &SB) == FALLO) return FALLO; //gestión de errores
    struct inodo inodos[BLOCKSIZE/INODOSIZE]; // inodos
    int contInodos = SB.posPrimerInodoLibre + 1;     // hemos inicializado SB.posPrimerInodoLibre = 0
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
            //escribimos el bloque de inodos i en el dispositivo virtual
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


//INICIO NIVEL 3
int escribir_bit(unsigned int nbloque, unsigned int bit){

    int posByte=nbloque/8;

    int posBit=nbloque%8;

    unsigned char mascara=128; // 10000000

    mascara >>=posBit; // desplazamiento de bits a la derecha

    struct superbloque SB;

    if(bread(posSB,&SB)==FALLO) return FALLO;

    unsigned char bufferMB[BLOCKSIZE];

    if(bread(SB.posPrimerBloqueMB,bufferMB)==FALLO) return FALLO;

    if(bit==0){
        bufferMB[posByte] &= ~mascara; //poner a 0 el bit indicado
    }else{
        bufferMB[posByte] /= mascara; //poner a 1 el bit indicado
    }

    if(bwrite(SB.posPrimerBloqueMB,bufferMB)==FALLO) return FALLO;

    return EXITO;
}

char leer_bit(unsigned int nbloque){
    unsigned char bufferMB[BLOCKSIZE];

    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha 
                                  // para dejar el 0 o 1 en el extremo derecho y leerlo en decimal
    return mascara;
}

int reservar_bloque(){

    struct superbloque SB;
    if(bread(posSB, &SB)==FALLO) return FALLO;
    if(SB.cantBloquesLibres==0) return FALLO;

    int nbloqueMB=SB.posPrimerBloqueDatos;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];

    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con bits a 1
    bread(nbloqueMB + SB.posPrimerBloqueMB , bufferMB);

    while(memcmp(bufferAux, bufferMB, BLOCKSIZE)==0){
        nbloqueMB++;
        bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB);
    }


}

