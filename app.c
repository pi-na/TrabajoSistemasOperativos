#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>             // opendir
#include <sys/stat.h>           // file-stat nos da info. sobre el path
#include "list.h"

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
        printf("You must pass a file or a list of files as an argument.\n");
        return EXIT_FAILURE;
    }

    

}
/*
Recibe un array de path de files validos
Retorna una lista con los file-path cargados a una lista ordenada por tamano 
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

        // Si es un DIR, lo salteo  (la consigna dice que es innecesario)
        if(S_ISDIR(file_stat.st_mode)){
            continue;
        }
        
        list = add(list, full_path, file_stat.st_size);     //tamano del archivo       
    }

    return list;
}