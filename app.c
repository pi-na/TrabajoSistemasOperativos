#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <dirent.h>             // opendir
#include <sys/stat.h>           // file-stat nos da info. sobre el path
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>

#include "stack.h"
#include "list.h"

#define SLAVE_BIN_PATH "./slave"
#define READ_END 0
#define WRITE_END 1
#define MAX_SLAVES 8
#define IDEAL_LOAD 4



/*
COMUNICACION FULL-DUPLEX MASTER-SLAVE
Cuando hago pipe(pipe_fd[]), en pipe_fd[0] queda el FD del read end y en pipe_fd[1] queda el FD del write end

OBS: Si se cierra un pipe, el que lee recibe EOF
*/

//TODO unused!
typedef struct slave_data{
    int PID;
    int duplex_fd[2];
}slave_data;

static int compare_ascending(size_t a, size_t b) {
    return a - b;
}

int main(int argc, char* argv[]){
    if (argc < 2) {
        printf("Debes enviar uno o mas archivos por argumento para comenzar la ejecucion.\n");
        return EXIT_FAILURE;
    }

    int total_jobs = argc - 1;
    int total_slaves = fmin(MAX_SLAVES, total_jobs / IDEAL_LOAD);
    int** slaves_data = malloc(total_slaves * sizeof(slave_data));
    int slaves_data_size = 0;
    list_t* file_list = init_file_list(argc, argv);
    init_slaves(total_slaves, slaves_data, slaves_data_size, total_jobs, file_list);
    free_list(file_list);

    fd_set readfds;

    return EXIT_SUCCESS; 
}

//AHORA
/*
    Necesito saber el PID? Voy a averiguar si lo necesito para el select para decidir si 
    el slaves_data es un array de struct slave_data o si directo guardo los FD
    por lo que veo en el man de select, puedo directo usar los FD: recorro el array y le hago IS_SET a todo y si tengo algo para leer lo mando al buffer de shm
*/

/*
    quiero implementar el proceso de levantar los slaves y de hacer select() para esperar a que escriban en sus pipes
    > levanto (argc - 1) / 2 slaves
        > Esto implica levantar sus respectivos pipes ( CERRANDO LOS NECESARIOS! )
        > Supongo que hago un for y voy haciendo muchos fork, necesito cargar los PID en un array?
          quizas necesito mapear pid de los hijos a los pipes indicados. debo investigar como funciona select()!
        > Por ahora apenas me contesta un slave imprimo su output
          si funcionan bien los slaves, no deberia tener problema "interfaceando" con ellos
        > en el slave debo cerrar todos los pipes menos el indicado
          uso el array de todos los pipes, lo recorro y cierro todos
          ANTES de hacer el execve()
    
    PROFE DICE Lo que tenes que hacer  cuando select(2) retorna, es iterar sobre cada fd_set (readfds y writefds) llamando a FD_ISSET() para cada fd que te interesa    
*/

/*
    Retorna un array de pipes, donde cada pipe[i] es un array de 2 enteros, siendo el primero el write end y el segundo el read end
    El indice del array es representativo de cada slave
*/
void init_slaves(int total_slaves, int** slaves_data, int slaves_data_size, int total_jobs, list_t* file_list){
    for(int i = 0; i < total_slaves; i++){
        while(init_slave(slaves_data, slaves_data_size, i) != 0);
    }
    
    iterator_t* it = list_iterator(file_list);
    int i = 0;
    while(iterator_has_next(it)) {
        char* path = sprintf("%s\n", iterator_next(it));
        write(slaves_data[i][WRITE_END], path, strlen(path));
        i = (i + 1) % total_slaves;
    }
    free_iterator(it);
}


