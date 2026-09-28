#!/usr/bin/env bash
# run_experiments.sh - Executa todos os experimentos e salva CSVs + logs em results/
# Aluno: Diego Teruya - RA 10723404
#
# Variáveis ajustáveis (ex.: BS=32 THREADS="1 2 4 8" ./run_experiments.sh):
#   BS        tamanho do bloco usado nos Itens 3 e 4        (padrão 64)
#   N_PAR     ordem da matriz no Item 4                     (padrão 2048)
#   CG_SIZES  N's simulados no cachegrind (lento! ~50x)     (padrão "512")
#   SKIP_8192 =1 pula N=8192 no Item 2 (usa 512 MB de RAM)
set -u
BS=${BS:-64}
N_PAR=${N_PAR:-2048}
CG_SIZES=${CG_SIZES:-512}
THREADS=${THREADS:-"1 2 4 8 16"}
R=results
mkdir -p "$R"

tempo() { grep -oE 'Tempo( \(min\))?: [0-9.]+' | grep -oE '[0-9.]+$'; }

echo "=== Compilação ==="
make clean >/dev/null && make 2>&1 | tee "$R/compilacao.txt"

echo "=== Caracterização do hardware -> $R/hardware.txt ==="
{
  echo "### lscpu";                 lscpu
  echo; echo "### Caches (lscpu -C)"; lscpu -C 2>/dev/null
  echo; echo "### Caches (sysfs: nível, tipo, tamanho, vias, linha)"
  for d in /sys/devices/system/cpu/cpu0/cache/index*; do
    echo "L$(cat $d/level) $(cat $d/type): $(cat $d/size), $(cat $d/ways_of_associativity)-way, linha $(cat $d/coherency_line_size) B"
  done
  echo; echo "### Memória";             free -h
  echo "(velocidade da RAM: rode 'sudo dmidecode -t memory | grep -i speed')"
  echo; echo "### SO / kernel";         grep PRETTY_NAME /etc/os-release; uname -r
  echo; echo "### GCC";                 gcc --version | head -1
  echo; echo "### Valgrind";            valgrind --version 2>/dev/null || echo "não instalado"
  echo; echo "### perf";                perf --version 2>/dev/null || echo "não instalado"
} > "$R/hardware.txt" 2>&1
cat "$R/hardware.txt" | sed -n '/### Caches (sysfs/,/### Memória/p'

echo "=== Item 2: varredura linha x coluna ==="
SIZES2="512 1024 2048 4096 8192"
[ "${SKIP_8192:-0}" = 1 ] && SIZES2="512 1024 2048 4096"
echo "opt,N,t_linha,t_coluna,slowdown" > "$R/item2.csv"
for opt in O0 O3; do
  for N in $SIZES2; do
    L=$(./varredura_linha_$opt  $N 5 | tee -a "$R/item2_log.txt")
    C=$(./varredura_coluna_$opt $N 5 | tee -a "$R/item2_log.txt")
    echo "$L"; echo "$C"
    tl=$(echo "$L" | tempo); tc=$(echo "$C" | tempo)
    echo "$opt,$N,$tl,$tc,$(awk -v a=$tc -v b=$tl 'BEGIN{printf "%.2f", a/b}')" >> "$R/item2.csv"
  done
done

echo "=== Item 3: matmul padrão x bloco (tempo nativo) ==="
echo "opt,N,versao,tempo,gflops" > "$R/item3_tempo.csv"
for opt in O0 O3; do
  for N in 512 1024 1536; do
    P=$(./matmul_padrao_$opt $N     | tee -a "$R/item3_log.txt"); echo "$P"
    B=$(./matmul_bloco_$opt  $N $BS | tee -a "$R/item3_log.txt"); echo "$B"
    echo "$opt,$N,padrao,$(echo "$P" | tempo),$(echo "$P" | grep -oE 'GFLOPS: [0-9.]+' | grep -oE '[0-9.]+$')" >> "$R/item3_tempo.csv"
    echo "$opt,$N,bloco,$(echo "$B" | tempo),$(echo "$B" | grep -oE 'GFLOPS: [0-9.]+' | grep -oE '[0-9.]+$')" >> "$R/item3_tempo.csv"
  done
done

echo "=== Item 3: calibração do tamanho do bloco (N=1024, -O3) ==="
echo "bs,tempo,gflops" > "$R/item3_bs.csv"
for b in 8 16 32 64 128 256 512 1024; do   # bs=1024=N -> só a ordem i,k,j, sem blocagem
  O=$(./matmul_bloco_O3 1024 $b | tee -a "$R/item3_log.txt"); echo "$O"
  echo "$b,$(echo "$O" | tempo),$(echo "$O" | grep -oE 'GFLOPS: [0-9.]+' | grep -oE '[0-9.]+$')" >> "$R/item3_bs.csv"
done

echo "=== Item 3: cachegrind (N em: $CG_SIZES) ==="
if command -v valgrind >/dev/null; then
  echo "opt,N,versao,D1_miss_rate,LLd_miss_rate,D1_misses,LLd_misses" > "$R/item3_cache.csv"
  for opt in O0 O3; do
    for N in $CG_SIZES; do
      for v in padrao bloco; do
        args="$N"; [ $v = bloco ] && args="$N $BS"
        f="$R/cachegrind_${v}_${opt}_${N}.txt"
        echo "valgrind --tool=cachegrind --cache-sim=yes ./matmul_${v}_${opt} $args"
        # --cache-sim=yes é obrigatório no Valgrind >= 3.21 (antes era o padrão)
        valgrind --tool=cachegrind --cache-sim=yes --cachegrind-out-file=/dev/null \
                 ./matmul_${v}_${opt} $args 2>&1 | tee "$f"
        d1r=$(grep -E 'D1 +miss rate' "$f"  | sed -E 's/.*rate: *([0-9.]+)%.*/\1/')
        llr=$(grep -E 'LLd miss rate' "$f"  | sed -E 's/.*rate: *([0-9.]+)%.*/\1/')
        d1m=$(grep -E 'D1 +misses' "$f"     | sed -E 's/.*misses: *([0-9,]+).*/\1/' | tr -d ,)
        llm=$(grep -E 'LLd misses' "$f"     | sed -E 's/.*misses: *([0-9,]+).*/\1/' | tr -d ,)
        echo "$opt,$N,$v,$d1r,$llr,$d1m,$llm" >> "$R/item3_cache.csv"
      done
    done
  done
else
  echo "valgrind não encontrado: sudo apt install valgrind"
fi

echo "=== Item 4: escalabilidade Pthreads (N=$N_PAR, -O3) ==="
echo "versao,T,tempo,gflops,verif" > "$R/item4.csv"
for T in $THREADS; do
  P=$(./matmul_pthreads_O3       $N_PAR $T     | tee -a "$R/item4_log.txt"); echo "$P"
  B=$(./matmul_pthreads_bloco_O3 $N_PAR $T $BS | tee -a "$R/item4_log.txt"); echo "$B"
  for X in "$P" "$B"; do
    v=pthreads; [[ "$X" == *Bloco* ]] && v=pthreads_bloco
    echo "$v,$T,$(echo "$X" | tempo),$(echo "$X" | grep -oE 'GFLOPS: [0-9.]+' | grep -oE '[0-9.]+$'),$(echo "$X" | grep -oE '(OK|FALHOU)$')" >> "$R/item4.csv"
  done
done

echo; echo "Pronto. CSVs e logs em $R/. Gere tabelas e gráficos com: python3 plot.py"
