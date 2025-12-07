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

int ENGINE_DEPTH = 5;

int main()
{
    std::cout << "Chess Engine - DSA Project\n";
    std::cout << "==========================\n\n";
    std::cout << "Initializing chess engine...\n";

    try
    {
        // Create board and set starting position
        Board board;

        board.setStartingPosition();
        Engine engine(board, ENGINE_DEPTH);
        // Test move generation
        std::vector<Move> legalMoves = board.generateLegalMoves();
        // Create and run GUI
        ChessGUI gui(board, engine, 0);
        gui.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nThank you for using Chess Engine!\n";
    return 0;
}
