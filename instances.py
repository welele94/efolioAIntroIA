"""Named board instances for testing the adversarial search agent."""

from __future__ import annotations

from board import INITIAL_BOARD_STRING


def _board(rows: list[str]) -> str:
    if len(rows) != 8:
        raise ValueError("An instance must have exactly 8 rows")
    if any(len(row) != 8 for row in rows):
        raise ValueError("Each instance row must have exactly 8 columns")
    return "".join(rows)


INSTANCES = {
    "official": INITIAL_BOARD_STRING,
    "open_race": _board(
        [
            "p  T  p ",
            "   p    ",
            "  A     ",
            "        ",
            "     V  ",
            "    p   ",
            " p  D  p",
            "        ",
        ]
    ),
    "capture_race": _board(
        [
            "p      p",
            "   T    ",
            "  A     ",
            "        ",
            "     V  ",
            "    B   ",
            "p      p",
            "   p    ",
        ]
    ),
    "knight_tactics": _board(
        [
            "p      p",
            "   p    ",
            "  C     ",
            "  A     ",
            "        ",
            "     V  ",
            "     C  ",
            "p      p",
        ]
    ),
    "queen_pressure": _board(
        [
            "p   p   ",
            "        ",
            "  A D   ",
            "        ",
            "        ",
            "   D V  ",
            "        ",
            "   p   p",
        ]
    ),
    "endgame_small": _board(
        [
            "        ",
            "  A T p ",
            "        ",
            "        ",
            "        ",
            " p B V  ",
            "        ",
            "        ",
        ]
    ),
}


def get_instance(name: str) -> str:
    try:
        return INSTANCES[name]
    except KeyError as exc:
        available = ", ".join(sorted(INSTANCES))
        raise ValueError(f"Unknown instance '{name}'. Available: {available}") from exc
