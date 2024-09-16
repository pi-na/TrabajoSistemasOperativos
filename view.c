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
#define SHM_SEM_NAME "/shm_sem"
#define READ_END 0
#define WRITE_END 1
#define MAX_SLAVES 8
#define IDEAL_LOAD 4
#define MAX_RETRIES 5
#define BUFF_SIZE 1024
#define PID_DIGITS 5
#define SLAVE_OUTPUT_DIVIDER 2
#define MD5_HASH_SIZE 32
#define SHM_CLOSED "/end"
#define RESULT_FILE_PATH "/shared/result.txt"

int main(int argc, char* argv[]){
    char * shm_name;
    size_t shm_size;
    char *sem_name;

    if(argc == 4){
        shm_name = argv[1];
        shm_size = atoi(argv[2]);
        sem_name = argv[3];
    } else{
        char buffer[BUFF_SIZE];

        if (scanf("%s", buffer) != 1) {
            fprintf(stderr, "Error al leer el nombre de la memoria compartida\n");
            exit(EXIT_FAILURE);
        }
        shm_name = strdup(buffer);

        if (scanf("%s", buffer) != 1 || (shm_size = strtoull(buffer, NULL, 10)) == 0) {
            fprintf(stderr, "Error al leer el tamaño de la memoria compartida\n");
            exit(EXIT_FAILURE);
        }

        if (scanf("%s", buffer) != 1) {
            fprintf(stderr, "Error al leer el nombre del semáforo\n");
            exit(EXIT_FAILURE);
        }
        sem_name = strdup(buffer);

        if (shm_name == NULL || sem_name == NULL || shm_size == 0) {
            fprintf(stderr, "Error al leer los datos de entrada\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("%s %zu %s\n", shm_name, shm_size, sem_name);
    sem_t *sem = sem_open(sem_name, O_RDWR, S_IRUSR | S_IWUSR, 0);
    if(sem == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    char * shm_map_address = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_map_address == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    char readBuf[BUFF_SIZE] = "";
    char printBuf[BUFF_SIZE] = "";
    while(strcmp(printBuf, SHM_CLOSED)!=0){
        sem_wait(sem);
        int i;
        for(i = 0; read(shm_fd, readBuf, 1) > 0 && *readBuf!= '\n'; i++){
            printBuf[i] = readBuf[0];
        }
        printBuf[i] = '\0';
        printf("%s\n", printBuf);
        
    }

    shm_unlink(shm_name);
    close(shm_fd);
    sem_close(sem);

    return EXIT_SUCCESS;
}

