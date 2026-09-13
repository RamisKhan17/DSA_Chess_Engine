## Setup and Execution

- Required Library: Install **SFML** (tested with SFML-3.0.2). The GUI requires SFML headers and libs.
- Build: use `chess.bat` (Windows) which sets `SFML_INCLUDE`/`SFML_LIB` and compiles the project.
- Run: run the produced `chess.exe`.

## Command-Line Flags

The following flags can be used when running chess.exe:

> **NOTE:** These flags are not necessary. Simply running chess.exe will be provide a good enough implementation of the engine.

- **`-t <ms>`** Set start time per side (default: 60000 ms)
- **`-i <ms>`** Set increment time per move (default: 0 ms)
- **`-d <n>`** Set maximum search depth (default: 5)
- **`-m <n>`** Match type (default: -1)
  - `0` = Human vs Human
  - `1` = Engine plays White
  - `-1` = Engine plays Black
  - `2` = Engine vs Engine
- **`-f <n>`** Search function (default: 4)
  - `0` = STANDARD
  - `1` = ALPHA_BETA
  - `2` = ITERATIVE_DEEPENING
  - `3` = TT_ITERATIVE_DEEPENING
  - `4` = TT_ITERATIVE_DEEPENING + ASPIRATION WINDOWS

## Example

```bash
./chess.exe -t 30000 -i 500 -d 5 -m 1 -f 4
```

> **(Engine vs Engine)**
>
> **NOTE:** While running Engine vs Engine, do not click anywhere as clicking will cause the game window to stop responding and you have to restart the program. Use Ctrl+C in the terminal to close the unresponsive game window smoothly.

## Contribution

- **Muhammad Ramis Khan (14_B):** Supervision, Debugging, Zobrist Hashing, Transposition Tables
- **Muhammad Abdullah (14_B):** GUI, Board & Moves Implementation, Timer
- **Anas Rizwan (14_B):** Evaluation Function, Perft testing
- **Muhammad Moazzam Ali (14_A):** Searching and Pruning, Move ordering
