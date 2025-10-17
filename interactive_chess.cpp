#include "include/Board.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

/**
 * Interactive Console Chess
 * Play chess in the terminal - no SFML required!
 */

void printHelp() {
    std::cout << "\n=== Chess Engine - Console Mode ===\n";
    std::cout << "Commands:\n";
    std::cout << "  move <from><to>  - Make a move (e.g., 'move e2e4')\n";
    std::cout << "  undo            - Undo last move\n";
    std::cout << "  reset           - Reset to starting position\n";
    std::cout << "  fen <position>  - Load position from FEN\n";
    std::cout << "  moves           - Show all legal moves\n";
    std::cout << "  help            - Show this help\n";
    std::cout << "  quit            - Exit\n\n";
}

void showMoves(Board& board) {
    std::vector<Move> moves = board.generateLegalMoves();
    std::cout << "\nLegal moves (" << moves.size() << "):\n";
    
    for (int i = 0; i < moves.size(); i++) {
        std::cout << moves[i].toAlgebraic();
        if ((i + 1) % 8 == 0) std::cout << "\n";
        else std::cout << " ";
    }
    std::cout << "\n\n";
}

bool makeMoveFromString(Board& board, const std::string& moveStr) {
    if (moveStr.length() < 4) return false;
    
    std::string from = moveStr.substr(0, 2);
    std::string to = moveStr.substr(2, 2);
    
    int fromSquare = algebraicToSquare(from);
    int toSquare = algebraicToSquare(to);
    
    if (fromSquare < 0 || toSquare < 0) return false;
    
    std::vector<Move> moves = board.getLegalMovesFrom(fromSquare);
    
    for (const Move& move : moves) {
        if (move.to == toSquare) {
            board.makeMove(move);
            return true;
        }
    }
    
    return false;
}

int main() {
    std::cout << "♟️  Chess Engine - Interactive Console Mode\n";
    std::cout << "==========================================\n\n";
    
    Board board;
    board.setStartingPosition();
    
    printHelp();
    board.print();
    
    std::string input;
    while (true) {
        std::cout << "\n" << (board.getSideToMove() == 0 ? "White" : "Black") << " to move: ";
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        // Convert to lowercase
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
        
        if (input == "quit" || input == "exit") {
            std::cout << "Thanks for playing!\n";
            break;
        }
        else if (input == "help") {
            printHelp();
        }
        else if (input == "reset") {
            board.setStartingPosition();
            std::cout << "Board reset to starting position.\n";
            board.print();
        }
        else if (input == "undo") {
            if (board.getFullMoveNumber() > 1 || board.getSideToMove() == 1) {
                board.undoMove();
                std::cout << "Move undone.\n";
                board.print();
            } else {
                std::cout << "No moves to undo.\n";
            }
        }
        else if (input == "moves") {
            showMoves(board);
        }
        else if (input.substr(0, 4) == "fen ") {
            std::string fen = input.substr(4);
            if (board.setFEN(fen)) {
                std::cout << "Position loaded from FEN.\n";
                board.print();
            } else {
                std::cout << "Invalid FEN string.\n";
            }
        }
        else if (input.substr(0, 5) == "move ") {
            std::string moveStr = input.substr(5);
            if (makeMoveFromString(board, moveStr)) {
                std::cout << "Move made: " << moveStr << "\n";
                board.print();
                
                // Check for game end
                if (board.isCheckmate()) {
                    std::cout << "\n🎉 CHECKMATE! " 
                              << (board.getSideToMove() == 0 ? "Black" : "White") 
                              << " wins!\n";
                } else if (board.isStalemate()) {
                    std::cout << "\n🤝 STALEMATE! Game is a draw.\n";
                } else if (board.isCheck(board.getSideToMove())) {
                    std::cout << "\n⚠️  CHECK!\n";
                }
            } else {
                std::cout << "Invalid move: " << moveStr << "\n";
                std::cout << "Use format: move e2e4\n";
            }
        }
        else {
            std::cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }
    
    return 0;
}
