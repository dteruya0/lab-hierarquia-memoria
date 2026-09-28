/*
 * matmul_padrao.c - Item 3: multiplicação canônica C = A x B (laços i, j, k).
 * No laço interno, B[k][j] salta N*8 bytes a cada iteração -> muitos cache misses.
 *
 * Uso: ./matmul_padrao_O3 N
 * Aluno: Diego Teruya - RA 10723404
 */
#include "common.h"

static void matmul_padrao(const double *A, const double *B, double *C, int n)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double soma = 0.0;
            for (int k = 0; k < n; k++) {
                soma += A[(size_t)i * n + k] * B[(size_t)k * n + j];
            }
            C[(size_t)i * n + j] = soma;
        }
    }
}

int main(int argc, char **argv)
{
    int n = le_inteiro(argc, argv, 1, 512, "N");

    double *A = aloca_matriz(n), *B = aloca_matriz(n), *C = aloca_matriz(n);
    inicializa(A, B, n);
    memset(C, 0, (size_t)n * n * sizeof(double));

    double t0 = agora();
    matmul_padrao(A, B, C, n);
    double t = agora() - t0;

    long erros = verifica_produto(C, n);
    printf("[Matmul Padrão] N: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.6e | Verificação: %s\n",
           n, t, gflops(n, t), checksum(C, n), erros == 0 ? "OK" : "FALHOU");

    free(A); free(B); free(C);
    return erros == 0 ? 0 : 1;
}
