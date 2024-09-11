#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>             // opendir
#include <sys/stat.h>           // file-stat nos da info. sobre el path
#include "list.h"

int main(int argc, char *argv[]){

    if (argc < 2) {
        printf("You must pass a file or a list of files as an argument.\n");
        return EXIT_FAILURE;
    }

    /*
    Iterate through the array
    For each element of the array, call my files function
    */
    for(int i = 1; i<argc; i++){
        FILE *file = fopen(argv[i], "r");
        
    }

}

/*
Recibe un array de path de files validos
Retorna una lista con los file-path cargados a una lista ordenada por tamano 
*/
listADT getFiless(char * exec_path, int total_files, char* files[]) {
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






void listDirectory(const char * path){
    
    DIR *dir = opendir(path);
    struct dirent *entry;
    struct stat file_stat;
    char full_path[1024];

    if(dir == NULL){  
        perror("Error opening directory");  //si no abre error
        return;
    }


    while((entry = readdir(dir)) != NULL){
         if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { //omito los "." y ".."
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
        }

        if(S_ISDIR(file_stat.st_mode)){ //chequeo si es un directorio o un archivo
            printf("d");
            tab(level + 1);
            printf("%s\n", entry->d_name);
            listDirectory(full_path, level + 1); //si es un directorio hago recursividad 
        }else{
            printf("f");
            tab(level + 1);
            printf("%s\n",entry->d_name); 
        }
    }

    closedir(dir);

    return;
}