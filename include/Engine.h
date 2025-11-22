#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include <vector>
#include <string>

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

class EvalBoard
{
public:
    Board board;
    int eval;
};

class Engine
{
private:
    Board &board;
    int searchDepth;
    int nodesSearched;
    Move best_move;
    bool best_validity = false;

public:
    /**
     * Constructor
     * @param depth - Default search depth
     */
    Engine(Board &board, int depth = 4);
    /**
     * Sets the board position
     * @param fen - FEN string of position
     */
    void setPosition(const std::string &fen);

    /**
     * Gets the best move for current position
     * This is where your search algorithm goes!
     *
     * @return Best move found
     *
     * TODO (Search Team): Implement alpha-beta search
     * TODO (Evaluation Team): Implement position evaluation
     * TODO (Optimization Team): Add transposition tables and move ordering
     */
    Move getAlphaBetaMove();
    Move getIterativeDeepeningMove();
    Move getBestMove();
    /**
     * Evaluates current position
     * Positive = good for white, Negative = good for black
     *
     * @return Evaluation score in centipawns
     *
     * TODO (Evaluation Team): Implement this!
     */
    int evaluate();

    /**
     * Search function with alpha-beta pruning
     *
     * @param depth - Remaining search depth
     * @param alpha - Alpha value for pruning
     * @param beta - Beta value for pruning
     * @return Best score found
     *
     * TODO (Search Team): Implement this!
     */

    int search(int depth);
    int alphaBeta(int depth, int alpha, int beta);
    int iterativeDeepening(int max_depth);
    int alphaBetaPlus(int depth, int max_depth, int alpha, int beta);
    // EvalBoard alphaBetaBoard(int depth, EvalBoard alpha_board, EvalBoard beta_board);

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
