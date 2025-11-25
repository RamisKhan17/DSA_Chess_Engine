#include "../include/Engine.h"
#include <iostream>
#include <algorithm>
#include <limits>

using namespace std;

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

int Engine::alphaBetaPlus(int depth, int max_depth, int alpha, int beta)
{
    nodesSearched++;
    if (board.isCheckmate())
        return -100000 + (max_depth - depth); // Favor quicker mates

    if (board.isStalemate() || board.isDraw())
        return 0;

    if (depth <= 0)
        return evaluate();

    vector<Move> moves = board.generateLegalMoves();
    if (best_validity)
    {
        auto it = std::find(moves.begin(), moves.end(), best_move);
        if (it != moves.end())
            swap(moves[0], *it);
    }
    for (Move &move : moves)
    {
        board.makeMove(move);
        int evaluation = -alphaBetaPlus(depth - 1, max_depth, -beta, -alpha);
        board.undoMove();
        if (evaluation >= beta)
            return beta;
        if (evaluation > alpha)
        {
            alpha = evaluation;
            if (depth == max_depth)
                best_move = move;
        }
    }
    return alpha;
}

int Engine::iterativeDeepening(int max_depth)
{
    int best_score = -1000000;
    best_validity = false;
    for (int i = 1; i <= max_depth; i++)
    {
        best_score = alphaBetaPlus(i, i, -1000000, 1000000);
        best_validity = true;
    }

    return best_score;
}
