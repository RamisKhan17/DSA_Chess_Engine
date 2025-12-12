#include "include/Board.h"
#include "include/Engine.h"
#include "include/GUI.h"
#include <iostream>

/**
 * Chess Engine - DSA Project
 *
 * This is the main entry point for the chess engine application.
 * It initializes the board and GUI, then runs the main game loop.
 *
 * Features:
 * - 0x88 board representation for efficient move generation
 * - Complete move generation including special moves (castling, en passant, promotion)
 * - Legal move filtering with check detection
 * - SFML-based graphical user interface
 * - Drag and drop piece movement
 * - Move history and game status display
 */

int main(int argc, char **argv)
{
    cout << "Chess Engine - DSA Project\n";
    cout << "==========================\n\n";
    cout << "Initializing chess engine...\n";

    int START_TIME = 60000;    // In milliseconds
    int TIME_INCREMENT = 1000; // In milliseconds
    int MATCH_TYPE = -1;
    int MAX_DEPTH = 4;
    searchType SEARCH_FUNC = TT_ITERATIVE_DEEPENING_AW;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-h")
        {
            cout << "Chess Engine Command Line Options:\n\n";
            cout << "  -t <ms>   Set start time per side (default: 60000 ms)\n";
            cout << "  -i <ms>   Set increment time per move (default: 1000 ms)\n";
            cout << "  -d <n>    Set maximum search depth (default: 4)\n";
            cout << "  -m <n>    Match type (default: -1)\n";
            cout << "            0 = Human vs Human\n";
            cout << "            1 = Engine plays White\n";
            cout << "           -1 = Engine plays Black\n";
            cout << "            2 = Engine vs Engine\n";
            cout << "  -f <n>    Search function (default: 4)\n";
            cout << "            0 = STANDARD\n";
            cout << "            1 = ALPHA_BETA\n";
            cout << "            2 = ITERATIVE_DEEPENING\n";
            cout << "            3 = TT_ITERATIVE_DEEPENING\n";
            cout << "            4 = TT_ITERATIVE_DEEPENING + ASPIRATION WINDOWS\n";
            cout << "\nExample:\n";
            cout << "  ./chess.exe -t 30000 -i 500 -d 5 -m 1 -f 4\n";
            return 0;
        }
        else if (arg == "-t" && i + 1 < argc)
            START_TIME = stoi(argv[++i]);
        else if (arg == "-d" && i + 1 < argc)
            MAX_DEPTH = stoi(argv[++i]);
        else if (arg == "-i" && i + 1 < argc)
            TIME_INCREMENT = stoi(argv[++i]);
        else if (arg == "-m" && i + 1 < argc)
        {
            int mode = stoi(argv[++i]);
            if (mode == 0 || mode == 1 || mode == -1 || mode == 2)
                MATCH_TYPE = mode;
            else
            {
                cerr << "Invalid value for -m. Use -h for help.\n";
                return 1;
            }
        }
        else if (arg == "-f" && i + 1 < argc)
        {
            int func = stoi(argv[++i]);
            if (func >= 0 && func <= 4)
                SEARCH_FUNC = static_cast<searchType>(func);
            else
            {
                cerr << "Invalid value for -f. Use -h for help.\n";
                return 1;
            }
        }
        else
        {
            cerr << "Unknown or incomplete argument: " << arg << ". Use -h for help.\n";
            return 1;
        }
    }

    try
    {
        // Create board and set starting position
        Board board;

        board.setStartingPosition();
        Engine e1 = Engine(board, SEARCH_FUNC, MAX_DEPTH, START_TIME, TIME_INCREMENT);
        Engine e2 = Engine(board, SEARCH_FUNC, MAX_DEPTH, START_TIME, TIME_INCREMENT);
        // Test move generation
        vector<Move> legalMoves = board.generateLegalMoves();
        // Create and run GUI
        ChessGUI gui(board, MATCH_TYPE, e1, e2, START_TIME, TIME_INCREMENT);
        gui.run();
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    cout << "\nThank you for using Chess Engine!\n";
    return 0;
}
