/*
 * matmul_pthreads.c - Item 4: multiplicação canônica paralelizada com Pthreads.
 * A matriz C é particionada em T faixas contíguas de linhas; cada thread escreve
 * apenas em suas linhas, portanto não há mutex nem condição de corrida.
 *
 * Uso: ./matmul_pthreads_O3 N T
 * Aluno: Diego Teruya - RA 10723404
 */
#include "common.h"
#include <pthread.h>

typedef struct {
    const double *A, *B;
    double *C;
    int n;
    int ini, fim;    /* faixa de linhas [ini, fim) desta thread */
} tarefa_t;

static void *trabalho(void *arg)
{
    const tarefa_t *t = (const tarefa_t *)arg;
    const double *A = t->A, *B = t->B;
    double *C = t->C;
    int n = t->n;

    for (int i = t->ini; i < t->fim; i++) {
        for (int j = 0; j < n; j++) {
            double soma = 0.0;   /* acumulador local: nada é escrito em C até o fim */
            for (int k = 0; k < n; k++) {
                soma += A[(size_t)i * n + k] * B[(size_t)k * n + j];
            }
            C[(size_t)i * n + j] = soma;
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int n  = le_inteiro(argc, argv, 1, 1024, "N");
    int nt = le_inteiro(argc, argv, 2, 4, "T");

    double *A = aloca_matriz(n), *B = aloca_matriz(n), *C = aloca_matriz(n);
    inicializa(A, B, n);
    memset(C, 0, (size_t)n * n * sizeof(double));

    pthread_t *th = malloc((size_t)nt * sizeof(pthread_t));
    tarefa_t  *tf = malloc((size_t)nt * sizeof(tarefa_t));
    if (!th || !tf) { fprintf(stderr, "Erro de alocação\n"); return EXIT_FAILURE; }

    /* Divisão em blocos contíguos; o resto (N % T) vai para as primeiras threads. */
    int base = n / nt, resto = n % nt, linha = 0;

    double t0 = agora();
    for (int t = 0; t < nt; t++) {
        int qtd = base + (t < resto ? 1 : 0);
        tf[t] = (tarefa_t){ A, B, C, n, linha, linha + qtd };
        linha += qtd;
        if (pthread_create(&th[t], NULL, trabalho, &tf[t]) != 0) {
            fprintf(stderr, "Erro ao criar thread %d\n", t);
            return EXIT_FAILURE;
        }
    }
    for (int t = 0; t < nt; t++) pthread_join(th[t], NULL);
    double tempo = agora() - t0;

    long erros = verifica_produto(C, n);
    printf("[Matmul Pthreads] N: %d | Threads: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.6e | Verificação: %s\n",
           n, nt, tempo, gflops(n, tempo), checksum(C, n), erros == 0 ? "OK" : "FALHOU");

    free(th); free(tf); free(A); free(B); free(C);
    return erros == 0 ? 0 : 1;
}
