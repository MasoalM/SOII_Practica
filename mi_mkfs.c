#include "bloques.h"
#include "bloques.c"
#include <string.h>

int main(int argc, char **argv) {
    unsigned char *buffer [BLOCKSIZE];
    memset(buffer, '\0', sizeof(buffer));

    if(strcmp(argv[0], "./mi_mkfs") != 0) {
        perror("Error de sintaxis");
        return FALLO;
    }
    bmount(argv[1]);
    
    int num_bloques = atoi(argv[2]);
    for(int i = 0; i < num_bloques; i++) {
        bwrite(i, buffer);
    }

    bumount();

    return EXITO;
}