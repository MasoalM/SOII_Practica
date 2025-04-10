#include "directorios.h" // directorios.h?

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    char copia;
    strcpy(camino, copia);
    char *p = strchr(camino, '/');
    
    if(strcmp(p, camino)!=0){
        return FALLO;
    }

    
    char *inicial = strtok(p, "/");
    printf("INICIAL = %s", inicial);

}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){

}