/*
    Inicializa un slave, retornando un array de 2 enteros.
    Levanta 2 pipes. Retorna 0 si todo sale bien, -1 si hubo error
    Llena un array de 2 enteros siendo el primero el FD del write end del pipe n1 y el segundo el FD del read end del pipe n2
*/
// TODO atrapar errores
int init_slave(int** slaves_data, int slaves_data_size, int new_slave_index){
    int pipe_fd_master_to_slave[2];
    int pipe_fd_slave_to_master[2];

    if(pipe(pipe_fd_master_to_slave) == -1 || pipe(pipe_fd_slave_to_master) == -1) {
        perror("Error al pipear");
        return -1;
    }

    switch(fork()) {
        case -1:
            perror("fork");
            return -1;
        case 0:
            close(pipe_fd_master_to_slave[WRITE_END]);                 // NO VOY A USAR el write end del pipe master->slave
            close(pipe_fd_slave_to_master[READ_END]);                  // NO VOY A USAR el read end del pipe slave->master
            dup2(pipe_fd_master_to_slave[READ_END], STDIN_FILENO);
            dup2(pipe_fd_slave_to_master[WRITE_END], STDOUT_FILENO);
            close(pipe_fd_master_to_slave[READ_END]);
            close(pipe_fd_slave_to_master[WRITE_END]);
            
            for(int i = 0; i < slaves_data_size; i++) {
                if(i != new_slave_index) {
                    close(slaves_data[i][READ_END]);
                    close(slaves_data[i][WRITE_END]);
                }
            }
            
            char* argv_slave[] = {SLAVE_BIN_PATH, NULL};
            execve(SLAVE_BIN_PATH, argv_slave, NULL);
            break;
        default:
            close(pipe_fd_master_to_slave[READ_END]);
            close(pipe_fd_slave_to_master[WRITE_END]);

            slaves_data[new_slave_index][READ_END] = pipe_fd_slave_to_master[READ_END];
            slaves_data[new_slave_index][WRITE_END] = pipe_fd_master_to_slave[WRITE_END];
            
            // COMO FUNCIONA ESTE GETPID??? ADEMAS, NOS SIRVE DE ALGO???            
            return getpid();
    }
}



/*
Recibe un array de path de files validos
Retorna una lista con una cola de files, ordenada por tamano 
*/
list_t* init_file_list(int argc, char* argv[]) {
    struct stat file_stat;
    char full_path[1024];
    char* file_path;                              // este directory stream queda abierto y se itera llamando a readdir() o rewinddir()
    char* current_directory = argv[0];
    list_t* list = new_list();
    
    for(int i = 1; i < argc; i++) {         // Mientras queden dir-entrys para iterar...
        file_path = argv[i];
        snprintf(full_path, sizeof(full_path), "%s/%s", current_directory, file_path);

        // Consigo la info de la entrada
        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
        }

        // Si es un DIR, lo salteo 
        if(S_ISDIR(file_stat.st_mode)){
            continue;
        }
        
        add(list, full_path, file_stat.st_size);     //tamano del archivo       
    }

    return list;
}



/*
Recibe un array de path de files validos
Retorna una lista con una cola de files, ordenada por tamano 
*/
stack_adt init_file_stack(int argc, char* argv[]) {
    struct stat file_stat;
    char full_path[1024];
    char * file_path;                              // este directory stream queda abierto y se itera llamando a readdir() o rewinddir()
    char * current_directory = argv[0];
    stack_adt stack = new_stack(compare_ascending);                  
    
    for(int i = 1; i < argc; i++) {         // Mientras queden dir-entrys para iterar...
        file_path = argv[i];
        snprintf(full_path, sizeof(full_path), "%s/%s", current_directory, file_path);

        // Consigo la info de la entrada
        if (stat(full_path, &file_stat) == -1) {
            perror("stat");
        }

        // Si es un DIR, lo salteo 
        if(S_ISDIR(file_stat.st_mode)){
            continue;
        }
        
        stack = add(stack, full_path, file_stat.st_size);     //tamano del archivo       
    }

    return stack;
}
