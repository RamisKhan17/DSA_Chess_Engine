#include "../include/Board.h"
#include <iostream>
#include <iomanip>
#include <chrono>

/**
 * Perft (Performance Test) - Counts nodes at each depth
 * 
 * This is a critical test for move generation correctness.
 * It counts all possible positions at each depth and compares
 * against known correct values.
 * 
 * Time Complexity: O(b^d) where b = branching factor, d = depth
 * Space Complexity: O(d) for recursion stack
 */

uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1;
    
    uint64_t nodes = 0;
    std::vector<Move> moves = board.generateLegalMoves();
    
    for (const Move& move : moves) {
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.undoMove();
    }
    
    return nodes;
}

/**
 * Perft Divide - Shows node count for each move at root
 * Useful for debugging move generation issues
 */
void perftDivide(Board& board, int depth) {
    std::cout << "\nPerft Divide (Depth " << depth << "):\n";
    std::cout << "====================================\n";
    
    std::vector<Move> moves = board.generateLegalMoves();
    uint64_t totalNodes = 0;
    
    for (const Move& move : moves) {
        board.makeMove(move);
        uint64_t nodes = perft(board, depth - 1);
        board.undoMove();
        
        std::cout << move.toAlgebraic() << ": " << nodes << "\n";
        totalNodes += nodes;
    }
    
    std::cout << "\nTotal nodes: " << totalNodes << "\n";
}

/**
 * Run perft test with timing
 */
void runPerftTest(Board& board, int depth, uint64_t expectedNodes) {
    std::cout << "\nPerft Depth " << depth << ":\n";
    std::cout << "Expected: " << expectedNodes << " nodes\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t nodes = perft(board, depth);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Result:   " << nodes << " nodes\n";
    std::cout << "Time:     " << duration.count() << " ms\n";
    
    if (nodes == expectedNodes) {
        std::cout << "Status:   ✓ PASSED\n";
    } else {
        std::cout << "Status:   ✗ FAILED (difference: " 
                  << (int64_t)(nodes - expectedNodes) << ")\n";
    }
    
    if (duration.count() > 0) {
        uint64_t nps = (nodes * 1000) / duration.count();
        std::cout << "Speed:    " << nps << " nodes/sec\n";
    }
}

/**
 * Test suite with multiple positions
 */
struct PerftPosition {
    std::string name;
    std::string fen;
    std::vector<uint64_t> expectedNodes;  // Results for depths 1, 2, 3, 4, 5...
};

int main() {
    std::cout << "===============================================\n";
    std::cout << "  PERFT Testing - Move Generation Validation\n";
    std::cout << "===============================================\n\n";
    
    std::cout << "Perft (Performance Test) counts all possible positions\n";
    std::cout << "at each depth. This validates move generation correctness.\n\n";
    
    // Test positions
    std::vector<PerftPosition> positions = {
        {
            "Starting Position",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {20, 400, 8902, 197281, 4865609}
        },
        {
            "Position 2 (Kiwipete)",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            {48, 2039, 97862, 4085603}
        },
        {
            "Position 3",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            {14, 191, 2812, 43238, 674624}
        },
        {
            "Position 4",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            {6, 264, 9467, 422333}
        },
        {
            "Position 5",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            {44, 1486, 62379, 2103487}
        },
        {
            "Position 6",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
            {46, 2079, 89890, 3894594}
        }
    };
    
    // Run tests for each position
    for (const auto& pos : positions) {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "Testing: " << pos.name << "\n";
        std::cout << "FEN: " << pos.fen << "\n";
        std::cout << std::string(60, '=') << "\n";
        
        Board board;
        board.setFEN(pos.fen);
        board.print();
        
        // Run perft tests up to depth 3 (depth 4+ takes too long for initial testing)
        int maxDepth = std::min(3, (int)pos.expectedNodes.size());
        
        for (int depth = 1; depth <= maxDepth; depth++) {
            runPerftTest(board, depth, pos.expectedNodes[depth - 1]);
        }
        
        // Optionally show divide for depth 1
        if (pos.name == "Starting Position") {
            perftDivide(board, 1);
        }
    }
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "Perft Testing Complete!\n";
    std::cout << std::string(60, '=') << "\n\n";
    
    std::cout << "Note: To run deeper perft tests (depth 4-6), compile with\n";
    std::cout << "optimizations enabled (-O3 or Release mode) for better performance.\n\n";
    
    std::cout << "Expected performance:\n";
    std::cout << "  - Depth 1-3: Instant (< 10ms)\n";
    std::cout << "  - Depth 4:   < 1 second\n";
    std::cout << "  - Depth 5:   < 30 seconds (optimized build)\n";
    std::cout << "  - Depth 6:   Several minutes\n\n";
    
    return 0;
}

