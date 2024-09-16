#ifndef __UTILITIES_H
#define __UTILITIES_H

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

#define MD5_BIN_PATH "/usr/bin/md5sum"

#endif // __UTILITIES_H
