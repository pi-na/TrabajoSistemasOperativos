#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define MD5_EXEC "/usr/bin/md5sum"
#define MD5_LENGTH 32

//valido que me hayan enviado un path
//fork -> exec MD5:
//      - abro pipe
//      - cierro las entradas q quedan duplicados
//      - le paso como args el path
int main(int argc, char *argv[]){

    // TODO: CHEQUEAR QUE ME PASARON UN FILE (VER EJ GUIA)
    if (argc < 2) {
        printf("Debes pasar un archivo como argumento.\n");
        return EXIT_FAILURE;
    }

    int pipefd[2];                      //pipe() deja en pipefd[0] y pipefd[1] los FD del pipe
    if(pipe(pipefd) == -1){
        perror("Error al pipear");
        return EXIT_FAILURE;
    }

    switch(fork()){
        case(-1):
            printf("FALLO EL FORK!!\n");
            return EXIT_FAILURE;

        case(0):
            close(pipefd[0]);   // CIERRO FD 0 DEL PIPE (LO USARA EL PADRE)
            close(1);           // CIERRO FD 1 PARA QUE NO TENGA STDOUT
            dup(pipefd[1]);     // DUPLICA EL FD DEL EXTREMO ABIERTO DEL PIPE Y LO ASIGNA AL FD MAS CHICO: EL 1
            close(pipefd[1]);   // Cerramos el FD original de escritura del pipe, ya está duplicado
            char *const argvMD5[] = {MD5_EXEC, argv[1], NULL};
            execve(MD5_EXEC, argvMD5, NULL);
            break;

        default:
            close(pipefd[1]);   // CIERRO FD 1 DEL PIPE (LO USARA EL HIJO)
            char md5_hash[MD5_LENGTH];
            read(pipefd[0], md5_hash, MD5_LENGTH);
            printf("El MD5 de %s es %s\n", argv[1], md5_hash);
            close(pipefd[0]);   // Cerramos el extremo de lectura después de leer
            break;
    }

    return EXIT_SUCCESS;
}

/*
pipefd   0     1
padre    0=====]    
         [=====0       hijo

argv[0] -> "slave.c"
argv[1] -> "/path/file.f"
*/




