#include "ficheros.h"

// NIVEL 5

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == -1) return FALLO;
    
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, "No hay permisos de escritura\n");
        return FALLO;
    }
    
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    unsigned int bytesEscritos = 0;
    
    unsigned char buf_bloque[BLOCKSIZE];
    unsigned int nbfisico;
    
    nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
    if (bread(nbfisico, buf_bloque) == -1) return FALLO;
    if (primerBL == ultimoBL) {
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == -1) return FALLO;
        bytesEscritos = nbytes;
    } else {
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == -1) return FALLO;
        bytesEscritos += BLOCKSIZE - desp1;
        
        for (unsigned int bl = primerBL + 1; bl < ultimoBL; bl++) {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            if (bwrite(nbfisico, buf_original + (bytesEscritos)) == -1) return FALLO;
            bytesEscritos += BLOCKSIZE;
        }
        
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (bread(nbfisico, buf_bloque) == -1) return FALLO;
        memcpy(buf_bloque, buf_original + bytesEscritos, desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == -1) return FALLO;
        bytesEscritos += desp2 + 1;
    }
    
    if (leer_inodo(ninodo, &inodo) == -1) return FALLO;
    if (offset + nbytes > inodo.tamEnBytesLog) inodo.tamEnBytesLog = offset + nbytes;
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == -1) return FALLO;
    
    return bytesEscritos;
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
    //int bytesLeidos = 0;
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

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        perror("Error al leer inodo");
        return FALLO;
    }

    // Comprobar si el tamaño a truncar es mayor que el tamaño actual
    if (nbytes > inodo.tamEnBytesLog) {
        fprintf(stderr, "El tamaño a truncar es mayor que el tamaño actual del fichero\n");
        return FALLO;
    }

    // Actualizar el tamaño lógico del fichero
    inodo.tamEnBytesLog = nbytes;

    // Actualizar ctime del inodo
    inodo.ctime = time(NULL);

    // Escribir el inodo actualizado
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        perror("Error al escribir el inodo actualizado");
        return FALLO;
    }

    return EXITO;
}