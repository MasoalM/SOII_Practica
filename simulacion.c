// simulacion.c

#include "simulacion.h"

static volatile int acabados = 0;

void reaper(int signum) {
    pid_t ended;
    (void)signum;  // evitar warning unused
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

// Función para dormir en milisegundos, reanudando tras señal
void my_sleep(unsigned msec) {
    struct timespec req, rem;
    int err;
    req.tv_sec = msec / 1000;
    req.tv_nsec = (msec % 1000) * 1000000;
    while ((req.tv_sec != 0) || (req.tv_nsec != 0)) {
        if (nanosleep(&req, &rem) == 0)
            break;
        err = errno;
        if (err == EINTR) {
            req = rem;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Sintaxis: %s <disco>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *disco = argv[1];

    // Asociar señal SIGCHLD a enterrador
    signal(SIGCHLD, reaper);

    // Montar disco padre
    if (bmount(disco) == FALLO) {
        perror("Error montando dispositivo");
        exit(EXIT_FAILURE);
    }

    // Crear directorio simul_yyyymmddhhmmss
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char simul_dir[256];
    snprintf(simul_dir, sizeof(simul_dir),
             "/simul_%04d%02d%02d%02d%02d%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);

    printf("Directorio de simulación: %s\n", simul_dir);

    if (mi_creat(simul_dir, 7) == FALLO) {
        fprintf(stderr, "Error creando directorio simulación\n");
        if(bumount()==FALLO) return FALLO;
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUMPROCESOS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            break;
        }
        if (pid == 0) { // hijo
            // Montar dispositivo en hijo para evitar conflictos descriptor abierto compartido
            if (bmount(disco) == FALLO) {
                perror("Error montando disco hijo");
                exit(EXIT_FAILURE);
            }

            char proceso_dir[300];
            snprintf(proceso_dir, sizeof(proceso_dir), "%s/proceso_%d", simul_dir, getpid());

            if (mi_creat(proceso_dir, 7) == FALLO) {
                fprintf(stderr, "[Proceso %d] Error creando directorio proceso\n", getpid());
                if(bumount()==FALLO) return FALLO;
                exit(EXIT_FAILURE);
            }

            char fichero_prueba[350];
            snprintf(fichero_prueba, sizeof(fichero_prueba), "%s/prueba.dat", proceso_dir);

            if (mi_creat(fichero_prueba, 6) == FALLO) {  // permisos 6: lectura + escritura
                fprintf(stderr, "[Proceso %d] Error creando fichero prueba.dat\n", getpid());
                if(bumount()==FALLO) return FALLO;
                exit(EXIT_FAILURE);
            }

            srand(time(NULL) + getpid());

            struct REGISTRO registro;

            for (int nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) {
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;

                off_t offset = registro.nRegistro * sizeof(struct REGISTRO);

                if (mi_write(fichero_prueba, &registro, offset, sizeof(struct REGISTRO)) == FALLO) {
                    fprintf(stderr, "[Proceso %d] Error en escritura %d\n", getpid(), nescritura);
                    if(bumount()==FALLO) return FALLO;
                    exit(EXIT_FAILURE);
                }

                printf("[simulación.c → Escritura %d en %s]\n", nescritura, fichero_prueba);

                usleep(50000); // 0.05 segundos
            }

            printf("[Proceso %d: Completadas %d escrituras en %s]\n", getpid(), NUMESCRITURAS, fichero_prueba);

            if(bumount()==FALLO) return FALLO;

            exit(0);
        } else {
            // padre espera 0.15 seg antes de crear siguiente hijo
            usleep(150000);
        }
    }

    // Padre espera que terminen todos los hijos
    while (acabados < NUMPROCESOS) {
        pause();
    }

    if(bumount()==FALLO) return FALLO;

    return EXITO;
}
