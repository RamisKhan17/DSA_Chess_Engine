#include "../include/Engine.h"
#include <iostream>
#include <algorithm>
#include <limits>

using namespace std;

int Engine::search(int depth)
{
    nodesSearched++;
    if (board.isCheckmate())
        return -100000 + (searchDepth - depth);

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
    if ((moveTime != -1) && (nodesSearched & (remainingTime / 275)) == 0)
    {
        if (chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - startTime)
                .count() >= moveTime)
        {
            searchCancelled = true;
            return alpha;
        }
    }
    if (board.isCheckmate())
        return -100000 + (searchDepth - depth);
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

int Engine::TTAlphaBeta(int depth, int alpha, int beta)
{
    nodesSearched++;
    if ((nodesSearched & (remainingTime / 275)) == 0)
    {
        if (chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - startTime)
                .count() >= moveTime)
        {
            searchCancelled = true;
            return alpha;
        }
    }
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
        return -100000 + (searchDepth - depth);
    if (board.isStalemate() || board.isDraw())
        return 0;
    if (depth <= 0)
        return evaluate();
    vector<Move> moves = board.generateLegalMoves();
    if (moves.empty())
        return 0;

    sortMovesWithOrdering(moves);
    int maxEval = -10000000;
    for (Move &move : moves)
    {
        board.makeMove(move);
        int evaluation = -TTAlphaBeta(depth - 1, -beta, -alpha);
        board.undoMove();
        if (searchCancelled)
            break;

        if (evaluation > maxEval)
        {
            maxEval = evaluation;
        }
        if (evaluation > alpha)
            alpha = evaluation;
        if (alpha >= beta)
            break;
    }
    if (searchCancelled)
        return maxEval;

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

int Engine::iterativeDeepening(int maxDepth)
{
    int bestScore = numeric_limits<int>::min() + 1;

    nodesSearched = 0;
    for (int depth = 1; depth <= maxDepth; depth++)
    {
        vector<Move> legalMoves = board.generateLegalMoves();
        if (legalMoves.empty())
            return 0;

        sortMovesWithOrdering(legalMoves);

        int alpha = numeric_limits<int>::min() + 1;
        int beta = numeric_limits<int>::max();
        int currentBestScore = numeric_limits<int>::min() + 1;

        for (Move &move : legalMoves)
        {
            board.makeMove(move);
            int score = -alphaBeta(depth - 1, -beta, -alpha);
            board.undoMove();

            if (searchCancelled)
                break;

            if (score > currentBestScore)
            {
                currentBestScore = score;
                best_move = move;
            }

            if (score > alpha)
                alpha = score;
        }
        if (searchCancelled)
            break;
        bestScore = currentBestScore;
    }

    return bestScore;
}

int Engine::TTIterativeDeepening(int maxDepth)
{
    int bestScore = numeric_limits<int>::min() + 1;

    nodesSearched = 0;
    for (int depth = 1; depth <= maxDepth; depth++)
    {
        vector<Move> legalMoves = board.generateLegalMoves();
        if (legalMoves.empty())
            return 0;

        sortMovesWithOrdering(legalMoves);

        int alpha = numeric_limits<int>::min() + 1;
        int beta = numeric_limits<int>::max();
        int currentBestScore = numeric_limits<int>::min() + 1;

        for (Move &move : legalMoves)
        {
            board.makeMove(move);
            int score = -TTAlphaBeta(depth - 1, -beta, -alpha);
            board.undoMove();

            if (searchCancelled)
                break;

            if (score > currentBestScore)
            {
                currentBestScore = score;
                best_move = move;
            }

            if (score > alpha)
                alpha = score;
        }
        if (searchCancelled)
            break;
        bestScore = currentBestScore;
    }

    return bestScore;
}

int Engine::TTIterativeDeepeningAW(int maxDepth)
{
    int bestScore = 0;
    nodesSearched = 0;

    for (int depth = 1; depth <= maxDepth; depth++)
    {
        vector<Move> legalMoves = board.generateLegalMoves();
        if (legalMoves.empty())
            return 0;

        sortMovesWithOrdering(legalMoves);

        int delta = 50;
        int alpha, beta;

        if (depth == 1)
        {
            alpha = numeric_limits<int>::min() + 1;
            beta = numeric_limits<int>::max();
        }
        else
        {
            alpha = bestScore - delta;
            beta = bestScore + delta;
        }

        int currentBestScore;
        Move currentBestMove;
        while (true)
        {
            int localAlpha = alpha;
            int localBeta = beta;

            currentBestScore = numeric_limits<int>::min() + 1;
            currentBestMove = legalMoves[0];
            for (Move &move : legalMoves)
            {
                board.makeMove(move);
                int score = -TTAlphaBeta(depth - 1, -localBeta, -localAlpha);
                board.undoMove();

                if (searchCancelled)
                    break;

                if (score > currentBestScore)
                {
                    currentBestScore = score;
                    currentBestMove = move;
                }

                if (score > localAlpha)
                    localAlpha = score;

                if (localAlpha >= localBeta) // fail-high cutoff
                    break;
            }

            if (searchCancelled)
                break;

            if (depth > 1 && currentBestScore <= alpha)
            {
                alpha -= delta;
                delta *= 2;
                continue;
            }

            if (depth > 1 && currentBestScore >= beta)
            {
                beta += delta;
                delta *= 2;
                continue;
            }

            break;
        }
        if (searchCancelled)
            break;
        bestScore = currentBestScore;
        best_move = currentBestMove;
    }

    return bestScore;
}
