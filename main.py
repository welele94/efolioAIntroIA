import csv
import os
import time

from solver import resolver
from utils import ler_instancia


LIMITE_MS = 20000


def obter_caminho_instancia(i):
    caminhos_possiveis = [
        f"instancia_{i}.txt",
        f"instancias/instancia_{i}.txt",
    ]

    for caminho in caminhos_possiveis:
        if os.path.exists(caminho):
            return caminho

    raise FileNotFoundError(f"instancia_{i}.txt nao encontrada")


def correr_todas():
    with open("resultados.csv", "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, delimiter=";")

        for i in range(1, 11):
            try:
                caminho = obter_caminho_instancia(i)
                tabuleiro = ler_instancia(caminho)

                inicio = time.perf_counter()
                solucao = resolver(tabuleiro, limite_ms=LIMITE_MS)
                fim = time.perf_counter()

                tempo_ms = int((fim - inicio) * 1000)
                solucao_str = " ".join(solucao) if solucao else ""

                estado = "OK" if solucao else "SEM SOLUCAO"
                print(f"Instancia {i}: {tempo_ms} ms - {estado}")

            except Exception as e:
                tempo_ms = 0
                solucao_str = ""
                print(f"Erro na instancia {i}: {type(e).__name__} - {e}")

            writer.writerow([i, tempo_ms, solucao_str])
            f.flush()

    print("CSV gerado: resultados.csv")


def main():
    correr_todas()


if __name__ == "__main__":
    main()
