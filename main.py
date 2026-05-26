"""Entry point to simulate one full adversarial game."""

import argparse
import sys

from board import coord_to_notation, print_board
from game_state import create_state_from_board_string
from instances import INSTANCES, get_instance
from minimax import minimax_decision

MAX_DEPTH = 2
PRINT_EACH_TURN = True


def render_with_agents(state):
    board = [row[:] for row in state.board]
    ar, ac = state.a_pos
    vr, vc = state.v_pos
    board[ar][ac] = "A"
    board[vr][vc] = "V"
    return board


def parse_args():
    parser = argparse.ArgumentParser(description="Simula o jogo adversarial do eFolio B.")
    parser.add_argument(
        "--instance",
        default="official",
        choices=sorted(INSTANCES),
        help="Instancia inicial a usar.",
    )
    parser.add_argument("--depth", type=int, default=MAX_DEPTH, help="Profundidade do Minimax.")
    parser.add_argument("--time-limit", type=float, default=1.0, help="Limite de tempo da procura, em segundos.")
    parser.add_argument("--player", choices=["A", "V"], default="A", help="Jogador a mover quando usa --board.")
    parser.add_argument(
        "--board",
        help="String externa de 64 caracteres com o estado atual. Use '-' para ler de stdin.",
    )
    parser.add_argument("--quiet", action="store_true", help="Nao imprime o tabuleiro a cada turno.")
    parser.add_argument("--list-instances", action="store_true", help="Lista as instancias disponiveis.")
    return parser.parse_args()


def choose_move_destination(state, depth: int, time_limit: float) -> str:
    if state.is_terminal():
        return coord_to_notation(state.a_pos if state.current_player == "A" else state.v_pos)

    move = minimax_decision(state, depth=depth, time_limit=time_limit)
    if move is None:
        return coord_to_notation(state.a_pos if state.current_player == "A" else state.v_pos)
    return coord_to_notation(move.to_pos)


def main():
    args = parse_args()

    if args.list_instances:
        print("Instancias disponiveis:")
        for name in sorted(INSTANCES):
            print(f"- {name}")
        return

    if args.board is not None:
        board_string = sys.stdin.read().rstrip("\n") if args.board == "-" else args.board
        state = create_state_from_board_string(board_string, current_player=args.player)
        print(choose_move_destination(state, args.depth, args.time_limit))
        return

    state = create_state_from_board_string(get_instance(args.instance))
    history = []
    print_each_turn = PRINT_EACH_TURN and not args.quiet

    if print_each_turn:
        print(f"Estado inicial: {args.instance}")
        print_board(render_with_agents(state))

    while not state.is_terminal():
        move = minimax_decision(state, depth=args.depth, time_limit=args.time_limit)
        if move is None:
            break
        history.append(f"{state.current_player}: {move.notation}")
        state = state.apply_move(move)

        if print_each_turn:
            print(f"\nAção {state.action_count}: {history[-1]}")
            print_board(render_with_agents(state))

    print("\n=== Resultado final ===")
    print("Sequência de jogadas:")
    for idx, item in enumerate(history, start=1):
        print(f"{idx:02d}. {item}")

    print(f"\nCapturas A (verde): {state.a_captures}")
    print(f"Capturas V (vermelho): {state.v_captures}")
    print(f"Total de ações: {state.action_count}")
    print(f"Vencedor: {state.get_winner()}")


if __name__ == "__main__":
    main()
