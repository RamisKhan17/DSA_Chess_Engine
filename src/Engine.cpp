#include "../include/Engine.h"
#include <algorithm>
#include <limits>
#include <iostream>
using namespace std;

Engine::Engine(Board &b, searchType searchFunc, int depth, int startingTime, int timeIncrement) : board(b), searchFunc(searchFunc), searchDepth(depth), nodesSearched(0), remainingTime(startingTime), timeIncrement(timeIncrement)
{
}

void Engine::setPosition(const string &fen)
{
    board.setFEN(fen);
}

inline int Engine::getPieceValue(int piece)
{
    switch (abs(piece))
    {
    case 1:
        return 100;
    case 2:
        return 320;
    case 3:
        return 330;
    case 4:
        return 500;
    case 5:
        return 900;
    case 6:
        return 20000;
    default:
        return 0;
    }
}

void Engine::sortMovesWithOrdering(vector<Move> &moves)
{
    for (Move &m : moves)
    {
        int score = 0;

        if (best_move == m)
            score += 1000000;

        if (m.capturedPiece != 0)
        {
            int victimVal = getPieceValue(m.capturedPiece);
            int attackerVal = getPieceValue(m.piece);
            score += victimVal * 100 - attackerVal + 100000;
        }

        uint64_t moveHash = board.moveHash(board.hash, m);
        TTEntry ent = TT[moveHash & (TT_SIZE - 1)];
        if (ent.key == moveHash)
            score += 50000;

        if (m.flags & FLAG_PROMOTION)
            score += getPieceValue(m.promotionPiece) * 100;

        m.score = score;
    }

sort(moves.begin(), moves.end(), [](const Move &a, const Move &b)
              { return a.score > b.score; });
}

Move Engine::getStandardSearchMove()
{
    nodesSearched = 0;
    vector<Move> legalMoves = board.generateLegalMoves();
    if (legalMoves.empty())
        return Move();

    Move bestMove = legalMoves[0];
    int bestScore = numeric_limits<int>::min() + 1;
    for (Move &move : legalMoves)
    {
        board.makeMove(move);
        int score = -search(searchDepth - 1);
        board.undoMove();
        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }
    }
    cout << "Best move: " << bestMove.toAlgebraic()
         << " (score: " << bestScore << ")\n";
    cout << "Nodes searched: " << nodesSearched << "\n";
    return bestMove;
}

Move Engine::getAlphaBetaMove()
{
    nodesSearched = 0;
    moveTime = -1;
    vector<Move> legalMoves = board.generateLegalMoves();
    if (legalMoves.empty())
        return Move();

    Move bestMove = legalMoves[0];
    int alpha = numeric_limits<int>::min() + 1;
    int beta = numeric_limits<int>::max();

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

    cout << "Best move: " << bestMove.toAlgebraic()
         << " (score: " << alpha << ")\n";
    cout << "Nodes searched: " << nodesSearched << "\n";

    return bestMove;
}

Move Engine::getIterativeDeepeningMove()
{
    nodesSearched = 0;
    moveTime = (remainingTime / 20.0 + timeIncrement / 2.0);
    startTime = chrono::steady_clock::now();
    int best_score;
    switch (searchFunc)
    {
    case ITERATIVE_DEEPENING:
        best_score = iterativeDeepening(searchDepth);
        break;
    case TT_ITERATIVE_DEEPENING:
        best_score = TTIterativeDeepening(searchDepth);
        break;
    case TT_ITERATIVE_DEEPENING_AW:
        best_score = TTIterativeDeepeningAW(searchDepth);
        break;
    default:
        cout << "Wrong value for searchType\n";
        return Move();
    }
cout << "Best move: " << best_move.toAlgebraic()
              << " (score: " << best_score << " )\n";
cout << "Nodes searched: " << nodesSearched << "\n";
    remainingTime = remainingTime - chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count() + timeIncrement;
    searchCancelled = false;
    return best_move;
}

Move Engine::getBestMove()
{
    switch (searchFunc)
    {
    case STANDARD_SEARCH:
        return getStandardSearchMove();
    case ALPHA_BETA:
        return getAlphaBetaMove();
    case ITERATIVE_DEEPENING:
    case TT_ITERATIVE_DEEPENING:
    case TT_ITERATIVE_DEEPENING_AW:
        return getIterativeDeepeningMove();
    default:
        cout << "Wrong value for searchType\n";
        return Move();
    }
}
