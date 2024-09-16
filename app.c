#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <dirent.h>            
#include <sys/stat.h>          
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>      
#include <semaphore.h>     

#include "list.h"

#define SLAVE_BIN_PATH "./slave"
#define SHM_NAME "/shm_buff"
#define SHM_SEM_NAME "/shm_sem"
#define SHM_CLOSED "/end"
#define READ_END 0
#define WRITE_END 1
#define MAX_SLAVES 8
#define IDEAL_LOAD 4
#define MAX_RETRIES 5
#define BUFF_SIZE 1024
#define PID_DIGITS 5
#define SLAVE_OUTPUT_DIVIDER 2
#define MD5_HASH_SIZE 32
#define RESULT_FILE_PATH "/shared/result.txt"

#define GET_OUTOUT_SIZE(file_path_len) \
    ((PID_DIGITS) + (MD5_HASH_SIZE) + (file_path_len) + 2 * SLAVE_OUTPUT_DIVIDER + 1)

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

static int compare_ascending(size_t a, size_t b);
void init_slaves(int total_slaves, int** slaves_duplex_fd, int slaves_duplex_fd_size, int total_jobs, list_adt file_list);
int init_slave(int** slaves_duplex_fd, int slaves_duplex_fd_size, int new_slave_index);
list_adt init_file_list(int argc, char* argv[], size_t* total_output_size);
int init_shm(char **shm_map_address, size_t shm_size);
void init_result_file(int processed_jobs, char *shm_map_address, size_t shm_size);

static int compare_ascending(size_t a, size_t b) {
    return a - b;
}

