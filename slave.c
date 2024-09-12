#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MD5_BIN_PATH "/usr/bin/md5sum"
#define MD5_HASH_SIZE 32
#define BUFF_SIZE 1024

void process_md5_hash(char *file_path);
size_t read_md5(char *path, int pipe_fd[], char *buff);
void exec_md5(char *path, int pipe_fd[]);


int main(int argc, char *argv[]){
    if(argc > 1){
        for(int i = 1; i < argc; i++){
            process_md5_hash(argv[i]);
        }
        return EXIT_SUCCESS;
    } 

    int file_path_size = 0;
    int EOF_flag = 0;
    char *file_path = malloc(BUFF_SIZE);

    if(file_path == NULL){
        perror("Error al reservar memoria para file_path");
        return EXIT_FAILURE;
    }

    while(!EOF_flag){
        EOF_flag = read(STDIN_FILENO, file_path + file_path_size, 1) == 0;

        if(EOF_flag){            
            if(file_path_size == 0)                 // Si encontre un EOF, podria tener un ultimo file_path para procesar
                continue;
        } else if(file_path[file_path_size] != '\n'){
            file_path_size++;
            continue;
        }

        // Acabo de leer una string terminada en \n o una string + EOF!
        file_path_size++;
        file_path[file_path_size - 1] = '\0'; 
        process_md5_hash(file_path);
        file_path_size = 0;
    }

    free(file_path);
    return EXIT_SUCCESS;
}


/*
Recibe un file y escribe por salida estandar su hash MD5.
*/
void process_md5_hash(char *file_path){
    int pipe_fd[2]; 
    char md5_result[BUFF_SIZE];
    size_t md5_result_size;

    if(pipe(pipe_fd) == -1){
        perror("Error al pipear");
        free(file_path);
        exit(EXIT_FAILURE);
    }
    
    int forkpid;
    
    switch(forkpid = fork()){
        case(-1):
            perror("ERROR en fork(). Terminando la ejecucion...\n");
            free(file_path);
            exit(EXIT_FAILURE);
        case(0):
            exec_md5(file_path, pipe_fd);
        default:
            md5_result_size = read_md5(file_path, pipe_fd, md5_result);
    }

    write(STDOUT_FILENO, md5_result, md5_result_size);
}


/*
Espera que el proceso hijo MD5 termine y lee el hash resultante desde el pipe.
Deja el resultado en el buffer que recibe por parametro.
*/
size_t read_md5(char *path, int pipe_fd[], char *buff){
    int status;
    close(pipe_fd[1]);
    wait(&status);

    if(!WIFEXITED(status)){
        fprintf(stderr, "Hubo un error en el procesamiento del hash MD5 para el archivo %s\n", path);
        free(path);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(pipe_fd[0], buff, BUFF_SIZE);

    if (bytes_read == -1) {
        perror("Error al leer del pipe");
        free(path);
        exit(EXIT_FAILURE);
    }

    close(pipe_fd[0]);
    return (size_t)bytes_read;            
}


/*
Recibe un path y devuelve el md5 del archivo. 
Prepara los args y pipes para ejecutar el md5sum.
*/
void exec_md5(char *path, int pipe_fd[]){
    close(pipe_fd[0]);   
    close(1);           
    dup(pipe_fd[1]);     
    close(pipe_fd[1]);   
    char *const argv_md5[] = {MD5_BIN_PATH, path, NULL};
    execve(MD5_BIN_PATH, argv_md5, NULL);
}
