def ler_instancia(caminho):
    # Abrir com utf-8-sig evita problemas caso o ficheiro tenha BOM.
    with open(caminho, "r", encoding="utf-8-sig") as ficheiro:
        conteudo = ficheiro.read()

    # As casas vazias são espaços e têm de ser preservadas.
    # Por isso removem-se apenas quebras de linha, incluindo CRLF do Windows.
    conteudo = conteudo.replace("\n", "").replace("\r", "")

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
