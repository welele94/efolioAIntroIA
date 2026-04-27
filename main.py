import sys

from solver import resolver
from utils import ler_instancia


def main():
    caminho_instancia = (
        sys.argv[1] if len(sys.argv) > 1 else "instancias/instancia_1.txt"
    )

    tabuleiro = ler_instancia(caminho_instancia)
    solucao = resolver(tabuleiro)

    if solucao:
        print("Solução:", " ".join(solucao))
        return 0

    print("Sem solução.")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
