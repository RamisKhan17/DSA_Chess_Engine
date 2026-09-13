#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include <vector>
#include <string>
#include <chrono>
using namespace std;

/**
 * Chess Engine - Interface for Search and Evaluation
 *
 * This is a placeholder interface for teammates implementing:
 * - Search algorithms (Alpha-Beta, Iterative Deepening)
 * - Position evaluation
 * - Move ordering
 * - Transposition tables
 *
 * The Move Generation module (Board class) is complete and ready for integration.
 */

static const int TT_SIZE = 4194304;

enum bound
{
    FLAG_EXACT,
    FLAG_LOWERBOUND,
    FLAG_UPPERBOUND
};

enum searchType
{
    STANDARD_SEARCH,
    ALPHA_BETA,
    ITERATIVE_DEEPENING,
    TT_ITERATIVE_DEEPENING,
    TT_ITERATIVE_DEEPENING_AW
};

struct TTEntry
{
    uint64_t key;
    int depth = -1;
    int score;
    bound flag;
};

inline int ttIndex(uint64_t hash)
{
    return hash & (TT_SIZE - 1);
}

class Engine
{
private:
    Board &board;
    searchType searchFunc;
    int searchDepth;
    int nodesSearched;
    long long remainingTime;
    int timeIncrement;
    long long moveTime;
    chrono::steady_clock::time_point startTime;
    bool searchCancelled = false;
    Move best_move;
    bool best_validity = false;
    int count = 0;
    TTEntry *TT = new TTEntry[TT_SIZE];

public:
    /**
     * Constructor
     * @param depth - Default search depth
     */
    Engine(Board &board, searchType searchFunc, int depth = 4, int startingTime = 200000, int timeIncrement = 0);
    /**
     * Sets the board position
     * @param fen - FEN string of position
     */
    void setPosition(const std::string &fen);

    Move getStandardSearchMove();
    Move getAlphaBetaMove();
    Move getIterativeDeepeningMove();
    Move getBestMove();

    int evaluate();
    inline int getPieceValue(int piece);
    void sortMovesWithOrdering(std::vector<Move> &moves);

    int search(int depth);
    int alphaBeta(int depth, int alpha, int beta);
    int TTAlphaBeta(int depth, int alpha, int beta);
    int iterativeDeepening(int max_depth);
    int TTIterativeDeepening(int max_depth);
    int TTIterativeDeepeningAW(int max_depth);

    /**
     * Gets number of nodes searched
     * @return Node count
     */
    int getNodesSearched() const
    {
        return nodesSearched;
    }

    /**
     * Gets current board
     * @return Reference to board
     */
    Board &getBoard() { return board; }
};

#endif // ENGINE_H
