import heapq
import random
import time
from collections import deque
from functools import lru_cache

from utils import pos_para_casa

PECAS_ATIVAVEIS = {"C", "T", "B", "D"}
PECAS_BRANCAS = {"P", "T", "B", "C", "D", "R"}
DIRECOES = {
    "T": ((1, 0), (-1, 0), (0, 1), (0, -1)),
    "B": ((1, 1), (1, -1), (-1, 1), (-1, -1)),
    "D": ((1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (1, -1), (-1, 1), (-1, -1)),
}
ULTIMO_ALGORITMO = ""

CELULAS = tuple(range(64))
BIT = tuple(1 << i for i in CELULAS)
LINHA = tuple(i // 8 for i in CELULAS)
COLUNA = tuple(i % 8 for i in CELULAS)
MOV_CAVALO, MOV_REI = {}, {}

for pos in CELULAS:
    l, c = LINHA[pos], COLUNA[pos]
    MOV_CAVALO[pos] = tuple(
        (l + dl) * 8 + (c + dc)
        for dl, dc in ((-2, -1), (-2, 1), (-1, -2), (-1, 2), (1, -2), (1, 2), (2, -1), (2, 1))
        if 0 <= l + dl < 8 and 0 <= c + dc < 8
    )
    MOV_REI[pos] = tuple(
        (l + dl) * 8 + (c + dc)
        for dl in (-1, 0, 1)
        for dc in (-1, 0, 1)
        if (dl, dc) != (0, 0) and 0 <= l + dl < 8 and 0 <= c + dc < 8
    )


def bit_count(mask):
    return mask.bit_count()


def parse_tabuleiro(tabuleiro):
    tipos, posicoes = [], []
    peoes_mask = bloqueios_mask = 0

    for l in range(8):
        for c in range(8):
            casa = tabuleiro[l][c]
            pos = l * 8 + c
            if casa in PECAS_ATIVAVEIS:
                tipos.append(casa)
                posicoes.append(pos)
            elif casa in PECAS_BRANCAS:
                bloqueios_mask |= BIT[pos]
            elif casa == "p":
                peoes_mask |= BIT[pos]

    return tuple(tipos), tuple(posicoes), peoes_mask, bloqueios_mask


def posicoes_mask(posicoes):
    mask = 0
    for pos in posicoes:
        mask |= BIT[pos]
    return mask


def mover_peca(posicoes, indice, destino):
    novas = list(posicoes)
    novas[indice] = destino
    return tuple(novas)


def capturas_linha(peoes_mask, ocupacao_mask, pos, direcoes):
    capturas = []
    for dl, dc in direcoes:
        l, c = LINHA[pos] + dl, COLUNA[pos] + dc
        while 0 <= l < 8 and 0 <= c < 8:
            destino = l * 8 + c
            if peoes_mask & BIT[destino]:
                capturas.append(destino)
                break
            if ocupacao_mask & BIT[destino]:
                break
            l += dl
            c += dc
    return capturas


def capturas_peca(tipos, posicoes, peoes_mask, bloqueios_mask, indice):
    peca = tipos[indice]
    pos = posicoes[indice]

    if peca == "C":
        return [p for p in MOV_CAVALO[pos] if peoes_mask & BIT[p]]

    ocupacao = (bloqueios_mask | posicoes_mask(posicoes)) & ~BIT[pos]
    return capturas_linha(peoes_mask, ocupacao, pos, DIRECOES[peca])


@lru_cache(maxsize=500000)
def capturas_peca_cache(tipos, posicoes, peoes_mask, bloqueios_mask, indice):
    return tuple(capturas_peca(tipos, posicoes, peoes_mask, bloqueios_mask, indice))


@lru_cache(maxsize=200000)
def caminhos_troca(origem, posicoes, peoes_mask, bloqueios_mask):
    bloqueados = peoes_mask | bloqueios_mask
    posicoes_ocupadas = posicoes_mask(posicoes)
    alvos = set(posicoes)
    visitados = {origem}
    fila = deque([(origem, ())])
    encontrados = {}

    while fila and len(encontrados) < len(posicoes) - 1:
        atual, caminho = fila.popleft()
        for vizinho in MOV_REI[atual]:
            if vizinho in visitados or (bloqueados & BIT[vizinho]):
                continue

            novo_caminho = caminho + (vizinho,)
            visitados.add(vizinho)

            if vizinho in alvos and vizinho != origem:
                encontrados[vizinho] = novo_caminho
                continue

            if posicoes_ocupadas & BIT[vizinho]:
                continue

            fila.append((vizinho, novo_caminho))

    return tuple(sorted(encontrados.items()))


def score_captura(tipos, posicoes, peoes_mask, bloqueios_mask, indice, destino):
    novos_peoes = peoes_mask & ~BIT[destino]
    novas_posicoes = mover_peca(posicoes, indice, destino)
    futuras = len(capturas_peca_cache(tipos, novas_posicoes, novos_peoes, bloqueios_mask, indice))
    return (-futuras, bit_count(novos_peoes), destino)


def capturas_por_peca(tipos, posicoes, peoes_mask, bloqueios_mask):
    return tuple(
        capturas_peca_cache(tipos, posicoes, peoes_mask, bloqueios_mask, i)
        for i in range(len(tipos))
    )


def ordenar_trocas(tipos, posicoes, peoes_mask, bloqueios_mask, indice_ativo, caps_por_peca):
    trocas = []
    origem = posicoes[indice_ativo]

    for destino, caminho in caminhos_troca(origem, posicoes, peoes_mask, bloqueios_mask):
        novo_indice = posicoes.index(destino)
        n_caps = len(caps_por_peca[novo_indice])
        if n_caps:
            trocas.append((novo_indice, caminho, n_caps, len(caminho)))

    trocas.sort(key=lambda t: (-t[2], t[3]))
    return [(indice, caminho) for indice, caminho, _caps, _dist in trocas]


def reconstruir_caminho(pais, estado):
    segmentos = []
    while estado is not None:
        estado, segmento = pais[estado]
        segmentos.append(segmento)

    caminho = []
    for segmento in reversed(segmentos):
        caminho.extend(segmento)

    return [pos_para_casa(LINHA[pos], COLUNA[pos]) for pos in caminho]


def heuristica_estado(tipos, posicoes, peoes_mask, bloqueios_mask, indice_ativo):
    restantes = bit_count(peoes_mask)
    if restantes == 0:
        return 0

    caps = capturas_por_peca(tipos, posicoes, peoes_mask, bloqueios_mask)
    if caps[indice_ativo]:
        return restantes

    melhores = caminhos_troca(posicoes[indice_ativo], posicoes, peoes_mask, bloqueios_mask)
    for destino, caminho in melhores:
        novo_indice = posicoes.index(destino)
        if caps[novo_indice]:
            return restantes + len(caminho)

    return restantes + 50


def procurar_gulosa(tipos, posicoes_ini, peoes_ini, bloqueios_mask, limite_ms):
    inicio = time.perf_counter()
    fila, melhor, pais = [], {}, {}
    contador = 0

    for indice, origem in enumerate(posicoes_ini):
        estado = (peoes_ini, posicoes_ini, indice)
        melhor[estado] = 1
        pais[estado] = (None, (origem,))
        heapq.heappush(fila, (heuristica_estado(tipos, posicoes_ini, peoes_ini, bloqueios_mask, indice), 1, contador, estado))
        contador += 1

    while fila:
        if (time.perf_counter() - inicio) * 1000 >= limite_ms:
            return None

        _prio, passos, _cnt, estado = heapq.heappop(fila)
        peoes_mask, posicoes, indice_ativo = estado

        if passos > melhor.get(estado, float("inf")):
            continue
        if peoes_mask == 0:
            return reconstruir_caminho(pais, estado)
        if passos + bit_count(peoes_mask) > 100:
            continue

        caps = capturas_por_peca(tipos, posicoes, peoes_mask, bloqueios_mask)
        sucessores = []

        capturas = sorted(
            caps[indice_ativo],
            key=lambda d: score_captura(tipos, posicoes, peoes_mask, bloqueios_mask, indice_ativo, d),
        )
        for destino in capturas:
            sucessores.append((peoes_mask & ~BIT[destino], mover_peca(posicoes, indice_ativo, destino), indice_ativo, (destino,), 1))

        for novo_indice, caminho in ordenar_trocas(tipos, posicoes, peoes_mask, bloqueios_mask, indice_ativo, caps)[:3]:
            sucessores.append((peoes_mask, posicoes, novo_indice, caminho, len(caminho)))

        for novos_peoes, novas_posicoes, novo_indice, segmento, extra in sucessores:
            novos_passos = passos + extra
            if novos_passos + bit_count(novos_peoes) > 100:
                continue

            novo_estado = (novos_peoes, novas_posicoes, novo_indice)
            if novos_passos >= melhor.get(novo_estado, float("inf")):
                continue

            melhor[novo_estado] = novos_passos
            pais[novo_estado] = (estado, segmento)
            heapq.heappush(
                fila,
                (heuristica_estado(tipos, novas_posicoes, novos_peoes, bloqueios_mask, novo_indice), novos_passos, contador, novo_estado),
            )
            contador += 1

    return []


def procurar_estocastica(tipos, posicoes_ini, peoes_ini, bloqueios_mask, limite_ms):
    inicio = time.perf_counter()

    for seed in range(2000):
        if (time.perf_counter() - inicio) * 1000 >= limite_ms:
            return None

        rng = random.Random(seed)
        indices = list(range(len(tipos)))
        rng.shuffle(indices)

        for indice_inicial in indices:
            posicoes = posicoes_ini
            peoes_mask = peoes_ini
            indice_ativo = indice_inicial
            caminho = [posicoes[indice_ativo]]
            vistos = set()

            while len(caminho) < 100:
                estado = (peoes_mask, posicoes, indice_ativo)
                if estado in vistos:
                    break
                vistos.add(estado)

                if peoes_mask == 0:
                    return [pos_para_casa(LINHA[p], COLUNA[p]) for p in caminho]

                capturas = list(capturas_peca_cache(tipos, posicoes, peoes_mask, bloqueios_mask, indice_ativo))
                if capturas:
                    capturas.sort(key=lambda d: score_captura(tipos, posicoes, peoes_mask, bloqueios_mask, indice_ativo, d))
                    destino = rng.choice(capturas[: min(4, len(capturas))])
                    peoes_mask &= ~BIT[destino]
                    posicoes = mover_peca(posicoes, indice_ativo, destino)
                    caminho.append(destino)
                    continue

                caps = capturas_por_peca(tipos, posicoes, peoes_mask, bloqueios_mask)
                trocas = ordenar_trocas(tipos, posicoes, peoes_mask, bloqueios_mask, indice_ativo, caps)
                if not trocas:
                    break

                indice_ativo, segmento = rng.choice(trocas[: min(5, len(trocas))])
                if len(caminho) + len(segmento) > 100:
                    break
                caminho.extend(segmento)

    return []


def resolver(tabuleiro, limite_ms=None):
    global ULTIMO_ALGORITMO
    ULTIMO_ALGORITMO = ""

    if limite_ms is None:
        limite_ms = 20000

    tipos, posicoes_ini, peoes_ini, bloqueios_mask = parse_tabuleiro(tabuleiro)
    if peoes_ini == 0:
        return []

    inicio = time.perf_counter()
    melhor_solucao = None
    melhor_algoritmo = "sem solução"
    min_teorico = bit_count(peoes_ini)

    def restante():
        return limite_ms - int((time.perf_counter() - inicio) * 1000)

    def guardar(solucao, algoritmo):
        nonlocal melhor_solucao, melhor_algoritmo
        if solucao and (melhor_solucao is None or len(solucao) < len(melhor_solucao)):
            melhor_solucao = solucao
            melhor_algoritmo = algoritmo

    limite = min(1200, restante())
    if limite > 100:
        guardar(procurar_estocastica(tipos, posicoes_ini, peoes_ini, bloqueios_mask, limite), "Estocastica")

    if melhor_solucao and len(melhor_solucao) <= min_teorico + 6:
        ULTIMO_ALGORITMO = melhor_algoritmo
        return melhor_solucao

    limite = min(4000, restante())
    if limite > 100:
        guardar(procurar_gulosa(tipos, posicoes_ini, peoes_ini, bloqueios_mask, limite), "Gulosa best-first")

    if melhor_solucao:
        ULTIMO_ALGORITMO = melhor_algoritmo
        return melhor_solucao

    ULTIMO_ALGORITMO = "sem solução"
    return []