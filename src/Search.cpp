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
    if (moves.empty())
        return 0;
    int maxEval = -1000000; // Negative infinity
    for (Move &move : moves)
    {
        board.makeMove(move);
        int evaluation = -alphaBeta(depth - 1, -beta, -alpha);
        board.undoMove();
        if (evaluation >= beta)
            return beta;

        if (evaluation > maxEval)
            maxEval = evaluation;
        if (evaluation > alpha)
            alpha = evaluation;
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

int Engine::iterativeDeepening(int maxDepth)
{
    int bestScore = std::numeric_limits<int>::min() + 1;

    for (int depth = 1; depth <= maxDepth; depth++)
    {
        nodesSearched = 0;
        std::vector<Move> legalMoves = board.generateLegalMoves();
        if (legalMoves.empty())
            return 0; // No legal moves

        // Move ordering: put previous best move first
        if (depth != 1)
        {
            auto it = find(legalMoves.begin(), legalMoves.end(), best_move);
            if (it != legalMoves.end())
                swap(*it, legalMoves[1]);
        }

        for (auto &move : legalMoves)
        {
            if (TT[board.moveHash(board.hash, move) & (TT_SIZE - 1)].key == board.moveHash(board.hash, move))
            {
                swap(move, legalMoves[0]);
                break;
            }
        }

        int alpha = std::numeric_limits<int>::min() + 1;
        int beta = std::numeric_limits<int>::max();
        int currentBestScore = std::numeric_limits<int>::min() + 1;

        for (Move &move : legalMoves)
        {
            // cout << move.toAlgebraic() << " ";
            board.makeMove(move);
            int score = -alphaBetaSigma(depth - 1, -beta, -alpha);
            board.undoMove();

            if (score > currentBestScore)
            {
                currentBestScore = score;
                best_move = move;
            }

            if (score > alpha)
                alpha = score;
        }

        bestScore = currentBestScore; // store best score at this depth
    }

    return bestScore;
}

int Engine::alphaBetaPlusPlus(int depth, int max_depth, int alpha, int beta)
{
    nodesSearched++;
    int originalAlpha = alpha;

    TTEntry &entry = TT[ttIndex(board.hash)];
    if (entry.key == board.hash)
    {
        if (entry.depth >= depth)
        {
            if (entry.flag == FLAG_EXACT)
                return entry.score;
            if (entry.flag == FLAG_LOWERBOUND)
                alpha = std::max(alpha, entry.score);
            else if (entry.flag == FLAG_UPPERBOUND)
                beta = std::min(beta, entry.score);
            if (alpha >= beta)
                return entry.score;
        }
    }

    if (board.isCheckmate())
        return -100000 + (max_depth - depth);

    if (board.isStalemate() || board.isDraw())
        return 0;

    if (depth <= 0)
        return evaluate();

    std::vector<Move> moves = board.generateLegalMoves();

    for (Move &move : moves)
    {
        board.makeMove(move);
        int evaluation = -alphaBetaPlusPlus(depth - 1, max_depth, -beta, -alpha);
        board.undoMove();

        if (evaluation >= beta)
        {
            TTEntry store;
            store.key = board.hash;
            store.depth = depth;
            store.score = evaluation;
            store.flag = FLAG_LOWERBOUND;
            TT[ttIndex(board.hash)] = store;
            return beta;
        }

        if (evaluation > alpha)
        {
            alpha = evaluation;
            if (depth == max_depth)
                best_move = move;
        }
    }

    TTEntry store;
    store.key = board.hash;
    store.depth = max_depth - depth; // root-relative depth
    store.score = alpha;

    if (alpha <= originalAlpha)
        store.flag = FLAG_UPPERBOUND;
    else if (alpha >= beta)
        store.flag = FLAG_LOWERBOUND;
    else
        store.flag = FLAG_EXACT;

    TT[ttIndex(board.hash)] = store;
    count++;
    return alpha;
}

int Engine::iterativeDeepeningAW(int max_depth)
{
    int best_score = 0;
    const int ASPIRATION_WINDOW = 100;
    best_validity = false;

    for (int d = 1; d <= max_depth; d++)
    {
        int alpha = best_score - ASPIRATION_WINDOW;
        int beta = best_score + ASPIRATION_WINDOW;

        int score = alphaBetaPlus(d, d, alpha, beta);

        if (score <= alpha)
            score = alphaBetaPlus(d, d, -1000000, beta);
        else if (score >= beta)
            score = alphaBetaPlus(d, d, alpha, 1000000);

        best_score = score;
        best_validity = true;
    }

    return best_score;
}

int Engine::alphaBetaSigma(int depth, int alpha, int beta)
{
    nodesSearched++;
    int originalAlpha = alpha;
    TTEntry &entry = TT[board.hash & (TT_SIZE - 1)];
    if (entry.key == board.hash && entry.depth >= depth)
    {
        if (entry.flag == FLAG_EXACT)
            return entry.score;
        else if (entry.flag == FLAG_LOWERBOUND)
            alpha = max(alpha, entry.score);
        else if (entry.flag == FLAG_UPPERBOUND)
            beta = min(beta, entry.score);
        if (alpha >= beta)
            return entry.score;
    }

    if (board.isCheckmate())
        return -100000 + (searchDepth - depth); // Favor quicker mates
    if (board.isStalemate() || board.isDraw())
        return 0;
    if (depth <= 0)
        return evaluate();
    vector<Move> moves = board.generateLegalMoves();
    if (moves.empty())
        return 0;
    int maxEval = -10000000; // Negative infinity
    for (Move &move : moves)
    {
        // cout << move.toAlgebraic() << " ";
        board.makeMove(move);
        int evaluation = -alphaBetaSigma(depth - 1, -beta, -alpha);
        board.undoMove();

        if (evaluation > maxEval)
        {
            maxEval = evaluation;
        }
        if (evaluation > alpha)
            alpha = evaluation;
        if (alpha >= beta)
            break;
    }

    entry.depth = depth;
    entry.key = board.hash;
    entry.score = maxEval;
    if (maxEval <= originalAlpha)
        entry.flag = FLAG_UPPERBOUND;
    else if (maxEval >= beta)
        entry.flag = FLAG_LOWERBOUND;
    else
        entry.flag = FLAG_EXACT;

    TT[board.hash & (TT_SIZE - 1)] = entry;
    return maxEval;
}

// int Engine::iterativeDeepening(int max_depth)
// {
//     int best_score = -1000000;
//     best_validity = false;
//     for (int i = 1; i <= max_depth; i++)
//     {
//         best_score = alphaBetaPlus(i, i, -1000000, 1000000);
//         best_validity = true;
//     }

//     return best_score;
// }
