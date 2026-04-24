def ler_instancia(caminho):
    with open(caminho, "r", encoding="utf-8") as ficheiro:
        conteudo = ficheiro.read()

    conteudo = conteudo.replace("\n", "")

    if len(conteudo) != 64:
        raise ValueError(f"A instância deve ter 64 caracteres, mas tem {len(conteudo)}.")

    tabuleiro = []

    for i in range(8):
        linha = list(conteudo[i * 8:(i + 1) * 8])
        tabuleiro.append(linha)

    return tabuleiro

def pos_para_casa(linha, coluna):
    letras = "abcdefgh"
    return letras[coluna] + str(linha + 1)