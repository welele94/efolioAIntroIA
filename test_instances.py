"""Run all predefined instances and print a compact summary."""

import argparse

from game_state import create_state_from_board_string
from instances import INSTANCES, get_instance
from minimax import minimax_decision


def play_instance(name: str, depth: int):
    state = create_state_from_board_string(get_instance(name))

    while not state.is_terminal():
        move = minimax_decision(state, depth=depth)
        if move is None:
            break
        state = state.apply_move(move)

    return state


def parse_args():
    parser = argparse.ArgumentParser(description="Corre todas as instancias de teste.")
    parser.add_argument("--depth", type=int, default=2, help="Profundidade do Minimax.")
    return parser.parse_args()


def main():
    args = parse_args()

    print(f"{'Instancia':<16} {'A':>3} {'V':>3} {'Acoes':>6} {'Vencedor':>8}")
    print("-" * 42)
    for name in sorted(INSTANCES):
        state = play_instance(name, args.depth)
        print(
            f"{name:<16} {state.a_captures:>3} {state.v_captures:>3} "
            f"{state.action_count:>6} {state.get_winner():>8}"
        )


if __name__ == "__main__":
    main()
