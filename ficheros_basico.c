#include "ficheros_basico.h"
#include <limits.h>


// Nombre: potencia
// Utilidad: Funcion que dada una base y un exponente, te calcula el valor del número resultante de la operación
// Parámetros de entrada: base y exponente (base)^exponente
// Salida: devuelve el valor de la operación (base)^exponente
// Dónde se utiliza: initMB
int potencia(int base, int exponente) {
    int resultado = 1;
    for (int i = 0; i < exponente; i++) {
        resultado *= base;
    }
    return resultado;
}


// Nombre: tamMB
// Utilidad: Función que calcula el tamaño del mapa de bits
// Parámetros de entrada: nbloque (número de bloque)
// Salida: Devuelve el tamaño del mapa de bits
// Dónde se utiliza: initSB e initMB
int tamMB(unsigned int nbloques) {
    int resultado = (nbloques / 8) / BLOCKSIZE;
    if((nbloques / 8) % BLOCKSIZE > 0) return (resultado+1);
    return resultado;
}


// Nombre: tamAI
// Utilidad: Función que calcula el tamaño del array de inodos
// Parámetros de entrada: ninodos (número de inodos)
// Salida: Devuelve el tamaño del array de inodos
// Dónde se utiliza: Main de initSB e initMB
int tamAI(unsigned int ninodos) {
    int resultado = (ninodos * INODOSIZE) / BLOCKSIZE;
    if((ninodos * INODOSIZE) % BLOCKSIZE) return (resultado+1);
    return resultado;
}


// Nombre: initSB
// Utilidad: Función que inicializa el superbloque
// Parámetros de entrada: nbloque (número de bloque), ninodos (número de inodos)
// Salida: Devuelve -1 en caso de fallo, 0 en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
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


// Nombre: initMB
// Utilidad: Función que inicializa el mapa de bits
// Parámetros de entrada: NONE
// Salida: -1 en caso de fallo, 0 en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
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


// Nombre: initAI
// Utilidad: Función que inicializa el array de inodos
// Parámetros de entrada: NONE
// Salida: -1 en caso de fallo, 0 en caso de éxito
// Dónde se utiliza: Main de mi_mkfs
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


//
//INICIO NIVEL 3
//


// Nombre: ecribir_bit
// Utilidad: Función que escribe el valor del bit (0 o 1) en la posición que representa el nbloque en el MB
// Parámetros de entrada: nbloque (número de bloque), bit (valor a escribir (0 o 1))
// Salida: -1 en caso de fallo, 0 en caso de éxito
// Dónde se utiliza: Reservar bloque, liberar bloque
int escribir_bit(unsigned int nbloque, unsigned int bit){

    //declaraciones
    struct superbloque SB; //superbloque
    if(bread(posSB, &SB) == FALLO) return FALLO;
    unsigned int posByte = nbloque/8;
    unsigned int posBit = nbloque%8;
    unsigned int nbloqueMB = posByte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    //tratamiento
    posByte = posByte % BLOCKSIZE;
    if(bread(SB.posPrimerBloqueMB,bufferMB) == FALLO) return FALLO;
    unsigned char mascara = 128; // 10000000
    mascara >>= posBit; // desplazamiento de bits a la derecha
    if(bit == 1){
        bufferMB[posByte] |= mascara; //poner a 1 el bit indicado 
    } else {
        bufferMB[posByte] &= ~mascara; //poner a 0 el bit indicado
    }

    //resultado
    if(bwrite(nbloqueabs, bufferMB) == FALLO) return FALLO;
    return EXITO;
}


// Nombre: leer_bit
// Utilidad: Función que lee el valor del bit en la posición del MB que es representado por el nbloque (número de bloque)
// Parámetros de entrada: nbloque (número de bloque)
// Salida: mascara (unsigned char) con el valor del bit a leer del MB
// Dónde se utiliza: leer_sf
char leer_bit(unsigned int nbloque){

    //declaraciones
    struct superbloque SB;
    if(bread(posSB, &SB) == FALLO) return FALLO;
    unsigned int posByte = nbloque / 8;
    unsigned int posBit = nbloque % 8;
    unsigned int nbloqueMB = posByte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    //tratamiento
    if(bread(nbloqueabs, bufferMB) == FALLO) return FALLO;
    posByte = posByte % BLOCKSIZE;
    unsigned char mascara = 128; // 10000000
    mascara >>= posBit;          // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posByte]; // operador AND para bits
    mascara >>= (7 - posBit);     // desplazamiento de bits a la derecha 
                                  // para dejar el 0 o 1 en el extremo derecho y leerlo en decimal

    //devolución de resultado
    return mascara;
}


