#include "directorios.h"

int main (int argc, char **argv) {
    //Revisamos que la entrada del comando sea correcta
    if (argc != 3) {
        fprintf(stderr, RED "Sintaxis: ./mi_ls <disco> </ruta>\n" WHITE);
        return FALLO;
    }
    //Montamos el sistema de ficheros
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error: No se pudo montar el dispositivo.\n" WHITE);
        return FALLO;
    }
    
    //Debemos crear el buffer que va a guardar la informacion que queremos ver por pantalla
    char buffer[TAMBUFFER];
    memset(buffer,0,sizeof(buffer));

    //Hacemos la llamada a mi_dir para recibir el directorio
    int entradas = mi_dir(argv[2],buffer);

    //Revisamos que haya algun tipo de entrada para mostrar y revisamos que no haya errores
    if (entradas >=0) {
        printf("Total: %d \n", entradas);
        printf("%s \n", buffer);
    } else {
        mostrar_error_buscar_entrada(entradas);
    }

    if(bumount()==FALLO) return FALLO;
    return EXITO;
}