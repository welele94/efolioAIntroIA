import csv
import time

from solver import resolver
from utils import ler_instancia


LIMITE_MS = 20000


def correr_todas():
    resultados = []

    for i in range(1, 11):
        caminho = f"instancias/instancia_{i}.txt"
        tabuleiro = ler_instancia(caminho)

        inicio = time.perf_counter()
        solucao = resolver(tabuleiro, limite_ms=LIMITE_MS)
        fim = time.perf_counter()

        tempo_ms = int((fim - inicio) * 1000)
        solucao_str = " ".join(solucao) if solucao else ""

        resultados.append([i, tempo_ms, solucao_str])

        estado = "OK" if solucao else "SEM SOLUCAO"
        print(f"Instância {i}: {tempo_ms} ms - {estado}")

    with open("resultados.csv", "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f, delimiter=";")
        writer.writerow(["Instancia", "Tempo(ms)", "Solucao"])
        writer.writerows(resultados)

    print("\nCSV gerado: resultados.csv")


def main():
    correr_todas()


if __name__ == "__main__":
    main()