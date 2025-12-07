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

inline int Engine::getPieceValue(int piece)
{
    switch (std::abs(piece))
    {
    case 1:
        return 100; // Pawn
    case 2:
        return 320; // Knight
    case 3:
        return 330; // Bishop
    case 4:
        return 500; // Rook
    case 5:
        return 900; // Queen
    case 6:
        return 20000; // King (very high)
    default:
        return 0;
    }
}

void Engine::sortMovesWithOrdering(std::vector<Move> &moves)
{
    for (Move &m : moves)
    {
        int score = 0;

        if (best_move == m)
            score += 1000000;

        // MVV-LVA for captures
        if (m.capturedPiece != 0)
        {
            int victimVal = getPieceValue(m.capturedPiece);
            int attackerVal = getPieceValue(m.piece);
            score += victimVal * 100 - attackerVal + 100000; // Offset to put captures above quiet moves
        }

        // TT Entry Check
        uint64_t moveHash = board.moveHash(board.hash, m);
        TTEntry ent = TT[moveHash & (TT_SIZE - 1)];
        if (ent.key == moveHash)
            score += 50000;

        // Promotions bonus
        if (m.flags & FLAG_PROMOTION)
            score += getPieceValue(m.promotionPiece) * 100;

        m.score = score;
    }

    std::sort(moves.begin(), moves.end(), [](const Move &a, const Move &b)
              { return a.score > b.score; });
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
    int best_score = TTIterativeDeepening(searchDepth);
    std::cout << "Best move: " << best_move.toAlgebraic()
              << "(score: " << best_score << " )\n";
    std::cout << "Nodes searched: " << nodesSearched << "\n";
    return best_move;
}

Move Engine::getBestMove()
{
    return getIterativeDeepeningMove(); // Change with getAlphaBetaMove for other implementation
}
