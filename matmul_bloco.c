/*
 * matmul_bloco.c - Item 3: multiplicação com blocagem (tiling) em sub-blocos bs x bs.
 * Ordem interna i, k, j: B[k][j] e C[i][j] são percorridos de forma contígua,
 * o que também permite a auto-vetorização SIMD sob -O3.
 *
 * Observação: no enunciado a variável do tamanho do bloco se chama "B", mesmo nome
 * da matriz B -- isso não compila. Aqui o tamanho do bloco se chama "bs".
 *
 * Uso: ./matmul_bloco_O3 N bs
 * Aluno: Diego Teruya - RA 10723404
 */
#include "common.h"

static void matmul_bloco(const double *A, const double *B, double *C, int n, int bs)
{
    for (int ii = 0; ii < n; ii += bs) {
        int i_fim = ii + bs < n ? ii + bs : n;
        for (int jj = 0; jj < n; jj += bs) {
            int j_fim = jj + bs < n ? jj + bs : n;
            for (int kk = 0; kk < n; kk += bs) {
                int k_fim = kk + bs < n ? kk + bs : n;
                /* Multiplicação do bloco bs x bs */
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
}

int main(int argc, char **argv)
{
    int n  = le_inteiro(argc, argv, 1, 512, "N");
    int bs = le_inteiro(argc, argv, 2, 64, "bs");

    double *A = aloca_matriz(n), *B = aloca_matriz(n), *C = aloca_matriz(n);
    inicializa(A, B, n);
    memset(C, 0, (size_t)n * n * sizeof(double));   /* obrigatório: o bloco acumula (+=) */

    double t0 = agora();
    matmul_bloco(A, B, C, n, bs);
    double t = agora() - t0;

    long erros = verifica_produto(C, n);
    printf("[Matmul Blocado] N: %d | Bloco B: %d | Tempo: %.4f s | GFLOPS: %.2f | Checksum: %.6e | Verificação: %s\n",
           n, bs, t, gflops(n, t), checksum(C, n), erros == 0 ? "OK" : "FALHOU");

    free(A); free(B); free(C);
    return erros == 0 ? 0 : 1;
}