// Nombre: reservar bloque
// Utilidad: Función que busca el primer bit del MB a 0, lo pone a 1, y devuelve su posición
// Parámetros de entrada: NONE
// Salida: -1 en caso de fallo, nBloqueFisico (posición del primer bloque libre)
// Dónde se utiliza: Reservar bloque, liberar bloque
int reservar_bloque(){

    //paso 1
    //declaramos variables
    struct superbloque SB;
    //gestión de errores
    if(bread(posSB, &SB) == FALLO) return FALLO;
    if(SB.cantBloquesLibres == 0) return FALLO;

    //vamos a gestionar el buffer
    unsigned int nbloqueMB = 0;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];

    memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con bits a 1
    //hacemos la primera lectura para entrar en el bucle
    if(bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO) return FALLO;

    //bucle de búsqueda del bloque con un byte libre
    while(memcmp(bufferAux, bufferMB, BLOCKSIZE) == 0){
        nbloqueMB++;
        if(bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO) return FALLO;
    }

    //paso 2
    //buscamos el byte con el bit libre
    unsigned int posbyte = 0;
    for(; bufferMB[posbyte] == 255; posbyte++);

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
    int nBloqueFisico = (nbloqueMB * BLOCKSIZE * 8) + (posbyte * 8) + posbit;
    SB.cantBloquesLibres--;
    //guardamos los datos, y gestionsmos la información obtenida
    if(escribir_bit(nBloqueFisico, 1) == FALLO) return FALLO;
    
    if(bwrite(posSB, &SB) == FALLO) return FALLO;
    memset(bufferAux, 0, BLOCKSIZE);
    if(bwrite(nBloqueFisico, bufferAux) == FALLO) return FALLO;
    return nBloqueFisico;
}

//TRATAMIENTO DE ERRORES
int liberar_bloque(unsigned int nbloque) {
    //iniciamos el superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == FALLO) return FALLO;
    //ponemos a 0 el bloque que nos dicen (lo liberamos)
    if(escribir_bit(nbloque, 0) == FALLO) return FALLO;
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
    if(bread(posSB, &SB) == FALLO) return FALLO;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    unsigned int nbloqueabs = nbloqueAI + SB.posPrimerBloqueAI;
    if (bread(nbloqueabs, inodos) == FALLO) return FALLO;
    unsigned int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[posinodo] = *inodo;
    if (bwrite(nbloqueabs, inodos) == FALLO) {
        perror(RED "Error al escribir el inodo");
        return FALLO;
    }
    
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    // Leer el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO) {
        perror(RED "Error al leer el superbloque");
        return FALLO;  // Usar un código de error adecuado
    }

    // Calcular el número de bloque donde se encuentra el inodo
    unsigned int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueAI + nbloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // Leer el bloque de inodos que contiene el inodo deseado
    if (bread(nbloqueabs, inodos) == FALLO) {
        perror(RED "Error al leer el bloque de inodos");
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
        perror(RED "Error al leer el superbloque");
        return FALLO;  // Usar un código de error adecuado
    }

    //comprobamos si hay inodos libres
    if(SB.posPrimerInodoLibre == UINT_MAX) {
        perror(RED "no hay inodos disponibles");
        return FALLO;
    }

    //guardamos la posición
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    //inicializaciones del inodo
    struct inodo inodoR;
    inodoR.permisos = permisos;
    inodoR.tipo = tipo;
    inodoR.nlinks = 1;
    inodoR.tamEnBytesLog = 0;
    inodoR.atime = time(NULL);
    //printf("A TIME OG:  %t")
    inodoR.btime = time(NULL);
    
    inodoR.ctime = time(NULL);
    
    inodoR.mtime = time(NULL);
    //printf("M time: %d", inodoR.mtime);
    inodoR.numBloquesOcupados = 0;;    memset(inodoR.punterosDirectos, 0, sizeof(inodoR.punterosDirectos));
    memset(inodoR.punterosIndirectos, 0, sizeof(inodoR.punterosIndirectos));

    // Leer el inodo actual para obtener el enlace al siguiente inodo libre
    struct inodo tempInodo;
    if (leer_inodo(posInodoReservado, &tempInodo) == FALLO) {
        perror("Error al leer el inodo para actualizar la lista enlazada");
        return FALLO;
    }
    //inodoR.punterosDirectos[0]=tempInodo.punterosDirectos[0];

    // Actualizar el primer inodo libre al siguiente en la lista
    SB.posPrimerInodoLibre = tempInodo.punterosDirectos[0];  // Suponiendo que el primer puntero directo guarda el siguiente inodo libre


    //Reservar el primer inodo libre
    if(escribir_inodo(posInodoReservado, &inodoR)) {
        perror(RED "Error al escribir el inodo");
        return FALLO;
    }    
    SB.cantInodosLibres--;

    if (bwrite(posSB, &SB) == FALLO) {
        perror(RED "Error al escribir el superbloque");
        return FALLO;
    }

    return posInodoReservado;
}


//
// NIVEL 4
//


// Nombre: obtener_nRangoBL
// Utilidad: Función que busca el rango en el cual se encuentra el bloque lógico
// Parámetros de entrada: inodo (inodo el cual apunta al bloque a hallar), nblogico (posición lógica del bloque),
//                        *ptr (puntero que apuntará a la posición que apunta el inodo en el rango deseado)
// Salida: 0, en el rango más bajo; 1, si nblogico<268; 2, si nblogico<65.804; 3, si nblogico<16.843.020
// Dónde se utiliza: ///
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) {
    //tratamiento
    if(nblogico < DIRECTOS) { // < 12
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    } else if(nblogico < INDIRECTOS0) { // < 268    
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    } else if(nblogico < INDIRECTOS1) { // < 65.804
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    } else if(nblogico < INDIRECTOS2) { // < 16.843.020
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;            
        perror(RED "Bloque lógico fuera de rango");       
        return FALLO;   
    }
}


