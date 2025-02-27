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

    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO) return FALLO;

    unsigned int posByte=nbloque/8;

    unsigned int posBit=nbloque%8;

    unsigned int nbloqueMB = posByte / BLOCKSIZE;

    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    unsigned char bufferMB[BLOCKSIZE];

    posByte = posByte % BLOCKSIZE;

    if(bread(SB.posPrimerBloqueMB,bufferMB)==FALLO) return FALLO;

    unsigned char mascara=128; // 10000000

    mascara >>= posBit; // desplazamiento de bits a la derecha

    if(bit == 1){
        bufferMB[posByte] |= mascara; //poner a 1 el bit indicado 
    } else {
        bufferMB[posByte] &= ~mascara; //poner a 0 el bit indicado
    }

    if(bwrite(nbloqueabs,bufferMB)==FALLO) return FALLO;

    return EXITO;
}

char leer_bit(unsigned int nbloque){

    struct superbloque SB;
    if(bread(posSB,&SB)==FALLO) return FALLO;

    unsigned int posByte=nbloque/8;

    unsigned int posBit=nbloque%8;

    unsigned int nbloqueMB = posByte / BLOCKSIZE;

    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    unsigned char bufferMB[BLOCKSIZE];

    posByte = posByte % BLOCKSIZE;

    unsigned char mascara = 128; // 10000000
    mascara >>= posBit;          // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posByte]; // operador AND para bits
    mascara >>= (7 - posBit);     // desplazamiento de bits a la derecha 
                                  // para dejar el 0 o 1 en el extremo derecho y leerlo en decimal
    return mascara;
}

int reservar_bloque(){

    //paso 1
    //declaramos variables
    struct superbloque SB;
    //gestión de errores
    if(bread(posSB, &SB)==FALLO) return FALLO;
    if(SB.cantBloquesLibres==0) return FALLO;

    //vamos a gestionar el buffer
    unsigned int nbloqueMB=SB.posPrimerBloqueDatos;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];

    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con bits a 1
    //hacemos la primera lectura para entrar en el bucle
    if(bread(nbloqueMB + SB.posPrimerBloqueMB , bufferMB)==FALLO) return FALLO;

    //bucle de búsqueda del bloque con un byte libre
    while(memcmp(bufferAux, bufferMB, BLOCKSIZE)==0){
        nbloqueMB++;
        if(bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB)==FALLO)return FALLO;
    }

    //paso 2
    //buscamos el byte con el bit libre
    unsigned int posbyte=0;
    for(;bufferMB[posbyte]==255;posbyte++){};

    //paso 3
    //gestionamos el bit libre
    unsigned char mascara = 128; // 10000000
    unsigned int posbit = 0;
    while (bufferMB[posbyte] & mascara) { // operador AND para bits
        bufferMB[posbyte] <<= 1;          // desplazamiento de bits a la izquierda
        posbit++;
    }

    //paso 4
    //finalizamos

    //calculamos en número de bloque real
    int nBloqueFisico = (nbloqueMB * BLOCKSIZE * 8) +  (posbyte * 8) + posbit;

    //guardamos los datos, y gestionsmos la información obtenida
    if(escribir_bit(nBloqueFisico,1)==FALLO) return FALLO;
    SB.cantBloquesLibres--;
    if(bwrite(posSB,&SB)==FALLO) return FALLO;
    memset(bufferAux,0,BLOCKSIZE);
    if(bwrite(nBloqueFisico, bufferAux)==FALLO) return FALLO;
    return nBloqueFisico;
}

//TRATAMIENTO DE ERRORES
int liberar_bloque(unsigned int nbloque) {
    //iniciamos el superbloque
    struct superbloque SB;
    if(bread(posSB, &SB)==FALLO) return FALLO;
    //ponemos a 0 el bloque que nos dicen (lo liberamos)
    if(escribir_bit(nbloque, 0)==FALLO) return FALLO;
    //aumentamos el número de bloques libres
    SB.cantBloquesLibres++;
    //guardamos el superbloque
    if (bwrite(posSB, &SB) == FALLO) {
        perror(RED "Error al escribir el superbloque");
        return FALLO;
    }
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo){
     
    //Tratamiento de errores
    struct superbloque SB;
    if(bread(posSB, &SB)==FALLO) return FALLO;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int nbloqueAI = (ninodo*INODOSIZE)/BLOCKSIZE;
    unsigned int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
    bread(nbloqueabs, inodos);
    unsigned int posinodo = ninodo % (BLOCKSIZE/INODOSIZE);
    inodos[posinodo] = *inodo;
    bwrite(nbloqueabs, inodos);
    
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    // Leer el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO) {
        perror("Error al leer el superbloque");
        return FALLO;  // Usar un código de error adecuado
    }

    // Calcular el número de bloque donde se encuentra el inodo
    unsigned int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueAI + nbloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // Leer el bloque de inodos que contiene el inodo deseado
    if (bread(nbloqueabs, inodos) == FALLO) {
        perror("Error al leer el bloque de inodos");
        return FALLO;
    }

    // Calcular la posición del inodo dentro del bloque
    unsigned int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[posinodo];  // Copiar el inodo leído en la estructura proporcionada

    return EXITO;  // Usar una constante que represente el éxito de la operación
}


int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    //declaramos el superbloque
    struct superbloque SB;
    // Leer el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO) {
        perror("Error al leer el superbloque");
        return FALLO;  // Usar un código de error adecuado
    }
    if(SB.posPrimerInodoLibre==0){
        perror(RED, "no hay inodos disponibles");
        return FALLO;
    }
    unsigned int reservarInodo=SB.posPrimerInodoLibre;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    
    struct inodo inodos;
    

}


