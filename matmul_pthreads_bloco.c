/*
 * matmul_pthreads_bloco.c - Item 4: Pthreads + blocagem (tiling).
 * Cada thread recebe uma faixa contígua de linhas de C e a multiplica
 * usando sub-blocos bs x bs (ordem interna i, k, j). Sem mutex.
 *
 * Uso: ./matmul_pthreads_bloco_O3 N T bs
 * Aluno: Diego Teruya - RA 10723404
 */
#include "common.h"
#include <pthread.h>

typedef struct {
    const double *A, *B;
    double *C;
    int n, bs;
    int ini, fim;    /* faixa de linhas [ini, fim) desta thread */
} tarefa_t;

static void *trabalho(void *arg)
{
    const tarefa_t *t = (const tarefa_t *)arg;
    const double *A = t->A, *B = t->B;
    double *C = t->C;
    int n = t->n, bs = t->bs;

    for (int ii = t->ini; ii < t->fim; ii += bs) {
        int i_fim = ii + bs < t->fim ? ii + bs : t->fim;   /* não ultrapassa a faixa da thread */
        for (int jj = 0; jj < n; jj += bs) {
            int j_fim = jj + bs < n ? jj + bs : n;
            for (int kk = 0; kk < n; kk += bs) {
                int k_fim = kk + bs < n ? kk + bs : n;
                for (int i = ii; i < i_fim; i++) {
                    for (int k = kk; k < k_fim; k++) {
                        double r = A[(size_t)i * n + k];
                        for (int j = jj; j < j_fim; j++) {
                            C[(size_t)i * n + j] += r * B[(size_t)k * n + j];
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int n  = le_inteiro(argc, argv, 1, 1024, "N");
    int nt = le_inteiro(argc, argv, 2, 4, "T");
    int bs = le_inteiro(argc, argv, 3, 64, "bs");

    double *A = aloca_matriz(n), *B = aloca_matriz(n), *C = aloca_matriz(n);
    inicializa(A, B, n);
    memset(C, 0, (size_t)n * n * sizeof(double));

    pthread_t *th = malloc((size_t)nt * sizeof(pthread_t));
    tarefa_t  *tf = malloc((size_t)nt * sizeof(tarefa_t));
    if (!th || !tf) { fprintf(stderr, "Erro de alocação\n"); return EXIT_FAILURE; }

    int base = n / nt, resto = n % nt, linha = 0;

    double t0 = agora();
    for (int t = 0; t < nt; t++) {
        int qtd = base + (t < resto ? 1 : 0);
        tf[t] = (tarefa_t){ A, B, C, n, bs, linha, linha + qtd };
        linha += qtd;
        if (pthread_create(&th[t], NULL, trabalho, &tf[t]) != 0) {
            fprintf(stderr, "Erro ao criar thread %d\n", t);
            return EXIT_FAILURE;
        }
    }
    for (int t = 0; t < nt; t++) pthread_join(th[t], NULL);
    double tempo = agora() - t0;

    long erros = verifica_produto(C, n);
    printf("[Matmul Pthreads Bloco] N: %d | Threads: %d | Bloco B: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.6e | Verificação: %s\n",
           n, nt, bs, tempo, gflops(n, tempo), checksum(C, n), erros == 0 ? "OK" : "FALHOU");

    free(th); free(tf); free(A); free(B); free(C);
    return erros == 0 ? 0 : 1;
}
