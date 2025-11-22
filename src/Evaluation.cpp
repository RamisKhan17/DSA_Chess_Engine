
#include "../include/Engine.h"
#include "../include/Board.h"
#include <cmath>
#include <algorithm>

// All scores are from White's point of view (positive = good for White)

namespace
{
    // ------------------ Piece type codes (from Board.cpp) ------------------
    constexpr int PAWN = 1;
    constexpr int KNIGHT = 2;
    constexpr int BISHOP = 3;
    constexpr int ROOK = 4;
    constexpr int QUEEN = 5;
    constexpr int KING = 6;

    // ------------------ Base Piece Values (centipawns) ------------------
    constexpr int PAWN_VALUE = 100;
    constexpr int KNIGHT_VALUE = 320;
    constexpr int BISHOP_VALUE = 330;
    constexpr int ROOK_VALUE = 500;
    constexpr int QUEEN_VALUE = 900;
    constexpr int MATE_SCORE = 100000;

    // ------------------ Pawn Piece-Square Table ------------------
    // Index: [rank][file]  (rank/file = 0..7)
    const int pawnPST[8][8] = {
        {0, 0, 0, 0, 0, 0, 0, 0},       // back rank (0)
        {0, 0, 0, 0, 0, 0, 0, 0},       // 2nd rank (start) → no bonus
        {5, 5, 10, 15, 15, 10, 5, 5},   // 3rd rank → central pawns preferred
        {5, 10, 15, 20, 20, 15, 10, 5}, // 4th rank
        {0, 5, 10, 15, 15, 10, 5, 0},   // 5th rank
        {0, 0, 0, 5, 5, 0, 0, 0},       // 6th rank
        {0, 0, 0, 0, 0, 0, 0, 0},       // 7th rank (reserved for endgame bonus)
        {0, 0, 0, 0, 0, 0, 0, 0}        // promotion rank (not on board yet)
    };

    // ------------------ Game Phase ------------------
    enum class GamePhase
    {
        OPENING,
        MIDDLEGAME,
        ENDGAME
    };

    GamePhase detectGamePhase(const Board &board)
    {
        bool whiteQueen = false, blackQueen = false;
        int minorAndRooks = 0;

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int piece = board.getPiece(sq);
            if (piece == EMPTY)
                continue;

            switch (std::abs(piece))
            {
            case QUEEN:
                if (piece > 0)
                    whiteQueen = true;
                else
                    blackQueen = true;
                break;

            case BISHOP:
            case KNIGHT:
            case ROOK:
                ++minorAndRooks;
                break;

            default:
                break;
            }
        }

        if (!whiteQueen && !blackQueen)
            return GamePhase::ENDGAME;

        if (minorAndRooks >= 6)
            return GamePhase::OPENING;

