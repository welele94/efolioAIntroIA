# eFólio B — Introdução à Inteligência Artificial

Este projeto é uma **evolução do eFólio A**.

- No **eFólio A**, o problema era de procura clássica com um único agente.
- No **eFólio B**, o problema passa a **procura adversa**, com dois agentes (A e V) a competir por capturas de peões pretos.

## Estrutura

- `main.py`: ciclo principal do jogo e impressão do resultado.
- `board.py`: parsing do tabuleiro, coordenadas e utilitários de impressão.
- `game_state.py`: classe `GameState`, aplicação de jogadas e condições de terminal.
- `moves.py`: geração de movimentos legais (rei, capturas em peça, saída da peça e pass).
- `evaluation.py`: função de avaliação com pesos ajustáveis.
- `minimax.py`: Minimax com poda alfa-beta e ordenação simples de jogadas.
- `instances.py`: conjunto de instâncias alternativas para testar o agente.

## Como correr

```bash
python main.py
```

Também é possível escolher uma instância, a profundidade e limitar a procura:

```bash
python main.py --list-instances
python main.py --instance open_race --depth 2
python main.py --instance knight_tactics --depth 2 --time-limit 1.0 --quiet
```

Para correr todas as instâncias e comparar resultados:

```bash
python test_instances.py --depth 2
```

## Modo avaliador

Além da instância inicial, o programa aceita um tabuleiro externo completo com exatamente 64 caracteres.
Nesse modo, o estado é construído diretamente a partir do tabuleiro recebido, são identificadas as posições de `A` e `V`, e a saída é apenas a casa de destino da jogada escolhida.

```bash
python main.py --board "<string de 64 caracteres>" --player A
```

Também pode ler a string por `stdin`:

```bash
python -c "from instances import get_instance; print(get_instance('open_race'), end='')" | python main.py --board - --player A
```

Por defeito, a procura usa profundidade `2` e limite de tempo de `1.0` segundo, para evitar avaliações demasiado demoradas.

## Instâncias de teste

O ficheiro `instances.py` contém cenários pequenos e controlados para testar partes diferentes do algoritmo:

- `official`: instância oficial do enunciado.
- `open_race`: tabuleiro mais aberto para comparar mobilidade e corrida aos peões.
- `capture_race`: peças fáceis de alcançar para testar entrada em peça e capturas.
- `knight_tactics`: cenário focado em movimentos de cavalo.
- `queen_pressure`: cenário com damas para testar linhas, colunas e diagonais.
- `endgame_small`: cenário curto para testar finais rápidos.

## Minimax, alfa-beta e avaliação

O agente escolhe jogadas com Minimax de profundidade limitada.

- **Minimax**: modela o jogo como alternância entre jogador maximizador (A) e minimizador (V).
- **Poda alfa-beta**: corta ramos que não podem melhorar o resultado, reduzindo custo de pesquisa.
- **Função de avaliação**: combina diferença de capturas, mobilidade e proximidade a peões pretos.

A base foi feita para ser simples de evoluir, sobretudo na heurística (`evaluation.py`) e no ajuste de profundidade (`main.py`).
