/*
AUTORES
Joan Matemalas Rosselló
Marcos Socías Alberto
Sergi Villalonga Gamundí
*/


#ifndef SIMULACION_H
#define SIMULACION_H

#include "directorios.h"  // Para mi_creat, mi_write, mi_mkdir, bmount, bumount
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>

#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define REGMAX 500000

struct REGISTRO {
    time_t fecha;
    pid_t pid;
    int nEscritura;
    int nRegistro;
};

#endif
