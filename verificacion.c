#include "verificacion.h"

#define TAM_BUFFER 256

// Elimina el salto de línea final que devuelve asctime()
char *formatear_asctime(time_t fecha) {
    char *texto = asctime(localtime(&fecha));
    texto[strlen(texto) - 1] = '\0'; // quitar el '\n'
    return texto;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <directorio_simulación>\n", argv[0]);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, "Error al montar el dispositivo\n");
        return FALLO;
    }

    char *directorio_sim = argv[2];
    struct STAT st;
    if (mi_stat(directorio_sim, &st) == FALLO) {
        fprintf(stderr, "Error al obtener stat de %s\n", directorio_sim);
        if(bumount() == FALLO) return FALLO;
        return FALLO;
    }

    int numentradas = st.tamEnBytesLog / sizeof(struct entrada);
    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, "Error: numentradas != NUMPROCESOS\n");
        if(bumount() == FALLO) return FALLO;
        return FALLO;
    }

    fprintf(stdout, "dir_sim: %s\n", directorio_sim);
    fprintf(stdout, "numentradas: %d NUMPROCESOS: %d\n", numentradas, NUMPROCESOS);

    char path_informe[256];
    sprintf(path_informe, "%sinforme.txt", directorio_sim);
    if (mi_creat(path_informe, 6) == FALLO) {
        fprintf(stderr, "Error creando informe.txt\n");
        if(bumount() == FALLO) return FALLO;
        return FALLO;
    }

    struct entrada entradas[NUMPROCESOS];
    if (mi_read(directorio_sim, entradas, 0, sizeof(entradas)) < 0) {
        fprintf(stderr, "Error leyendo entradas\n");
        if(bumount() == FALLO) return FALLO;
        return FALLO;
    }

    int offset = 0;

    for (int i = 0; i < NUMPROCESOS; i++) {
        struct INFORMACION info = {0};

        char *nombre = entradas[i].nombre;
        char *pidstr = strchr(nombre, '_');
        if (!pidstr) continue;
        info.pid = atoi(pidstr + 1);

        char fichero_prueba[256];
        sprintf(fichero_prueba, "%s%s/prueba.dat", directorio_sim, nombre);

        int desplazamiento = 0;
        struct REGISTRO buffer[TAM_BUFFER];
        int leidos;

        while ((leidos = mi_read(fichero_prueba, buffer, desplazamiento, sizeof(buffer))) > 0) {
            int num_registros = leidos / sizeof(struct REGISTRO);

            for (int j = 0; j < num_registros; j++) {
                if (buffer[j].pid != info.pid) continue;

                if (info.nEscrituras == 0) {
                    info.PrimeraEscritura = buffer[j];
                    info.UltimaEscritura = buffer[j];
                    info.MenorPosicion = buffer[j];
                    info.MayorPosicion = buffer[j];
                } else {
                    if (buffer[j].nEscritura < info.PrimeraEscritura.nEscritura)
                        info.PrimeraEscritura = buffer[j];
                    if (buffer[j].nEscritura > info.UltimaEscritura.nEscritura)
                        info.UltimaEscritura = buffer[j];
                    if (buffer[j].nRegistro < info.MenorPosicion.nRegistro)
                        info.MenorPosicion = buffer[j];
                    if (buffer[j].nRegistro > info.MayorPosicion.nRegistro)
                        info.MayorPosicion = buffer[j];
                }

                info.nEscrituras++;
            }

            desplazamiento += leidos;
        }

        fprintf(stderr, "[%d) %d escrituras validadas en %s]\n", i + 1, info.nEscrituras, fichero_prueba);

        char salida[BLOCKSIZE];
        memset(salida, 0, BLOCKSIZE);

        snprintf(salida, BLOCKSIZE,
                 "PID: %d\nNumero de escrituras: %d\n"
                 "Primera Escritura\t%d\t%d\t%s\n"
                 "Ultima Escritura\t%d\t%d\t%s\n"
                 "Menor Posición\t\t%d\t%d\t%s\n"
                 "Mayor Posición\t\t%d\t%d\t%s\n\n",
                 info.pid, info.nEscrituras,
                 info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, formatear_asctime(info.PrimeraEscritura.fecha),
                 info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, formatear_asctime(info.UltimaEscritura.fecha),
                 info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, formatear_asctime(info.MenorPosicion.fecha),
                 info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, formatear_asctime(info.MayorPosicion.fecha));

        int escritos = mi_write(path_informe, salida, offset, strlen(salida));
        if (escritos < 0) {
            fprintf(stderr, "Error escribiendo en informe\n");
            if(bumount() == FALLO) return FALLO;
            return FALLO;
        }

        offset += escritos;
    }

    if (bumount() == FALLO) {
        fprintf(stderr, "Error al desmontar el dispositivo\n");
        return FALLO;
    }

    return EXITO;
}
