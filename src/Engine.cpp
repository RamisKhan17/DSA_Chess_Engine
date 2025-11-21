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

Move Engine::getBestMove()
{
    std::cout << "Searching for best move at depth " << searchDepth << "...\n";

    nodesSearched = 0;
    std::vector<Move> legalMoves = board.generateLegalMoves();

    if (legalMoves.empty())
        return Move();

    Move bestMove = legalMoves[0];
    int bestScore = std::numeric_limits<int>::min() + 1;
    for (Move &move : legalMoves)
    {
        board.makeMove(move);
        int score = -alphaBeta(searchDepth - 1,
                               std::numeric_limits<int>::min() + 1,
                               std::numeric_limits<int>::max());

        // int score = -search(searchDepth);
        board.undoMove();

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }
    }

    std::cout << "Best move: " << bestMove.toAlgebraic()
              << " (score: " << bestScore << ")\n";
    std::cout << "Nodes searched: " << nodesSearched << "\n";

    return bestMove;
}

// int Engine::evaluate()
// {

//     int score = 0;

//     // Piece values in centipawns
//     const int PAWN_VALUE = 100;
//     const int KNIGHT_VALUE = 300;
//     const int BISHOP_VALUE = 325;
//     const int ROOK_VALUE = 500;
//     const int QUEEN_VALUE = 900;

//     for (int sq = 0; sq < 128; sq++)
//     {
//         if (sq & 0x88)
//             continue;

//         int piece = board.getPiece(sq);
//         if (piece == EMPTY)
//             continue;

//         int value = 0;
//         switch (std::abs(piece))
//         {
//         case 1:
//             value = PAWN_VALUE;
//             break;
//         case 2:
//             value = KNIGHT_VALUE;
//             break;
//         case 3:
//             value = BISHOP_VALUE;
//             break;
//         case 4:
//             value = ROOK_VALUE;
//             break;
//         case 5:
//             value = QUEEN_VALUE;
//             break;
//         case 6:
//             value = 0;
//             break; // King has no material value
//         }

//         if (piece > 0)
//         {
//             score += value;
//         }
//         else
//         {
//             score -= value;
//         }
//     }
//     if (board.getSideToMove() == 1)
//         score = -score;
//     return score;
// }

int Engine::search(int depth)
{
    nodesSearched++;
    if (board.isCheckmate())
        return -100000 + (searchDepth - depth); // Favor quicker mates

    if (board.isStalemate() || board.isDraw())
        return 0;

    if (depth <= 0)
        return evaluate();

    vector<Move> moves = board.generateLegalMoves();
    int bestEval = -100000;
    for (Move &move : moves)
    {
        board.makeMove(move);
        int evaluate = -search(depth - 1);
        bestEval = max(evaluate, bestEval);
        board.undoMove();
    }

    return bestEval;
}

int Engine::alphaBeta(int depth, int alpha, int beta)
{
    nodesSearched++;
    if (board.isCheckmate())
        return -100000 + (searchDepth - depth); // Favor quicker mates

    if (board.isStalemate() || board.isDraw())
        return 0;

    if (depth <= 0)
        return evaluate();

    vector<Move> moves = board.generateLegalMoves();
    for (Move &move : moves)
    {
        board.makeMove(move);
        int evaluation = -alphaBeta(depth - 1, -beta, -alpha);
        board.undoMove();
        if (evaluation >= beta)
            return beta;
        alpha = max(alpha, evaluation);
    }

    return alpha;
}
