#include "../include/Engine.h"
#include <algorithm>
#include <limits>
#include <iostream>

/**
 * Engine Implementation
 * 
 * This is a basic placeholder implementation.
 * Teammates should extend this with proper search and evaluation.
 */

Engine::Engine(int depth) : searchDepth(depth), nodesSearched(0) {
    board.setStartingPosition();
}

void Engine::setPosition(const std::string& fen) {
    board.setFEN(fen);
}

Move Engine::getBestMove() {
    std::cout << "Searching for best move at depth " << searchDepth << "...\n";
    
    nodesSearched = 0;
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    if (legalMoves.empty()) {
        // No legal moves - game over
        return Move();
    }
    
    // Very basic implementation - just picks first legal move
    // TODO (Search Team): Replace with alpha-beta search
    
    Move bestMove = legalMoves[0];
    int bestScore = std::numeric_limits<int>::min();
    
    for (Move& move : legalMoves) {
        board.makeMove(move);
        
        // Simple recursive search (not optimized)
        int score = -alphaBeta(searchDepth - 1, 
                               std::numeric_limits<int>::min(), 
                               std::numeric_limits<int>::max());
        
        board.undoMove();
        
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    
    std::cout << "Best move: " << bestMove.toAlgebraic() 
              << " (score: " << bestScore << ")\n";
    std::cout << "Nodes searched: " << nodesSearched << "\n";
    
    return bestMove;
}

int Engine::evaluate() {
    // Very basic material evaluation
    // TODO (Evaluation Team): Implement proper evaluation with:
    // - Piece-square tables
    // - Pawn structure
    // - King safety
    // - Mobility
    // - etc.
    
    int score = 0;
    
    // Piece values in centipawns
    const int PAWN_VALUE = 100;
    const int KNIGHT_VALUE = 320;
    const int BISHOP_VALUE = 330;
    const int ROOK_VALUE = 500;
    const int QUEEN_VALUE = 900;
    
    for (int sq = 0; sq < 128; sq++) {
        if (sq & 0x88) continue;
        
        int piece = board.getPiece(sq);
        if (piece == EMPTY) continue;
        
        int value = 0;
        switch (std::abs(piece)) {
            case 1: value = PAWN_VALUE; break;
            case 2: value = KNIGHT_VALUE; break;
            case 3: value = BISHOP_VALUE; break;
            case 4: value = ROOK_VALUE; break;
            case 5: value = QUEEN_VALUE; break;
            case 6: value = 0; break;  // King has no material value
        }
        
        if (piece > 0) {
            score += value;
        } else {
            score -= value;
        }
    }
    
    // Return score from current side's perspective
    if (board.getSideToMove() == 1) {
        score = -score;
    }
    
    return score;
}

int Engine::alphaBeta(int depth, int alpha, int beta) {
    nodesSearched++;
    
    // Check for terminal conditions
    if (board.isCheckmate()) {
        return -100000 + (searchDepth - depth);  // Favor quicker mates
    }
    
    if (board.isStalemate() || board.isDraw()) {
        return 0;
    }
    
    // Leaf node - evaluate position
    if (depth <= 0) {
        return evaluate();
    }
    
    // TODO (Search Team): Implement proper alpha-beta with:
    // - Move ordering
    // - Transposition table lookups
    // - Null move pruning
    // - Late move reductions
    // - Quiescence search
    
    std::vector<Move> moves = board.generateLegalMoves();
    
    // TODO (Optimization Team): Order moves here!
    
    for (Move& move : moves) {
        board.makeMove(move);
        int score = -alphaBeta(depth - 1, -beta, -alpha);
        board.undoMove();
        
        if (score >= beta) {
            return beta;  // Beta cutoff
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}


