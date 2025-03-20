#include "ficheros.h"

// NIVEL 5

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {

    //paso 1, iniciar inodo y comprobar permisos
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);

    if ((inodo.permisos & 2) != 2){
        fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
        return FALLO;
    }

    //paso 2, calcular distancias
    unsigned int primerBL = offset/BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    unsigned int ky = 0;
    //casos
    if(primerBL == ultimoBL) {

        //obtenemos algo
        unsigned int BF = traducir_bloque_inodo(ninodo, primerBL, 1);
        if(BF == FALLO){
            perror(RED "ERROR");
            return FALLO;
        }

        unsigned char bufferBloque[BLOCKSIZE];
        if(bread(BF, bufferBloque) == FALLO){
            perror(RED "ERROR");
            return FALLO;        
        } 
        memcpy(bufferBloque + desp1, buf_original, nbytes);
        bwrite(BF, bufferBloque);
    }else{

        //primer bloque lógico
        //obtenemos algo
        unsigned int BF = traducir_bloque_inodo(ninodo, primerBL, 1);
        if(BF == FALLO){
            perror(RED "ERROR");
            return FALLO;
        }

        unsigned char bufferBloque[BLOCKSIZE];
        if(bread(BF, bufferBloque) == FALLO){
            perror(RED "ERROR");
            return FALLO;        
        } 

        memcpy(bufferBloque + desp1, buf_original, (BLOCKSIZE - desp1));
        bwrite(BF, bufferBloque);

        //Bloques intermedios
        for(int bl=BF+1; (bl - BF) < (primerBL - ultimoBL); bl++){
            bwrite(BF + ( bl - BF ) + 1, buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1)*BLOCKSIZE);
        }

        //bloque final
        BF = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if(BF == FALLO){
            perror(RED "ERROR");
            return FALLO;
        }

        if(bread(BF, bufferBloque) == FALLO){
            perror(RED "ERROR");
            return FALLO;        
        }

        memcpy(bufferBloque, buf_original + (nbytes-(desp2+1)), desp2+1);
        if(bwrite(BF, bufferBloque) == FALLO) return FALLO;
        ky=1;
    }

    //actualizar metainformación
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    if(ky=1){
        if(){

        }
        inodo.tamEnBytesLog
    }

}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {

    
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        perror("Error al leer inodo");
        return FALLO;
    }

    // Comprobar permisos de lectura
    if ((inodo.permisos & 4) != 4) {
        fprintf(stderr, "No hay permisos de lectura\n");
        return FALLO;
    }

    // Ajustar nbytes si es necesario para no leer más allá del tamaño del fichero
    if (offset >= inodo.tamEnBytesLog) {
        return 0;  // No podemos leer nada
    }
    if ((offset + nbytes) > inodo.tamEnBytesLog) {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    // Calcular el primer y último bloque lógico donde se leerá
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    char buf_bloque[BLOCKSIZE];
    int bytesLeidos = 0;
    int totalLeidos = 0;

    for (unsigned int bl = primerBL; bl <= ultimoBL; bl++) {
        int nbfisico = traducir_bloque_inodo(ninodo, bl, 0);  // No reservar bloques
        if (nbfisico == -1) {
            // Si no hay bloque físico, asumimos que el bloque está lleno de ceros
            memset(buf_bloque, 0, BLOCKSIZE);
        } else {
            if (bread(nbfisico, buf_bloque) == FALLO) {
                perror("Error al leer el bloque físico");
                return FALLO;
            }
        }

        // Copiar los datos del bloque a buf_original
        int despInicio = (bl == primerBL) ? desp1 : 0;
        int despFin = (bl == ultimoBL) ? desp2 : BLOCKSIZE - 1;
        int tam = despFin - despInicio + 1;

        memcpy(buf_original + totalLeidos, buf_bloque + despInicio, tam);
        totalLeidos += tam;
    }

    // Actualizar el atime del inodo, ya que se ha accedido a los datos
    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        perror("Error al actualizar el inodo");
        return FALLO;
    }

    return totalLeidos;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    struct inodo in;
    // Leer inodo
    if(leer_inodo(ninodo, &in) == -1) return FALLO;
    
    // Pasar los datos del inodo al STAT
    p_stat->atime = in.atime;
    p_stat->btime = in.btime;
    p_stat->ctime = in.ctime;
    p_stat->mtime = in.mtime;
    p_stat->nlinks = in.nlinks;
    p_stat->numBloquesOcupados = in.numBloquesOcupados;
    p_stat->permisos = in.permisos;
    p_stat->tamEnBytesLog = in.tamEnBytesLog;
    p_stat->tipo = in.tipo;

    // Fin del método
    return EXITO;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        perror("Error al leer inodo");
        return FALLO;
    }

    // Cambiar permisos
    inodo.permisos = permisos;

    // Actualizar ctime del inodo para reflejar el cambio de permisos
    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        perror("Error al escribir el inodo actualizado");
        return FALLO;
    }

    return EXITO;

}