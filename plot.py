#!/usr/bin/env python3
"""plot.py - Gera tabelas (Markdown) e gráficos a partir de results/*.csv.
Aluno: Diego Teruya - RA 10723404
Requer: pip install matplotlib
"""
import csv, os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

R = "results"
def ler(nome):
    p = os.path.join(R, nome)
    return list(csv.DictReader(open(p))) if os.path.exists(p) else []

md = []

# ---------- Tabela 1: Item 2 ----------
it2 = ler("item2.csv")
if it2:
    md += ["## Tabela 1 - Varredura linha x coluna\n",
           "| Otimização | N | T_linha (s) | T_coluna (s) | Slowdown (T_col/T_lin) |",
           "|---|---|---|---|---|"]
    md += [f"| -{r['opt']} | {r['N']} | {float(r['t_linha']):.6f} | {float(r['t_coluna']):.6f} | {r['slowdown']}x |" for r in it2]
    plt.figure(figsize=(6, 4))
    for opt in ("O0", "O3"):
        rs = [r for r in it2 if r["opt"] == opt]
        plt.plot([int(r["N"]) for r in rs], [float(r["slowdown"]) for r in rs], "o-", label=f"-{opt}")
    plt.xscale("log", base=2); plt.xlabel("N"); plt.ylabel("Slowdown (T_coluna / T_linha)")
    plt.title("Item 2 - Custo da má localidade espacial"); plt.grid(alpha=.3); plt.legend()
    plt.tight_layout(); plt.savefig(f"{R}/grafico_item2_slowdown.png", dpi=150); plt.close()

# ---------- Tabela 2: Item 3 ----------
tempo = {(r["opt"], r["N"], r["versao"]): r for r in ler("item3_tempo.csv")}
cache = {(r["opt"], r["N"], r["versao"]): r for r in ler("item3_cache.csv")}
if tempo:
    md += ["\n## Tabela 2 - Multiplicação padrão x blocada\n",
           "| Otimização | N | Algoritmo | Tempo (s) | GFLOPS | D1 miss rate (%) | LLd miss rate (%) | LLd misses |",
           "|---|---|---|---|---|---|---|---|"]
    for k, r in tempo.items():
        c = cache.get(k, {})
        md.append(f"| -{k[0]} | {k[1]} | {k[2]} | {float(r['tempo']):.4f} | {r['gflops']} | "
                  f"{c.get('D1_miss_rate','—')} | {c.get('LLd_miss_rate','—')} | {c.get('LLd_misses','—')} |")
if cache:
    # Barras agrupadas: um grupo por (otimização, N), duas barras (padrão x bloco)
    grupos = []
    for (opt, n, _v) in cache:
        if (opt, n) not in grupos: grupos.append((opt, n))
    grupos.sort(key=lambda g: (g[0], int(g[1])))
    x = list(range(len(grupos))); w = 0.38
    rotulos = [f"N = {n}\n-{opt}" for opt, n in grupos]
    cores = {"padrao": "#c0392b", "bloco": "#2e86c1"}
    nomes = {"padrao": "Padrão (i, j, k)", "bloco": "Blocado (bs = 64)"}
    fig, ax = plt.subplots(1, 2, figsize=(12, 4.8))
    for a_, campo, titulo in ((ax[0], "D1_misses", "D1 misses (faltas na L1)"),
                              (ax[1], "LLd_misses", "LLd misses (acessos à DRAM)")):
        for d, v in ((-w / 2, "padrao"), (w / 2, "bloco")):
            ys = [int(cache[(o, n, v)][campo]) if (o, n, v) in cache else 0 for o, n in grupos]
            a_.bar([i + d for i in x], ys, w, color=cores[v], label=nomes[v])
        a_.set_yscale("log"); a_.set_title(titulo)
        a_.set_xticks(x); a_.set_xticklabels(rotulos, fontsize=9)
        a_.set_ylabel("Número de misses (escala log)")
        a_.grid(axis="y", alpha=.3); a_.legend(fontsize=9)
        if len(grupos) > 1 and grupos[0][0] != grupos[-1][0]:
            meio = sum(1 for g in grupos if g[0] == grupos[0][0]) - 0.5
            a_.axvline(meio, color="gray", ls=":", lw=1)
    plt.suptitle("Item 3 - Cache misses medidos com o Cachegrind")
    plt.tight_layout(); plt.savefig(f"{R}/grafico_item3_cache_misses.png", dpi=150); plt.close()

bs = ler("item3_bs.csv")
if bs:
    md += ["\n## Calibração do tamanho do bloco (N = 1024, -O3)\n",
           "| bs | 3·bs²·8 (KiB) | Tempo (s) | GFLOPS |", "|---|---|---|---|"]
    md += [f"| {r['bs']} | {3*int(r['bs'])**2*8/1024:.1f} | {float(r['tempo']):.4f} | {r['gflops']} |" for r in bs]
    plt.figure(figsize=(6, 4))
    plt.plot([int(r["bs"]) for r in bs], [float(r["gflops"]) for r in bs], "o-")
    plt.xscale("log", base=2); plt.xlabel("Tamanho do bloco bs"); plt.ylabel("GFLOPS")
    plt.title("Item 3 - Desempenho x tamanho do bloco"); plt.grid(alpha=.3)
    plt.tight_layout(); plt.savefig(f"{R}/grafico_item3_bloco.png", dpi=150); plt.close()

# ---------- Tabela 3: Item 4 ----------
it4 = ler("item4.csv")
if it4:
    md += ["\n## Tabela 3 - Escalabilidade Pthreads\n",
           "| Versão | Threads (p) | Tempo (s) | Speedup Sp = T1/Tp | Eficiência Ep = Sp/p | Verificação |",
           "|---|---|---|---|---|---|"]
    plt.figure(figsize=(6, 4))
    Ts = []
    for v in ("pthreads", "pthreads_bloco"):
        rs = sorted([r for r in it4 if r["versao"] == v], key=lambda r: int(r["T"]))
        if not rs: continue
        t1 = float(rs[0]["tempo"])
        Ts = [int(r["T"]) for r in rs]
        sp = [t1 / float(r["tempo"]) for r in rs]
        for r, s in zip(rs, sp):
            p = int(r["T"])
            md.append(f"| {v} | {p} | {float(r['tempo']):.4f} | {s:.2f} | {s/p:.2f} | {r['verif']} |")
        plt.plot(Ts, sp, "o-", label=v)
    plt.plot(Ts, Ts, "k--", alpha=.5, label="ideal (linear)")
    plt.xscale("log", base=2); plt.yscale("log", base=2); plt.xticks(Ts, Ts); plt.yticks(Ts, Ts)
    plt.xlabel("Número de threads"); plt.ylabel("Speedup"); plt.title("Item 4 - Speedup x threads")
    plt.grid(alpha=.3); plt.legend(); plt.tight_layout()
    plt.savefig(f"{R}/grafico_item4_speedup.png", dpi=150); plt.close()

open(f"{R}/tabelas.md", "w").write("\n".join(md) + "\n")
print("\n".join(md)); print(f"\nGráficos e tabelas.md salvos em {R}/")
