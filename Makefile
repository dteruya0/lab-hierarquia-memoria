# Makefile - Lab Hierarquia de Memória + Pthreads
# Aluno: Diego Teruya - RA 10723404
#
#   make          -> compila todos os programas em -O0 e -O3
#   make clean    -> remove executáveis
#   make run      -> executa todos os experimentos (run_experiments.sh)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
LDFLAGS = -pthread

PROGS = varredura_linha varredura_coluna matmul_padrao matmul_bloco \
        matmul_pthreads matmul_pthreads_bloco

BINS = $(addsuffix _O0,$(PROGS)) $(addsuffix _O3,$(PROGS))

all: $(BINS)

%_O0: %.c common.h
	$(CC) -O0 $(CFLAGS) $< -o $@ $(LDFLAGS)

%_O3: %.c common.h
	$(CC) -O3 $(CFLAGS) $< -o $@ $(LDFLAGS)

run: all
	./run_experiments.sh

clean:
	rm -f $(BINS)

.PHONY: all clean run
