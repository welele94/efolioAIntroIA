from collections import deque
from utils import pos_para_casa


def encontrar_pecas(tabuleiro):
    pecas = []
    peoes = []

    for linha in range(8):
        for coluna in range(8):
            casa = tabuleiro[linha][coluna]

            if casa.isupper():
                pecas.append((casa, linha, coluna))
            elif casa == "p":
                peoes.append((linha, coluna))

    return pecas, peoes


def dentro(linha, coluna):
    return 0 <= linha < 8 and 0 <= coluna < 8


def movimentos_cavalo(linha, coluna):
    movimentos = [
        (-2, -1), (-2, 1),
        (-1, -2), (-1, 2),
        (1, -2), (1, 2),
        (2, -1), (2, 1)
    ]

    destinos = []

    for dl, dc in movimentos:
        nl = linha + dl
        nc = coluna + dc

        if dentro(nl, nc):
            destinos.append((nl, nc))

    return destinos
def movimentos_linha(tabuleiro, linha, coluna, direcoes):
    destinos = []

    for dl, dc in direcoes:
        nl = linha + dl
        nc = coluna + dc

        while dentro(nl, nc):
            casa = tabuleiro[nl][nc]

            if casa == " ":
                nl += dl
                nc += dc
                continue

            if casa == "p":
                destinos.append((nl, nc))

            break

    return destinos


def capturas_peca(tabuleiro, peca, linha, coluna):
    if peca == "C":
        return [
            (l, c)
            for l, c in movimentos_cavalo(linha, coluna)
            if tabuleiro[l][c] == "p"
        ]

    if peca == "T":
        return movimentos_linha(tabuleiro, linha, coluna, [
            (1, 0), (-1, 0), (0, 1), (0, -1)
        ])

    if peca == "B":
        return movimentos_linha(tabuleiro, linha, coluna, [
            (1, 1), (1, -1), (-1, 1), (-1, -1)
        ])

    if peca == "D":
        return movimentos_linha(tabuleiro, linha, coluna, [
            (1, 0), (-1, 0), (0, 1), (0, -1),
            (1, 1), (1, -1), (-1, 1), (-1, -1)
        ])

    return []

def tabuleiro_para_string(tabuleiro):
    return "".join("".join(linha) for linha in tabuleiro)


def copiar_tabuleiro(tabuleiro):
    return [linha[:] for linha in tabuleiro]


def resolver(tabuleiro):
    pecas, peoes = encontrar_pecas(tabuleiro)

    fila = deque()
    visitados = set()

    # ação inicial: escolher uma peça branca
    for peca, linha, coluna in pecas:
        if peca == "C":
            caminho = [pos_para_casa(linha, coluna)]
            estado = (tabuleiro_para_string(tabuleiro), linha, coluna)

            fila.append((tabuleiro, linha, coluna, caminho))
            visitados.add(estado)

    while fila:
        atual, linha, coluna, caminho = fila.popleft()

        _, peoes_restantes = encontrar_pecas(atual)

        if len(peoes_restantes) == 0:
            return caminho
        
        peca_ativa = atual[linha][coluna]

        for nl, nc in capturas_peca(atual, peca_ativa, linha, coluna):
            novo = copiar_tabuleiro(atual)

            novo[linha][coluna] = " "
            novo[nl][nc] = peca_ativa

            novo_caminho = caminho + [pos_para_casa(nl, nc)]
            estado = (tabuleiro_para_string(novo), nl, nc)

            if estado not in visitados:
                visitados.add(estado)
                fila.append((novo, nl, nc, novo_caminho))

    return []