int main(int argc, char* argv[]){
    if (argc < 2) {
        printf("Debes enviar uno o mas archivos por argumento para comenzar la ejecucion.\n");
        return EXIT_FAILURE;
    }

    int total_jobs = argc - 1;
    int total_slaves = MIN(MAX_SLAVES, total_jobs / IDEAL_LOAD + 1);
    int** slaves_duplex_fd = calloc(total_slaves, sizeof(int*));
    if(slaves_duplex_fd == NULL){
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < total_slaves; i++){
        slaves_duplex_fd[i] = malloc(2 * sizeof(int));
        if(slaves_duplex_fd[i] == NULL){
            perror("memory allocation failed");
            // Liberar la memoria ya asignada antes de salir
            for(int j = 0; j < i; j++){
                free(slaves_duplex_fd[j]);
            }
            free(slaves_duplex_fd);
            exit(EXIT_FAILURE);
        }
    }

    fd_set rfds;
    int files_processed = 0;
    int slaves_duplex_fd_size = 0;
    size_t total_output_size = 0;
    //TODO si no se usa, eliminar
    char * shm_map_address;
    char file_path[BUFF_SIZE];
    char buff[BUFF_SIZE];
    char processed_file_result[2 * BUFF_SIZE];

    sem_t * shm_sem = sem_open(SHM_SEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
    if(shm_sem == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    list_adt file_list = init_file_list(argc, argv, &total_output_size);
    init_slaves(total_slaves, slaves_duplex_fd, slaves_duplex_fd_size, total_jobs, file_list);
    int shm_fd = init_shm(&shm_map_address, total_output_size);

    printf("%s\n%zu\n%s\n", SHM_NAME, total_output_size, SHM_SEM_NAME);
    fflush(stdout);

    while(files_processed < total_jobs){
        FD_ZERO(&rfds);
        int nfds = 0;
        for(int i = 0; i < total_slaves; i++){
            if(slaves_duplex_fd[i][READ_END] != -1){
                FD_SET(slaves_duplex_fd[i][READ_END], &rfds);
                nfds = MAX(nfds, slaves_duplex_fd[i][READ_END]);
            }
        }

        if(select(nfds + 1, &rfds, NULL, NULL, NULL) == -1){
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        for(int i = 0; i < total_slaves; i++){
            int* current_slave_fd = slaves_duplex_fd[i];
            if(current_slave_fd[READ_END] != -1 && FD_ISSET(current_slave_fd[READ_END], &rfds)){
                int bytes_read;
                if((bytes_read = read(current_slave_fd[READ_END], buff, BUFF_SIZE)) > 0){
                    sprintf(processed_file_result, "%s\n", buff);
                    write(shm_fd, processed_file_result, strlen(processed_file_result));
                    sem_post(shm_sem);
                    files_processed++;
                    if(has_next(file_list)){
                        sprintf(file_path,"%s\n", next(file_list));
                        write(current_slave_fd[WRITE_END], file_path, strlen(file_path));
                    } else {
                        close(current_slave_fd[READ_END]);
                        close(current_slave_fd[WRITE_END]);
                        current_slave_fd[READ_END] = -1;
                        current_slave_fd[WRITE_END] = -1;
                    }
                }else{
                    close(current_slave_fd[READ_END]);
                    close(current_slave_fd[WRITE_END]);
                    perror("Error al leer del slave. Fue ejecutado...");
                }
            }
        }     
    }
    init_result_file(files_processed, shm_map_address, total_output_size);

    write(shm_fd, SHM_CLOSED, strlen(SHM_CLOSED));
    close(shm_fd);
    sem_post(shm_sem);
    //TODO DUDOSO PUEDE CAUSAR FALLOS
    sem_close(shm_sem);

    for(int i = 0; i < total_slaves; i++){
        free(slaves_duplex_fd[i]);
    }

    free(slaves_duplex_fd);
    free_list(file_list);
    return EXIT_SUCCESS; 
}

//AHORA
/*
    Necesito saber el PID? Voy a averiguar si lo necesito para el select para decidir si 
    el slaves_duplex_fd es un array de struct slave_data o si directo guardo los FD
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
void init_slaves(int total_slaves, int** slaves_duplex_fd, int slaves_duplex_fd_size, int total_jobs, list_adt file_list){
    for(int i = 0; i < total_slaves; i++){
        int retries = 0;
        while(init_slave(slaves_duplex_fd, slaves_duplex_fd_size, i) && 
              retries++ < MAX_RETRIES);
        if(retries == MAX_RETRIES){
            fprintf(stderr, "Error al inicializar el slave %d\n", i);
            exit(EXIT_FAILURE);
        }
    }
    
    to_begin(file_list);
    for(int i = 0; has_next(file_list) && i < total_slaves; i++){
        char path[BUFF_SIZE];
        sprintf(path,"%s\n", next(file_list));
        write(slaves_duplex_fd[i][WRITE_END], path, strlen(path));
    }
}


/*
    Inicializa un slave, retornando un array de 2 enteros.
    Levanta 2 pipes. Retorna 0 si todo sale bien, -1 si hubo error
    Llena un array de 2 enteros siendo el primero el FD del write end del pipe n1 y el segundo el FD del read end del pipe n2
*/
int init_slave(int** slaves_duplex_fd, int slaves_duplex_fd_size, int new_slave_index){
    int pipe_fd_master_to_slave[2];
    int pipe_fd_slave_to_master[2];

    if(pipe(pipe_fd_master_to_slave) == -1 || pipe(pipe_fd_slave_to_master) == -1) {
        perror("pipe failed");
        return -1;
    }

    switch(fork()) {
        case -1:
            perror("fork failed");
            return -1;
        case 0:
            close(pipe_fd_master_to_slave[WRITE_END]);                 // NO VOY A USAR el write end del pipe master->slave
            close(pipe_fd_slave_to_master[READ_END]);                  // NO VOY A USAR el read end del pipe slave->master
            dup2(pipe_fd_master_to_slave[READ_END], STDIN_FILENO);
            dup2(pipe_fd_slave_to_master[WRITE_END], STDOUT_FILENO);
            close(pipe_fd_master_to_slave[READ_END]);
            close(pipe_fd_slave_to_master[WRITE_END]);
            
            for(int i = 0; i < slaves_duplex_fd_size; i++) {
                if(i != new_slave_index) {
                    close(slaves_duplex_fd[i][READ_END]);
                    close(slaves_duplex_fd[i][WRITE_END]);
                }
            }
            
            char* argv_slave[] = {SLAVE_BIN_PATH, NULL};
            execv(SLAVE_BIN_PATH, argv_slave);
            break;
        default:
            close(pipe_fd_master_to_slave[READ_END]);
            close(pipe_fd_slave_to_master[WRITE_END]);
            slaves_duplex_fd[new_slave_index][READ_END] = pipe_fd_slave_to_master[READ_END];
            slaves_duplex_fd[new_slave_index][WRITE_END] = pipe_fd_master_to_slave[WRITE_END];
            break;
    }
    return 0;
}


/*
Recibe un array de path de files validos
Retorna una lista con una cola de files, ordenada por tamano 
*/
list_adt init_file_list(int argc, char* argv[], size_t* total_output_size) {
    struct stat file_stat;
    char* file_path;                              // este directory stream queda abierto y se itera llamando a readdir() o rewinddir()
    list_adt list = new_list(compare_ascending);

    for(int i = 1; i < argc; i++) {         // Mientras queden dir-entrys para iterar...
        file_path = argv[i];
        
        // Consigo la info de la entrada
        if (stat(file_path, &file_stat) == -1) {
            perror("stat");
        }

        // add(list, file_path, file_stat.st_size);        
        if(!S_ISDIR(file_stat.st_mode)){
            add(list, file_path, file_stat.st_size);
        }
        
        *total_output_size += GET_OUTOUT_SIZE(strlen(file_path));   // +1 por el salto de linea
    }

    return list;
}


/*
    Inicializa la memoria compartida
    Retorna el file descriptor de la memoria compartida
    Guarda la direccion de memoria en shm_map_address & imprime el nombre de la memoria compartida
*/
int init_shm(char **shm_map_address, size_t shm_size) {
  int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("shm_open failed");
    exit(EXIT_FAILURE);
  }

  if (ftruncate(fd, shm_size) == -1) {
    perror("ftruncate failed");
    exit(EXIT_FAILURE);
  }
  
  *shm_map_address = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (*shm_map_address == MAP_FAILED) {
    perror("mmap failed");
    exit(EXIT_FAILURE);
  }

    fflush(stdout);
  
  return fd;
}


/*
    Crea un archivo y vuelva los resultados guardados en la shm
*/
void init_result_file(int processed_jobs, char *shm_map_address, size_t shm_size) {
    FILE * file = fopen(RESULT_FILE_PATH, "w");
    if (file == NULL) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "PID -- MD5 -- Filename\n");

    size_t offset = 0;
    while (processed_jobs > 0) {
        char *current_string = shm_map_address + offset;
        
        fprintf(file, "%s", current_string);
        offset += strlen(current_string) + 1;
        
        processed_jobs--;
    }

    fclose(file);
}