        return GamePhase::MIDDLEGAME;
    }

    // ------------------ 1) Material ------------------
    int evaluateMaterial(const Board &board)
    {
        int score = 0;

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int piece = board.getPiece(sq);
            if (piece == EMPTY)
                continue;

            int val = 0;
            switch (std::abs(piece))
            {
            case PAWN:
                val = PAWN_VALUE;
                break;
            case KNIGHT:
                val = KNIGHT_VALUE;
                break;
            case BISHOP:
                val = BISHOP_VALUE;
                break;
            case ROOK:
                val = ROOK_VALUE;
                break;
            case QUEEN:
                val = QUEEN_VALUE;
                break;
            case KING:
                val = 0;
                break; // king value handled via safety
            }

            score += (piece > 0 ? val : -val);
        }

        return score;
    }

    // ------------------ 2) Piece-Square Tables ------------------
    int evaluatePieceSquare(const Board &board)
    {
        int score = 0;

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int piece = board.getPiece(sq);
            if (piece == EMPTY)
                continue;

            int r = rankOf(sq);
            int f = fileOf(sq);

            switch (std::abs(piece))
            {
            case PAWN:
                if (piece > 0)
                    score += pawnPST[r][f];
                else
                    score -= pawnPST[7 - r][f]; // mirror for black
                break;

            default:
                break;
            }
        }

        return score;
    }

    // ------------------ 3) Pawn Structure ------------------
    int evaluatePawnStructure(const Board &board)
    {
        int score = 0;
        int whitePawnFiles[8] = {0};
        int blackPawnFiles[8] = {0};

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int piece = board.getPiece(sq);
            if (std::abs(piece) != PAWN)
                continue;

            int f = fileOf(sq);
            if (piece > 0)
                whitePawnFiles[f]++;
            else
                blackPawnFiles[f]++;
        }

        // Doubled pawns (very basic)
        for (int f = 0; f < 8; ++f)
        {
            if (whitePawnFiles[f] > 1)
                score -= 10 * (whitePawnFiles[f] - 1);
            if (blackPawnFiles[f] > 1)
                score += 10 * (blackPawnFiles[f] - 1);
        }

        return score;
    }

    // ------------------ 4) Evaluation of Pieces ------------------
    int evaluatePieces(const Board &board)
    {
        int score = 0;

        // Example: bishop pair bonus
        int whiteBishops = 0, blackBishops = 0;

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int p = board.getPiece(sq);
            if (p == WHITE_BISHOP)
                ++whiteBishops;
            if (p == BLACK_BISHOP)
                ++blackBishops;
        }

        if (whiteBishops >= 2)
            score += 25;
        if (blackBishops >= 2)
            score -= 25;

        return score;
    }

    // ------------------ 5) Evaluation Patterns ------------------
    int evaluatePatterns(const Board &board)
    {
        int score = 0;

        // Example: rook on 7th rank
        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int p = board.getPiece(sq);
            if (p == WHITE_ROOK && rankOf(sq) == 6)
                score += 20;
            if (p == BLACK_ROOK && rankOf(sq) == 1)
                score -= 20;
        }

        return score;
    }

    // ------------------ 6) Mobility ------------------
    int evaluateMobility(Board &board)
    {
        auto moves = board.generateLegalMoves();
        int count = static_cast<int>(moves.size());
        int score = count * 2; // weight 2 per legal move (tune later)

        // sideToMove: 0 = White, 1 = Black (see Board.cpp / GUI.cpp)
        int sideToMove = board.getSideToMove();
        return (sideToMove == 0) ? score : -score;
    }

    // ------------------ 7) Center Control ------------------
    int evaluateCenterControl(const Board &board)
    {
        int score = 0;

        int centers[4] = {
            makeSquare(3, 3), // d4
            makeSquare(4, 3), // e4
            makeSquare(3, 4), // d5
            makeSquare(4, 4)  // e5
        };

        for (int sq : centers)
        {
            int p = board.getPiece(sq);
            if (p == EMPTY)
                continue;

            int bonus = 10;
            if (p > 0)
                score += bonus;
            else
                score -= bonus;
        }

        return score;
    }

    // ------------------ 8) Connectivity ------------------
    int evaluateConnectivity(const Board &board)
    {
        int score = 0;
        static const int kingOffsets[8] = {-17, -16, -15, -1, 1, 15, 16, 17};

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int piece = board.getPiece(sq);
            if (piece == EMPTY)
                continue;

            int friends = 0;
            for (int off : kingOffsets)
            {
                int to = sq + off;
                if (to & 0x88)
                    continue;
                int p2 = board.getPiece(to);
                if (p2 == EMPTY)
                    continue;
                if ((p2 > 0 && piece > 0) || (p2 < 0 && piece < 0))
                    ++friends;
            }

            int val = 3 * friends; // weight 3 per friendly neighbour
            if (piece > 0)
                score += val;
            else
                score -= val;
        }

        return score;
    }

    // ------------------ 9) Trapped Pieces ------------------
    int evaluateTrappedPieces(Board &board)
    {
        int score = 0;

        auto moves = board.generateLegalMoves();
        int mobility[128] = {0};
        for (const Move &m : moves)
        {
            if (m.from & 0x88)
                continue;
            mobility[m.from]++;
        }

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int piece = board.getPiece(sq);
            if (piece == EMPTY)
                continue;

            if ((std::abs(piece) == KNIGHT || std::abs(piece) == BISHOP) &&
                mobility[sq] <= 1)
            {
                int pen = 20;
                if (piece > 0)
                    score -= pen;
                else
                    score += pen;
            }
        }

        return score;
    }

    // ------------------ 10) King Safety ------------------
    int evaluateKingSafety(const Board &board, GamePhase phase)
    {
        int score = 0;
        int whiteKingSq = -1, blackKingSq = -1;

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int p = board.getPiece(sq);
            if (p == WHITE_KING)
                whiteKingSq = sq;
            if (p == BLACK_KING)
                blackKingSq = sq;
        }

        auto kingSafety = [&](int sq, bool isWhite)
        {
            if (sq < 0)
                return 0;
            int r = rankOf(sq);
            int f = fileOf(sq);
            int s = 0;

            bool inCenterFile = (f >= 2 && f <= 5);
            bool inBackRank = (isWhite ? r == 0 : r == 7);

            if (!inBackRank && phase != GamePhase::ENDGAME)
                s -= 30; // uncastled / off back rank

            if (inCenterFile && phase != GamePhase::ENDGAME)
                s -= 20; // central king dangerous early

            if (phase == GamePhase::ENDGAME && inCenterFile)
                s += 20; // central king good in endgame

            return s;
        };

        score += kingSafety(whiteKingSq, true);
        score -= kingSafety(blackKingSq, false);

        return score;
    }

    // ------------------ 11) Space ------------------
    int evaluateSpace(const Board &board)
    {
        int score = 0;

        for (int sq = 0; sq < 128; ++sq)
        {
            if (sq & 0x88)
                continue;
            int p = board.getPiece(sq);
            if (p == EMPTY)
                continue;

            int r = rankOf(sq);

            // White piece in Black half
            if (p > 0 && r >= 4)
                score += 5;
            // Black piece in White half
            if (p < 0 && r <= 3)
                score -= 5;
        }

        return score;
    }

    // ------------------ 12) Tempo ------------------
    int evaluateTempo(const Board &board)
    {
        // small bonus to side-to-move
        return (board.getSideToMove() == 0) ? 10 : -10;
    }

} // anonymous namespace

// ==========================================================
// Public evaluation function used by Engine search
// ==========================================================

int Engine::evaluate()
{
    GamePhase phase = detectGamePhase(board);

    int score = 0;
    score += evaluateMaterial(board);
    score += evaluatePieceSquare(board);
    score += evaluatePawnStructure(board);
    score += evaluatePieces(board);
    score += evaluatePatterns(board);
    score += evaluateMobility(board);
    score += evaluateCenterControl(board);
    score += evaluateConnectivity(board);
    score += evaluateTrappedPieces(board);
    score += evaluateKingSafety(board, phase);
    score += evaluateSpace(board);
    score += evaluateTempo(board);

    // Convert from White POV to side-to-move POV
    if (board.getSideToMove() == 1) // 1 = Black
        score = -score;

    return score;
}
