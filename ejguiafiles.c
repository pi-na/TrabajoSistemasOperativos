// Escribir un programa en C que liste recursivamente el contenido de un directorio
// que recibe como primer y único argumento, indicar si cada elemento es un archivo 
// o un directorio, tabular la salida por nivel de anidamiento (stat opendir readdir)

/*
Esto lo dieron en arqui. https://www.ibm.com/docs/es/i/7.5?topic=functions-main-function
El primer parámetro, argc (recuento de argumentos) es un entero que indica cuántos argumentos se han especificado en la línea de mandatos cuando se inició el programa. 
El segundo parámetro, argv (vector de argumento), es una matriz de punteros a matrices de objetos de caracteres. Los objetos de matriz son series terminadas en nulo, que representan los argumentos que se especificaron en la línea de mandatos cuando se inició el programa.
El primer elemento de la matriz, argv[0], es un puntero a la matriz de caracteres que contiene el nombre de programa o el nombre de invocación del programa que se está ejecutando desde la línea de mandatos. argv[1] indica el primer argumento pasado al programa, argv[2] el segundo argumento, etc.
 */

#include <stdio.h>
#include <sys/stat.h>           // fstat
#include <dirent.h>             // opendir
#include <strings.h>      

void printArgs(int arg, char *argv[]);
int findFileInCurrentDir(char * fileName, DIR * dir);
void printDir(char *path, int depth);

int main (int argc, char *argv[]){
    // printArgs(argc, argv);

    // printf("Se busco myFile.c. Resultado: %s\n",
            // findFileInCurrentDir("myFile.c", "dir2") ? "Encontrado!":"No se encontro.");

    if(argc < 2) return -1;
    printDir(argv[1], 0);

    return 0;
}


void printDir(char *path, int depth) {
    DIR * dirp;              // opendir retorna un puntero a DIR, un directory stream (equivalente al FD, para los dir)
                             // este directory stream queda abierto y se itera llamando a readdir() o rewinddir()
    struct dirent * direntry;      // readdir retorna un struct dirent con la info de la siguiente entrada en el dir
    char *FILETYPES[] = {"UNKNOWN", "DIR", "FILE"};     // 4 es DIR, 8 es FILE

    //  No hay entradas en el dir actual o no se pudo abrir
    if (dirp == NULL) {
        perror("opendir");
        return;
    }

    while ((direntry = readdir(dirp)) != NULL) {    // Mientras queden dir-entrys... 
        // Salteo /. y /.. (que vienen primeras)
        if (strcmp(direntry->d_name, ".") == 0 || strcmp(direntry->d_name, "..") == 0) {
            continue;
        }

        // Imprimir el tipo de archivo y el nombre
        printf("%s", FILETYPES[direntry->d_type / 4]);
        for (int i = 0; i <= depth; i++) {
            printf("    ");
        }
        printf(">%s\n", direntry->d_name);

        // Si la entrada es un directorio, llama recursivamente
        if (direntry->d_type == DT_DIR) {
            // Construye la ruta completa del subdirectorio (gracias GPT)
            char subdirPath[1024];
            snprintf(subdirPath, sizeof(subdirPath), "%s/%s", path, direntry->d_name);

            printDir(subdirPath, depth + 1);
        }
    }

    closedir(dirp);
}


//  Codigo de ejemplo sacado de man3 opendir
int findFileInCurrentDir(char * fileName, DIR * dir){
    #define ERROR -1
    #define NOT_FOUND 0
    #define FOUND 1
    DIR * dirp;              // opendir retorna un puntero a DIR
    struct dirent * direntry;      // readdir retorna un struct dirent con la info de la entrada actual del dir
    int len;
    char * name = fileName;

    dirp = opendir(dir);
    if (dirp == NULL)
            return (ERROR);
    len = strlen(name);
    while ((direntry = readdir(dirp)) != NULL) {
            if (direntry->d_namlen == len && strcmp(direntry->d_name, name) == 0) {
                    (void)closedir(dirp);
                    return (FOUND);
            }
    }
    (void)closedir(dirp);
    return (NOT_FOUND);
}


void printArgs(int argc, char *argv[]){
    printf("ARGV[0], NOMBRE DE PROGRAMA:%s\n", argv[0]);
    for(int i = 1; i < argc; i++){
        printf("ARGV[%d]:%s\n", i, argv[i]);
    }   
}

/*
    struct dirent{
        ino_t          d_ino;       // Número de inodo 
        off_t          d_off;       // Desplazamiento al siguiente dirent 
        unsigned short d_reclen;    // Longitud de este registro 
        unsigned char  d_type;      // Tipo de archivo 
        char           d_name[256]; // Nombre del archivo (null-terminated) 
    }
*/

/*
$> ./ej1 *
ARGV[0], NOMBRE DE PROGRAMA:./ej1
ARGV[1]:ej1
ARGV[2]:ej1.c
ARGV[3]:file1
ARGV[4]:file2
ARGV[5]:file3
*/