// Nombre: obtener_indice
// Utilidad: Función que obtiene el índice del bloque de los punteros
// Parámetros de entrada: nblogico (posición lógica del bloque), nivel_punteros (nivel de los punteros en 
//                        el cual se encuentra)
// Salida: nblogico, si nblogico<directos;
//         nblogico-DIRECTOS, si nblogico<Indirectos0;
//         (nblogico-Indirectos0)/NPUNTEROS, si (nblogico<indirectos1)&&nivel_punteros==2;
//         (nblogico-indirectos0)%npunteross, si (nblogico<indirectos1)&&nivel_punteros==1;
//         (nblogico-indirectos1)/(npunteros*npunteros), si (nblogico<indirectos2)&&(nivel_punteros==3);
//         ((nblogico-indirectos1)%(npunteros*npunteros))/npunteros, si (nblogico<indirectos2)&&(nivel_punteros==2);
//         ((nblogico-indirectos1)%(npunteros*npunteros))%npunteros, si (nblogico<indirectos2)&&(nivel_punteros==1);
//         -1 en caso de fallo.
// Dónde se utiliza: ///
int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) return nblogico;
    else if (nblogico < INDIRECTOS0) return nblogico - DIRECTOS;
    else if (nblogico < INDIRECTOS1) {
        if (nivel_punteros == 2) return (nblogico - INDIRECTOS0) / NPUNTEROS;
        else if (nivel_punteros == 1) return (nblogico - INDIRECTOS0) % NPUNTEROS;
    } else if (nblogico < INDIRECTOS2) {
        if (nivel_punteros == 3) return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        else if (nivel_punteros == 2) return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        else if (nivel_punteros == 1) return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
    }
    return FALLO; // En caso de un valor fuera de rango
}


// Nombre: traducir_bloque_inodo
// Utilidad: Función que obtiene el número de bloque físico a través del bloque lógico
// Parámetros de entrada: ninodo (número del inodo), nblogico (posición lógica del bloque),
//                        reservar
// Salida: 0, en el rango más bajo; 1, si nblogico<268; 2, si nblogico<65.804; 3, si nblogico<16.843.020
// Dónde se utiliza: ///
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar) {
    unsigned int ptr = 0, ptr_ant = 0, salvar_inodo = 0, indice = 0;
    int nRangoBL, nivel_punteros;
    unsigned int buffer[NPUNTEROS];
    struct inodo inodo;

    // Leer inodo
    leer_inodo(ninodo, &inodo);
    
    // Obtener el rango del bloque lógico
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    nivel_punteros = nRangoBL; // Nivel más alto de punteros indirectos

    while (nivel_punteros > 0) {  // Iterar sobre los niveles de punteros indirectos
        if (ptr == 0) { // No hay bloques de punteros asignados
            if (reservar == 0) return -1; // Bloque inexistente

            // Reservar bloque de punteros y crear enlaces desde el inodo hasta los bloques de datos
            ptr = reservar_bloque();
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL);
            salvar_inodo = 1;

            if (nivel_punteros == nRangoBL) {  // Bloque cuelga directamente del inodo
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
            } else {  // Bloque cuelga de otro bloque de punteros
                buffer[indice] = ptr;
                if (bwrite(ptr_ant, buffer) == FALLO) { // Guardar en el dispositivo
                    perror(RED "Error al guardar en el dispositivo");
                    return FALLO;
                }
            }
            memset(buffer, 0, BLOCKSIZE); // Inicializar el buffer a 0
        } else {
            if (bread(ptr, buffer) == FALLO) return FALLO; // Leer el bloque de punteros existente
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr; // Guardar puntero actual
        ptr = buffer[indice]; // Desplazarse al siguiente nivel
        nivel_punteros--;
    }

    // Al salir del bucle estamos en el nivel de datos
    if (ptr == 0) {  // No existe bloque de datos
        if (reservar == 0) return -1; // Error de lectura, bloque inexistente

        ptr = reservar_bloque();
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL);
        salvar_inodo = 1;

        if (nRangoBL == 0) {  // Puntero directo
            inodo.punterosDirectos[nblogico] = ptr;
        } else {
            buffer[indice] = ptr;
            if (bwrite(ptr_ant, buffer) == FALLO) { // Guardar en el dispositivo
                perror(RED "Error al guardar en el dispositivo");
                return FALLO;
            }
        }
    }

    // Guardar el inodo si hubo cambios
    if (salvar_inodo) {
        if(escribir_inodo(ninodo, &inodo)==FALLO) {
            perror(RED "Error al escribir el inodo");
            return FALLO;
        }
    }

    return ptr;  // Retorna el número de bloque físico correspondiente al bloque lógico
}

