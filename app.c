#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>             // opendir
#include <sys/stat.h>           // file-stat nos da info. sobre el path
#include "list.h"

#define SLAVE_BIN_PATH "./slave"
/*
TODO BORRAR ESTO
pipefd   0     1
padre    0=====]    
         [=====0       hijo

argv[0] -> "slave.c"
argv[1] -> "/path/file.f"

Si se cierra un pipe, el que lee recibe EOF
*/




int main(int argc, char *argv[]){

    if (argc < 2) {
        printf("Debes enviar uno o mas archivos por argumento para comenzar la ejecucion.\n");
        return EXIT_FAILURE;
    }

        int total_slaves = argc / 2;
    init_slaves(total_slaves);

    return EXIT_SUCCESS;

}

/*
    Retorna un array de pipes, donde cada pipe[i] es un array de 2 enteros, siendo el primero el write end y el segundo el read end
    El indice del array es representativo de cada slave
*/
int** init_slaves(int total_slaves){
    int pipes[total_slaves][2];
    for(int i = 1; i < total_slaves; i++){
        
    }
}


    // POR AHORA HAGO UN SLAVE CADA 2 FILES!
    /*
    quiero implementar el proceso de levantar los slaves y de hacer select() para esperar a que esten listos
    > levanto (argc - 1) / 2 slaves
        > Esto implica levantar sus respectivos pipes ( CERRANDO LOS NECESARIOS! )
        > Supongo que hago un for y voy haciendo muchos fork, necesito cargar los PID en un array?
          quizas necesito mapear pid de los hijos a los pipes indicados. debo investigar como funciona select()!
        > Por ahora apenas me contesta un slave imprimo su output
          si funcionan bien los slaves, no deberia tener problema "interfaceando" con ellos
        > en el slave debo cerrar todos los pipes menos el indicado
          uso el array de todos los pipes, lo recorro y cierro todos
          ANTES de hacer el execve()
    */
/*
    Inicializa un slave, retornando un array de 2 enteros, 
    siendo el primero el write end y el segundo el read end del pipe asociado
*/
// TODO Pasar por parametro un array donde dejar los pipes???
int* init_slave(){
    int pipe_fd[2];

    if(pipe(pipe_fd) == -1){
        perror("Error al pipear");
        exit(EXIT_FAILURE);
    }

    switch(fork()){
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            char *const argv_slave[] = {SLAVE_BIN_PATH, NULL};
            execve(SLAVE_BIN_PATH, argv_slave, NULL);
            break;
        default:
            return getpid();
    }
}

/*
Recibe un array de path de files validos
Retorna una lista con una cola de files, ordenada por tamano 
*/
listADT get_files(char * exec_path, int total_files, char* files[]) {
    struct stat file_stat;
    char full_path[1024];
    char * file_path;                              // este directory stream queda abierto y se itera llamando a readdir() o rewinddir()
    listADT list = newList();                      // TODO Usar el metodo correcto
    
    for(int i = 1; i < total_files; i++) {         // Mientras queden dir-entrys para iterar...
        file_path = files[i];
        snprintf(full_path, sizeof(full_path), "%s/%s", exec_path, file_path);

        // Consigo la info de la entrada
        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
        }

        // Si es un DIR, lo salteo 
        if(S_ISDIR(file_stat.st_mode)){
            continue;
        }
        
        list = add(list, full_path, file_stat.st_size);     //tamano del archivo       
    }

    return list;
}