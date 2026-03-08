#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <omp.h>

int sudoku[9][9];
int valid = 1;

void check_rows() {

    omp_set_num_threads(4);

    #pragma omp parallel for schedule(dynamic)
    for(int i = 0; i < 9; i++) {

        printf("Fila ejecutada por TID: %ld\n",
               syscall(SYS_gettid));

        int seen[10] = {0};
        int local_valid = 1;

        for(int j = 0; j < 9; j++) {
            int num = sudoku[i][j];

            if(num < 1 || num > 9 || seen[num]) {
                local_valid = 0;
            }
            seen[num] = 1;
        }

        if(!local_valid) {
            #pragma omp critical
            valid = 0;
        }
    }
}

void check_columns() {

    omp_set_num_threads(4);

    #pragma omp parallel for schedule(dynamic)
    for(int j = 0; j < 9; j++) {

        printf("Columna ejecutada por TID: %ld\n",
               syscall(SYS_gettid));

        int seen[10] = {0};
        int local_valid = 1;

        for(int i = 0; i < 9; i++) {
            int num = sudoku[i][j];

            if(num < 1 || num > 9 || seen[num]) {
                local_valid = 0;
            }
            seen[num] = 1;
        }

        if(!local_valid) {
            #pragma omp critical
            valid = 0;
        }
    }
}

void check_subgrids() {

    omp_set_num_threads(4);

    #pragma omp parallel for schedule(dynamic)
    for(int idx = 0; idx < 9; idx++) {

        int start_row = (idx / 3) * 3;
        int start_col = (idx % 3) * 3;

        printf("Subgrid ejecutado por TID: %ld\n",
               syscall(SYS_gettid));

        int seen[10] = {0};
        int local_valid = 1;

        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {

                int num = sudoku[start_row + i][start_col + j];

                if(num < 1 || num > 9 || seen[num]) {
                    local_valid = 0;
                }

                seen[num] = 1;
            }
        }

        if(!local_valid) {
            #pragma omp critical
            valid = 0;
        }
    }
}

void* column_thread(void* arg) {

    printf("PTHREAD columnas (TID): %ld\n",
           syscall(SYS_gettid));

    check_columns();

    pthread_exit(0);
}

int main(int argc, char *argv[]) {

    if(argc != 2) {
        printf("Uso: %s <archivo>\n", argv[0]);
        return 1;
    }

    omp_set_nested(1);


    int fd = open(argv[1], O_RDONLY);
    if(fd < 0) {
        perror("Error abriendo archivo");
        return 1;
    }

    char *map = mmap(NULL, 82, PROT_READ, MAP_PRIVATE, fd, 0);
    if(map == MAP_FAILED) {
        perror("Error en mmap");
        return 1;
    }

    int k = 0;
    for(int i = 0; i < 9; i++) {
        for(int j = 0; j < 9; j++) {
            sudoku[i][j] = map[k++] - '0';
        }
    }

    check_subgrids();

    pid_t parent_pid = getpid();
    pid_t pid = fork();

    if(pid == 0) {
        char pid_str[20];
        sprintf(pid_str, "%d", parent_pid);
        execlp("ps", "ps", "-p", pid_str, "-lLf", NULL);
        exit(0);
    } else {
        wait(NULL);
    }

    pthread_t tid;
    pthread_create(&tid, NULL, column_thread, NULL);
    pthread_join(tid, NULL);

    printf("Thread principal luego join (TID): %ld\n",
           syscall(SYS_gettid));

    check_rows();

    if(valid)
        printf("Sudoku válido\n");
    else
        printf("Sudoku inválido\n");

    parent_pid = getpid();
    pid = fork();

    if(pid == 0) {
        char pid_str[20];
        sprintf(pid_str, "%d", parent_pid);
        execlp("ps", "ps", "-p", pid_str, "-lLf", NULL);
        exit(0);
    } else {
        wait(NULL);
    }

    munmap(map, 82);
    close(fd);

    return 0;
}
