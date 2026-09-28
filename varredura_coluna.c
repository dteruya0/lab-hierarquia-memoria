/*
 * varredura_coluna.c - Item 2: contagem de pares em ordem de COLUNA (Column-Major).
 * Laço externo em j (colunas), interno em i (linhas): saltos de N*8 bytes a cada acesso.
 *
 * Uso: ./varredura_coluna_O3 N [repeticoes]
 * Aluno: Diego Teruya - RA 10723404
 */
#include "common.h"

int main(int argc, char **argv)
{
    int n    = le_inteiro(argc, argv, 1, 4096, "N");
    int reps = le_inteiro(argc, argv, 2, 5, "repeticoes");

    double *A = aloca_matriz(n);
    inicializa(A, NULL, n);

    long pares = 0;
    double melhor = 1e30, soma_t = 0.0;
    for (int r = 0; r < reps; r++) {
        double t0 = agora();
        pares = 0;
        for (int j = 0; j < n; j++) {  // colunas
            for (int i = 0; i < n; i++) {  // linhas
                if ((int)A[(size_t)i * n + j] % 2 == 0) {
                    pares++;
                }
            }
        }
        double t = agora() - t0;
        soma_t += t;
        if (t < melhor) melhor = t;
    }

    printf("[Varredura Coluna] N: %d | Pares: %ld | Tempo (min): %.6f s | Tempo (media): %.6f s | Reps: %d\n",
           n, pares, melhor, soma_t / reps, reps);
    free(A);
    return 0;
}
