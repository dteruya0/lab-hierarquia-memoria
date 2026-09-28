/*
 * common.h - Utilitários compartilhados pelo laboratório de Hierarquia de Memória.
 *
 * Aluno: Diego Teruya - RA 10723404
 *
 * Contém:
 *   - agora():            relógio de alta precisão (clock_gettime, CLOCK_MONOTONIC)
 *   - aloca_matriz():     matriz N x N contígua, alinhada em 64 bytes (1 linha de cache)
 *   - inicializa():       inicialização determinística do Item 1
 *   - verifica_produto(): confere C = A x B contra a fórmula fechada (integridade numérica)
 *   - le_inteiro():       leitura validada de argumentos da linha de comando
 */
#ifndef COMMON_H
#define COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Tempo de parede em segundos (resolução de nanossegundos). */
static inline double agora(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* Aloca N*N doubles em um único bloco contíguo, alinhado à linha de cache.
 * Encerra o programa se não houver memória suficiente. */
static inline double *aloca_matriz(int n)
{
    void *p = NULL;
    size_t bytes = (size_t)n * (size_t)n * sizeof(double);
    if (posix_memalign(&p, 64, bytes) != 0) {
        fprintf(stderr, "Erro: falha ao alocar %zu bytes (N = %d)\n", bytes, n);
        exit(EXIT_FAILURE);
    }
    return (double *)p;
}

/* Inicialização padronizada do Item 1. B pode ser NULL (Item 2 só usa A). */
static inline void inicializa(double *A, double *B, int n)
{
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[(size_t)i * n + j] = (double)(i + j);
            if (B) B[(size_t)i * n + j] = (double)(i * j);
        }
    }
}

/* Com A[i][k] = i + k e B[k][j] = k * j, o produto tem forma fechada:
 *   C[i][j] = sum_k (i + k) * k * j = j * (i * S1 + S2),
 *   S1 = sum k   = n(n-1)/2,   S2 = sum k^2 = (n-1)n(2n-1)/6.
 * Todos os valores são inteiros < 2^53, logo exatos em double: a comparação
 * pode ser exata, independentemente da ordem das somas (padrão, bloco, threads).
 * Retorna o número de elementos incorretos. */
static inline long verifica_produto(const double *C, int n)
{
    double nn = (double)n;
    double S1 = nn * (nn - 1.0) / 2.0;
    double S2 = (nn - 1.0) * nn * (2.0 * nn - 1.0) / 6.0;
    long erros = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double esperado = (double)j * ((double)i * S1 + S2);
            if (C[(size_t)i * n + j] != esperado) erros++;
        }
    }
    return erros;
}

/* Soma de todos os elementos: "impressão digital" do resultado. */
static inline double checksum(const double *C, int n)
{
    double s = 0.0;
    for (size_t i = 0; i < (size_t)n * n; i++) s += C[i];
    return s;
}

/* Converte argv[idx] em inteiro positivo; usa 'padrao' se ausente. */
static inline int le_inteiro(int argc, char **argv, int idx, int padrao, const char *nome)
{
    if (argc <= idx) return padrao;
    char *fim = NULL;
    errno = 0;
    long v = strtol(argv[idx], &fim, 10);
    if (errno != 0 || *fim != '\0' || v <= 0 || v > 1000000) {
        fprintf(stderr, "Erro: argumento %s inválido: '%s'\n", nome, argv[idx]);
        exit(EXIT_FAILURE);
    }
    return (int)v;
}

static inline double gflops(int n, double segundos)
{
    return 2.0 * (double)n * (double)n * (double)n / segundos / 1e9;
}

#endif /* COMMON_H */
