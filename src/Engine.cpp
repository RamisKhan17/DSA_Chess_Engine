#include "../include/Engine.h"
#include <algorithm>
#include <limits>
#include <iostream>
using namespace std;
/**
 * Engine Implementation
 *
 * This is a basic placeholder implementation.
 * Teammates should extend this with proper search and evaluation.
 */

Engine::Engine(Board &b, int depth) : board(b), searchDepth(depth), nodesSearched(0)
{
}

void Engine::setPosition(const std::string &fen)
{
    board.setFEN(fen);
}

Move Engine::getAlphaBetaMove()
{
    nodesSearched = 0;
    std::vector<Move> legalMoves = board.generateLegalMoves();
    if (legalMoves.empty())
        return Move();

    Move bestMove = legalMoves[0];
    int alpha = std::numeric_limits<int>::min() + 1;
    int beta = std::numeric_limits<int>::max();

    for (Move &move : legalMoves)
    {
        board.makeMove(move);
        int score = -alphaBeta(searchDepth - 1, -beta, -alpha);
        board.undoMove();

        if (score > alpha)
        {
            alpha = score;
            bestMove = move;
        }
    }

    std::cout << "Best move: " << bestMove.toAlgebraic()
              << " (score: " << alpha << ")\n";
    std::cout << "Nodes searched: " << nodesSearched << "\n";

    return bestMove;
}

Move Engine::getIterativeDeepeningMove()
{
    nodesSearched = 0;
    int best_score = iterativeDeepening(searchDepth);
    if (best_validity)
    {
        std::cout << "Best move: " << best_move.toAlgebraic()
                  << "(score: " << best_score << " )\n";
        std::cout << "Nodes searched: " << nodesSearched << "\n";
        return best_move;
    }
}

Move Engine::getBestMove()
{
    return getIterativeDeepeningMove(); // Change with getAlphaBetaMove for other implementation
}