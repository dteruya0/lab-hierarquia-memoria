# Lab CompPar — Hierarquia de Memória, Tiling e Pthreads

**Aluno:** Diego Teruya — RA 10723404

## Arquivos
| Arquivo | Item | Descrição |
|---|---|---|
| `common.h` | — | Tempo (`clock_gettime`), alocação contígua alinhada, inicialização do Item 1, verificação exata do produto |
| `varredura_linha.c` / `varredura_coluna.c` | 2 | Contagem de pares em ordem Row-Major / Column-Major |
| `matmul_padrao.c` | 3 | Multiplicação canônica (i, j, k) |
| `matmul_bloco.c` | 3 | Multiplicação com blocagem bs × bs (ordem interna i, k, j) |
| `matmul_pthreads.c` | 4 | Canônica paralela, faixas contíguas de linhas, sem mutex |
| `matmul_pthreads_bloco.c` | 4 | Pthreads + blocagem |
| `Makefile` | — | Gera `<programa>_O0` e `<programa>_O3` |
| `run_experiments.sh` | — | Roda todos os experimentos, coleta hardware, salva CSVs/logs em `results/` |
| `plot.py` | — | Gera as Tabelas 1–3 (`results/tabelas.md`) e os gráficos |

## Como rodar (Linux)
```bash
sudo apt install build-essential valgrind python3-matplotlib
make                       # compilação limpa com -Wall -Wextra
./run_experiments.sh       # ~10–30 min dependendo da máquina
python3 plot.py            # tabelas + gráficos em results/
```
Variáveis opcionais: `BS=32`, `N_PAR=2048`, `CG_SIZES="512 1024"`, `THREADS="1 2 4 8 16"`, `SKIP_8192=1`.

## Uso individual
```bash
./varredura_linha_O3 4096 5        # N, repetições
./matmul_padrao_O3 1024            # N
./matmul_bloco_O3 1024 64          # N, bloco
./matmul_pthreads_O3 2048 8        # N, threads
./matmul_pthreads_bloco_O3 2048 8 64
valgrind --tool=cachegrind --cache-sim=yes ./matmul_bloco_O3 512 64
```

## Verificação de integridade
Com A[i][k] = i+k e B[k][j] = k·j, vale C[i][j] = j·(i·S1 + S2), com S1 = Σk e S2 = Σk².
Todos os valores são inteiros exatos em `double`, então cada programa compara **todos** os
elementos de C com a fórmula e imprime `Verificação: OK`.
