# Lab 3 - Sudoku Validator

Validador de tableros Sudoku implementado en C utilizando **hilos POSIX (pthreads)**, **OpenMP** y **`fork()`** para verificar concurrentemente las filas, columnas y subcuadrículas de 3×3.

## Descripción

El programa `SudokuValidator` lee un tablero de Sudoku desde un archivo plano, luego valida:

- **Subcuadrículas 3×3** → mediante un bucle paralelo OpenMP en el hilo principal.
- **Columnas** → mediante un hilo POSIX (`pthread`) que internamente usa OpenMP.
- **Filas** → mediante un bucle paralelo OpenMP en el hilo principal tras el `pthread_join`.

Además, se realizan dos llamadas a `fork()` + `ps -lLf` para mostrar el estado de los LWP (Light Weight Processes / hilos del SO) antes y después de la validación de filas.

## Archivos

| Archivo | Descripción |
|---|---|
| `SudokuValidator.c` | Código fuente principal |
| `sudoku` | Tablero de Sudoku de prueba (81 dígitos, sin separadores) |

## Compilación

```bash
gcc -o SudokuValidator SudokuValidator.c -lpthread -fopenmp
```

## Uso

```bash
./SudokuValidator <archivo>
```

**Ejemplo:**
```bash
./SudokuValidator sudoku
```

## Output del programa

```
Subgrid ejecutado por TID: 9339
Subgrid ejecutado por TID: 9339
Subgrid ejecutado por TID: 9339
Subgrid ejecutado por TID: 9339
Subgrid ejecutado por TID: 9339
Subgrid ejecutado por TID: 9339
Subgrid ejecutado por TID: 9339
Subgrid ejecutado por TID: 9341
Subgrid ejecutado por TID: 9340
PTHREAD columnas (TID): 9343
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Columna ejecutada por TID: 9348
Thread principal luego join (TID): 9334
F S UID        PID  PPID   LWP C NLWP PRI  NI ADDR SZ WCHAN  STIME TTY          TIME CMD
0 S joserui+  9334  4715  9334  0    8  80   0 - 56201 do_wai 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9335  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9336  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9337  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9338  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9339  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9340  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9341  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
Fila ejecutada por TID: 9334
Fila ejecutada por TID: 9337
Fila ejecutada por TID: 9337
Fila ejecutada por TID: 9334
Fila ejecutada por TID: 9334
Fila ejecutada por TID: 9334
Fila ejecutada por TID: 9334
Fila ejecutada por TID: 9334
Fila ejecutada por TID: 9334
Sudoku válido
F S UID        PID  PPID   LWP C NLWP PRI  NI ADDR SZ WCHAN  STIME TTY          TIME CMD
0 S joserui+  9334  4715  9334  0    8  80   0 - 56201 do_wai 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9335  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9336  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9337  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9338  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9339  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9340  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
1 S joserui+  9334  4715  9341  0    8  80   0 - 56201 futex_ 23:19 pts/0    00:00:00 ./SudokuValidator sudoku
```

### Observaciones del output

- Los **subgrids** son ejecutados principalmente por TID `9339`, con dos subgrids ejecutados por TIDs `9341` y `9340`, evidenciando el paralelismo dinámico de OpenMP repartiendo trabajo entre hilos del pool.
- El **pthread de columnas** (TID `9343`) crea internamente un nuevo pool de OpenMP (TID `9348`) para las 9 columnas.
- El `ps -lLf` antes de `check_rows()` muestra **8 LWPs** activos (NLWP=8): el hilo principal más los del pool OpenMP y pthreads.
- Las **filas** son ejecutadas por TIDs `9334` y `9337`, mostrando nuevamente el reparto dinámico de OpenMP.
- El segundo `ps -lLf` (tras la validación) muestra el mismo conjunto de LWPs, ya que los hilos de OpenMP permanecen vivos en el pool hasta el fin del programa.
- El resultado final es **`Sudoku válido`**, confirmando que el tablero de prueba es correcto.
