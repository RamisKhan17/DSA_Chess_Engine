#include "../include/Board.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

// Helper: piece value lookup used for MVV-LVA ordering
static inline int getPieceValue(int piece)
{
    switch (abs(piece))
    {
    case 1:
        return 100; // Pawn
    case 2:
        return 320; // Knight
    case 3:
        return 330; // Bishop
    case 4:
        return 500; // Rook
    case 5:
        return 900; // Queen
    case 6:
        return 20000; // King (very high)
    default:
        return 0;
    }
}

struct TTEntry
{
    uint64_t key;
    int depth;
    int score;
    int flag;
};

// Static member initialization - precomputed move offsets
const int Board::KNIGHT_OFFSETS[8] = {-33, -31, -18, -14, 14, 18, 31, 33};
const int Board::BISHOP_DIRECTIONS[4] = {-17, -15, 15, 17};
const int Board::ROOK_DIRECTIONS[4] = {-16, -1, 1, 16};
const int Board::KING_OFFSETS[8] = {-17, -16, -15, -1, 1, 15, 16, 17};

Board::Board()
{
    for (int i = 0; i < 128; i++)
    {
        board[i] = EMPTY;
    }

    sideToMove = 0;
    castlingRights = 0;
    enPassantSquare = -1;
    halfMoveClock = 0;
    fullMoveNumber = 1;
    initZobristArrays();
}

void Board::setStartingPosition()
{
    for (int i = 0; i < 128; i++)
    {
        board[i] = EMPTY;
    }
    board[0x00] = WHITE_ROOK;
    board[0x01] = WHITE_KNIGHT;
    board[0x02] = WHITE_BISHOP;
    board[0x03] = WHITE_QUEEN;
    board[0x04] = WHITE_KING;
    board[0x05] = WHITE_BISHOP;
    board[0x06] = WHITE_KNIGHT;
    board[0x07] = WHITE_ROOK;

    for (int file = 0; file < 8; file++)
    {
        board[0x10 + file] = WHITE_PAWN;
    }

    for (int file = 0; file < 8; file++)
    {
        board[0x60 + file] = BLACK_PAWN;
    }
    board[0x70] = BLACK_ROOK;
    board[0x71] = BLACK_KNIGHT;
    board[0x72] = BLACK_BISHOP;
    board[0x73] = BLACK_QUEEN;
    board[0x74] = BLACK_KING;
    board[0x75] = BLACK_BISHOP;
    board[0x76] = BLACK_KNIGHT;
    board[0x77] = BLACK_ROOK;

    sideToMove = 0;
    castlingRights = CASTLING_WHITE_KINGSIDE | CASTLING_WHITE_QUEENSIDE |
                     CASTLING_BLACK_KINGSIDE | CASTLING_BLACK_QUEENSIDE;
    enPassantSquare = -1;
    halfMoveClock = 0;
    fullMoveNumber = 1;
    setHash();
    moveHistory.clear();
}

bool Board::setFEN(const string &fen)
{
istringstream ss(fen);
string pieces, side, castling, enpassant;
    int halfmove, fullmove;

    ss >> pieces >> side >> castling >> enpassant >> halfmove >> fullmove;

    // Clear board
    for (int i = 0; i < 128; i++)
    {
        board[i] = EMPTY;
    }

    // Parse piece placement
    int rank = 7;
    int file = 0;

    for (char c : pieces)
    {
        if (c == '/')
        {
            rank--;
            file = 0;
        }
        else if (c >= '1' && c <= '8')
        {
            file += (c - '0');
        }
        else
        {
            int square = makeSquare(file, rank);
            board[square] = charToPiece(c);
            file++;
        }
    }

    // Parse side to move
    sideToMove = (side == "w" || side == "W") ? 0 : 1;

    // Parse castling rights
    castlingRights = 0;
    for (char c : castling)
    {
        if (c == 'K')
            castlingRights |= CASTLING_WHITE_KINGSIDE;
        else if (c == 'Q')
            castlingRights |= CASTLING_WHITE_QUEENSIDE;
        else if (c == 'k')
            castlingRights |= CASTLING_BLACK_KINGSIDE;
        else if (c == 'q')
            castlingRights |= CASTLING_BLACK_QUEENSIDE;
    }

    // Parse en passant square
    if (enpassant != "-")
    {
        enPassantSquare = algebraicToSquare(enpassant);
    }
    else
    {
        enPassantSquare = -1;
    }

    // Parse move counters
    halfMoveClock = halfmove;
    fullMoveNumber = fullmove;

    moveHistory.clear();
    setHash();
    return true;
}

string Board::getFEN() const
{
    ostringstream fen;

    for (int rank = 7; rank >= 0; rank--)
    {
        int emptyCount = 0;

        for (int file = 0; file < 8; file++)
        {
            int square = makeSquare(file, rank);
            int piece = board[square];

            if (piece == EMPTY)
            {
                emptyCount++;
            }
            else
            {
                if (emptyCount > 0)
                {
                    fen << emptyCount;
                    emptyCount = 0;
                }
                fen << pieceToChar(piece);
            }
        }

        if (emptyCount > 0)
        {
            fen << emptyCount;
        }

        if (rank > 0)
        {
            fen << '/';
        }
    }

    // Side to move
    fen << (sideToMove == 0 ? " w " : " b ");

    // Castling rights
    if (castlingRights == 0)
    {
        fen << "- ";
    }
    else
    {
        if (castlingRights & CASTLING_WHITE_KINGSIDE)
            fen << 'K';
        if (castlingRights & CASTLING_WHITE_QUEENSIDE)
            fen << 'Q';
        if (castlingRights & CASTLING_BLACK_KINGSIDE)
            fen << 'k';
        if (castlingRights & CASTLING_BLACK_QUEENSIDE)
            fen << 'q';
        fen << ' ';
    }

    // En passant square
    if (enPassantSquare >= 0)
    {
        fen << squareToAlgebraic(enPassantSquare) << ' ';
    }
    else
    {
        fen << "- ";
    }

    // Move counters
    fen << halfMoveClock << ' ' << fullMoveNumber;

    return fen.str();
}


uint64_t Board::rand64(uint64_t &state)
{
    uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

int Board::pieceToZobristNumbering(int piece)
{
    if (piece > 0)
        return piece - 1;
    else
        return -piece + 5;
}

void Board::initZobristArrays()
{
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 128; j++)
            zobristPieceSquare[i][j] = rand64(seed);

    for (int i = 0; i < 16; i++)
        zobristCastling[i] = rand64(seed);

    for (int i = 0; i < 8; i++)
        zobristPassant[i] = rand64(seed);

    zobristSide = rand64(seed);
}

void Board::setHash()
{
    hash = 0;
    for (int sq = 0; sq < 128; sq++)
    {
        if (sq & 0x88)
            continue;
        int piece = board[sq];
        if (piece == 0)
            continue;
        piece = pieceToZobristNumbering(piece);
        hash ^= zobristPieceSquare[piece][sq];
    }
    hash ^= zobristCastling[castlingRights];

    if (enPassantSquare != -1)
        hash ^= zobristPassant[enPassantSquare & 7];

    if (sideToMove == 1)
        hash ^= zobristSide;
}

void Board::hashUpdate(Move move)
{
    hash = moveHash(hash, move);
}

uint64_t Board::moveHash(uint64_t currentHash, Move move)
{
    int pieceType = pieceToZobristNumbering(move.piece);
    currentHash ^= zobristSide;
    currentHash ^= zobristPieceSquare[pieceType][move.from];

    if (move.promotionPiece)
        currentHash ^= zobristPieceSquare[pieceToZobristNumbering(move.promotionPiece)][move.to];
    else
        currentHash ^= zobristPieceSquare[pieceType][move.to];

    if (move.capturedPiece)
    {
        int capturedTo = move.to;
        if (move.flags & FLAG_EN_PASSANT)
            capturedTo += (sideToMove == 1 ? -16 : 16); // Flipped cause side to move also flipped
        currentHash ^= zobristPieceSquare[pieceToZobristNumbering(move.capturedPiece)][capturedTo];
    }

    if (!moveHistory.empty())
    {
        int prevEP = moveHistory.back().enPassantSquare;
        if (prevEP != -1)
            currentHash ^= zobristPassant[prevEP & 7];
    }

    if (enPassantSquare != -1)
        currentHash ^= zobristPassant[enPassantSquare & 7];

    if (!moveHistory.empty() && moveHistory.back().castlingRights != castlingRights)
    {
        currentHash ^= zobristCastling[moveHistory.back().castlingRights];
        currentHash ^= zobristCastling[castlingRights];
    }

    if (move.flags & FLAG_CASTLING_KINGSIDE)
    {
        int rookFrom = sideToMove == 1 ? 7 : 119;
        int rookTo = sideToMove == 1 ? 5 : 117;
        int piece = pieceToZobristNumbering(sideToMove == 1 ? WHITE_ROOK : BLACK_ROOK);
        currentHash ^= zobristPieceSquare[piece][rookFrom];
        currentHash ^= zobristPieceSquare[piece][rookTo];
    }

    if (move.flags & FLAG_CASTLING_QUEENSIDE)
    {
        int rookFrom = sideToMove == 1 ? 0 : 112;
        int rookTo = sideToMove == 1 ? 3 : 115;
        int piece = pieceToZobristNumbering(sideToMove == 1 ? WHITE_ROOK : BLACK_ROOK);
        currentHash ^= zobristPieceSquare[piece][rookFrom];
        currentHash ^= zobristPieceSquare[piece][rookTo];
    }
    return currentHash;
}


vector<Move> Board::generateLegalMoves()
{
vector<Move> pseudoLegal = generatePseudoLegalMoves();
vector<Move> legalMoves;

    for (const Move &move : pseudoLegal)
    {
        // Make move temporarily
        MoveInfo info;
        info.move = move;
        info.capturedPiece = board[move.to];
        info.castlingRights = castlingRights;
        info.enPassantSquare = enPassantSquare;
        info.halfMoveClock = halfMoveClock;

        // Execute move
        board[move.to] = board[move.from];
        board[move.from] = EMPTY;

        // Handle special moves
        if (move.flags & FLAG_EN_PASSANT)
        {
            int capturedPawnSquare = move.to + (sideToMove == 0 ? -16 : 16);
            board[capturedPawnSquare] = EMPTY;
        }

        if (move.flags & FLAG_CASTLING_KINGSIDE)
        {
            int rookFrom = sideToMove == 0 ? 0x07 : 0x77;
            int rookTo = sideToMove == 0 ? 0x05 : 0x75;
            board[rookTo] = board[rookFrom];
            board[rookFrom] = EMPTY;
        }

        if (move.flags & FLAG_CASTLING_QUEENSIDE)
        {
            int rookFrom = sideToMove == 0 ? 0x00 : 0x70;
            int rookTo = sideToMove == 0 ? 0x03 : 0x73;
            board[rookTo] = board[rookFrom];
            board[rookFrom] = EMPTY;
        }

        if (move.flags & FLAG_PROMOTION)
        {
            board[move.to] = move.promotionPiece;
        }

        bool legal = !isCheck(sideToMove);
        board[move.from] = board[move.to];
        board[move.to] = info.capturedPiece;

        if (move.flags & FLAG_EN_PASSANT)
        {
            int capturedPawnSquare = move.to + (sideToMove == 0 ? -16 : 16);
            board[capturedPawnSquare] = (sideToMove == 0 ? BLACK_PAWN : WHITE_PAWN);
        }

        if (move.flags & FLAG_CASTLING_KINGSIDE)
        {
            int rookFrom = sideToMove == 0 ? 0x07 : 0x77;
            int rookTo = sideToMove == 0 ? 0x05 : 0x75;
            board[rookFrom] = board[rookTo];
            board[rookTo] = EMPTY;
        }

        if (move.flags & FLAG_CASTLING_QUEENSIDE)
        {
            int rookFrom = sideToMove == 0 ? 0x00 : 0x70;
            int rookTo = sideToMove == 0 ? 0x03 : 0x73;
            board[rookFrom] = board[rookTo];
            board[rookTo] = EMPTY;
        }

        if (move.flags & FLAG_PROMOTION)
        {
            board[move.from] = move.piece;
        }

        if (legal)
        {
            legalMoves.push_back(move);
        }
    }

    return legalMoves;
}

vector<Move> Board::getLegalMovesFrom(int square)
{
vector<Move> allLegalMoves = generateLegalMoves();
vector<Move> movesFromSquare;

    for (const Move &move : allLegalMoves)
    {
        if (move.from == square)
        {
            movesFromSquare.push_back(move);
        }
    }

    return movesFromSquare;
}

vector<Move> Board::generatePseudoLegalMoves() const
{
vector<Move> moves;

    for (int square = 0; square < 128; square++)
    {
        if (square & 0x88)
            continue;

        int piece = board[square];
        if (piece == EMPTY)
            continue;

        if ((sideToMove == 0 && piece < 0) || (sideToMove == 1 && piece > 0))
        {
            continue;
        }

        int pieceType = abs(piece);

        switch (pieceType)
        {
        case 1:
            generatePawnMoves(moves, square);
            break;
        case 2:
            generateKnightMoves(moves, square);
            break;
        case 3:
            generateSlidingMoves(moves, square, BISHOP_DIRECTIONS, 4);
            break;
        case 4:
            generateSlidingMoves(moves, square, ROOK_DIRECTIONS, 4);
            break;
        case 5:
            generateSlidingMoves(moves, square, BISHOP_DIRECTIONS, 4);
            generateSlidingMoves(moves, square, ROOK_DIRECTIONS, 4);
            break;
        case 6:
            generateKingMoves(moves, square);
            break;
        }
    }

    return moves;
}

void Board::generateKnightMoves(vector<Move> &moves, int from) const
{
    int piece = board[from];

    for (int i = 0; i < 8; i++)
    {
        int to = from + KNIGHT_OFFSETS[i];

        // 0x88 boundary check
        if (to & 0x88)
            continue;

        int target = board[to];

        // Can't capture own pieces
        if ((piece > 0 && target > 0) || (piece < 0 && target < 0))
        {
            continue;
        }

        moves.push_back(Move(from, to, piece, target));
    }
}

void Board::generateSlidingMoves(vector<Move> &moves, int from,
                                 const int *directions, int numDirections) const
{
    int piece = board[from];

    for (int i = 0; i < numDirections; i++)
    {
        int dir = directions[i];
        int to = from + dir;

        // Slide in this direction until hitting edge or piece
        while (!(to & 0x88))
        { // While on board
            int target = board[to];

            if (target == EMPTY)
            {
                moves.push_back(Move(from, to, piece, 0));
                to += dir; // Continue in this direction
            }
            else
            {
                // Hit a piece
                if ((piece > 0 && target < 0) || (piece < 0 && target > 0))
                {
                    // Can capture opponent's piece
                    moves.push_back(Move(from, to, piece, target));
                }
                break; // Can't move further in this direction
            }
        }
    }
}

void Board::generatePawnMoves(vector<Move> &moves, int from) const
{
    int piece = board[from];
    int direction = (piece > 0) ? 16 : -16; // White up, black down
    int startRank = (piece > 0) ? 1 : 6;
    int promotionRank = (piece > 0) ? 7 : 0;

    // Single push
    int to = from + direction;
    if (!(to & 0x88) && board[to] == EMPTY)
    {
        if (rankOf(to) == promotionRank)
        {
            // Promotion
            addPromotionMoves(moves, from, to, piece);
        }
        else
        {
            moves.push_back(Move(from, to, piece, 0));

            // Double push from starting position
            if (rankOf(from) == startRank)
            {
                int doubleTo = from + 2 * direction;
                if (!(doubleTo & 0x88) && board[doubleTo] == EMPTY)
                {
                    Move m(from, doubleTo, piece, 0);
                    m.flags = FLAG_DOUBLE_PAWN_PUSH;
                    moves.push_back(m);
                }
            }
        }
    }

    // Captures (including en passant)
    int captureOffsets[2] = {direction - 1, direction + 1};
    for (int i = 0; i < 2; i++)
    {
        int to = from + captureOffsets[i];

        if (to & 0x88)
            continue; // Off board

        int target = board[to];

        // Normal capture
        if ((piece > 0 && target < 0) || (piece < 0 && target > 0))
        {
            if (rankOf(to) == promotionRank)
            {
                addPromotionMoves(moves, from, to, piece, target);
            }
            else
            {
                moves.push_back(Move(from, to, piece, target));
            }
        }

        // En passant
        if (to == enPassantSquare)
        {
            Move m(from, to, piece, (piece > 0) ? BLACK_PAWN : WHITE_PAWN);
            m.flags = FLAG_EN_PASSANT;
            moves.push_back(m);
        }
    }
}

void Board::generateKingMoves(vector<Move> &moves, int from) const
{
    int piece = board[from];

    // Normal king moves
    for (int i = 0; i < 8; i++)
    {
        int to = from + KING_OFFSETS[i];

        if (to & 0x88)
            continue;

        int target = board[to];
        if ((piece > 0 && target > 0) || (piece < 0 && target < 0))
        {
            continue;
        }

        moves.push_back(Move(from, to, piece, target));
    }

    // Castling
    generateCastlingMoves(moves, from);
}

void Board::generateCastlingMoves(vector<Move> &moves, int kingSquare) const
{
    // White kingside castling
    if (sideToMove == 0 && (castlingRights & CASTLING_WHITE_KINGSIDE))
    {
        if (board[0x05] == EMPTY && board[0x06] == EMPTY)
        {
            if (!isSquareAttacked(0x04, 1) && !isSquareAttacked(0x05, 1) &&
                !isSquareAttacked(0x06, 1))
            {
                Move m(0x04, 0x06, WHITE_KING, 0);
                m.flags = FLAG_CASTLING_KINGSIDE;
                moves.push_back(m);
            }
        }
    }

    // White queenside castling
    if (sideToMove == 0 && (castlingRights & CASTLING_WHITE_QUEENSIDE))
    {
        if (board[0x03] == EMPTY && board[0x02] == EMPTY && board[0x01] == EMPTY)
        {
            if (!isSquareAttacked(0x04, 1) && !isSquareAttacked(0x03, 1) &&
                !isSquareAttacked(0x02, 1))
            {
                Move m(0x04, 0x02, WHITE_KING, 0);
                m.flags = FLAG_CASTLING_QUEENSIDE;
                moves.push_back(m);
            }
        }
    }

    // Black kingside castling
    if (sideToMove == 1 && (castlingRights & CASTLING_BLACK_KINGSIDE))
    {
        if (board[0x75] == EMPTY && board[0x76] == EMPTY)
        {
            if (!isSquareAttacked(0x74, 0) && !isSquareAttacked(0x75, 0) &&
                !isSquareAttacked(0x76, 0))
            {
                Move m(0x74, 0x76, BLACK_KING, 0);
                m.flags = FLAG_CASTLING_KINGSIDE;
                moves.push_back(m);
            }
        }
    }

    // Black queenside castling
    if (sideToMove == 1 && (castlingRights & CASTLING_BLACK_QUEENSIDE))
    {
        if (board[0x73] == EMPTY && board[0x72] == EMPTY && board[0x71] == EMPTY)
        {
            if (!isSquareAttacked(0x74, 0) && !isSquareAttacked(0x73, 0) &&
                !isSquareAttacked(0x72, 0))
            {
                Move m(0x74, 0x72, BLACK_KING, 0);
                m.flags = FLAG_CASTLING_QUEENSIDE;
                moves.push_back(m);
            }
        }
    }
}

void Board::addPromotionMoves(vector<Move> &moves, int from, int to,
                              int piece, int capturedPiece) const
{
    // Add all four promotion options
    int promotionPieces[4];

    if (piece > 0)
    { // White
        promotionPieces[0] = WHITE_QUEEN;
        promotionPieces[1] = WHITE_ROOK;
        promotionPieces[2] = WHITE_BISHOP;
        promotionPieces[3] = WHITE_KNIGHT;
    }
    else
    { // Black
        promotionPieces[0] = BLACK_QUEEN;
        promotionPieces[1] = BLACK_ROOK;
        promotionPieces[2] = BLACK_BISHOP;
        promotionPieces[3] = BLACK_KNIGHT;
    }

    for (int i = 0; i < 4; i++)
    {
        Move m(from, to, piece, capturedPiece);
        m.promotionPiece = promotionPieces[i];
        m.flags = FLAG_PROMOTION;
        moves.push_back(m);
    }
}


bool Board::isCheck(int side) const
{
    int kingSquare = findKing(side);
    if (kingSquare < 0)
        return false; // King not found (shouldn't happen)

    return isSquareAttacked(kingSquare, 1 - side);
}

bool Board::enemyPawnCanCaptureEP(int epSquare)
{
    int enemyPawn = (sideToMove == 0) ? BLACK_PAWN : WHITE_PAWN;

    // Enemy pawn must be on left or right file
    int left = epSquare - 1;
    int right = epSquare + 1;

    return (board[left] == enemyPawn || board[right] == enemyPawn);
}

bool Board::isSquareAttacked(int square, int attackingSide) const
{
    // Check knight attacks
    for (int i = 0; i < 8; i++)
    {
        int from = square + KNIGHT_OFFSETS[i];
        if (from & 0x88)
            continue;

        int piece = board[from];
        int knight = (attackingSide == 0) ? WHITE_KNIGHT : BLACK_KNIGHT;
        if (piece == knight)
            return true;
    }

    // Check bishop/queen attacks (diagonal)
    for (int i = 0; i < 4; i++)
    {
        int dir = BISHOP_DIRECTIONS[i];
        int from = square + dir;

        while (!(from & 0x88))
        {
            int piece = board[from];
            if (piece != EMPTY)
            {
                int bishop = (attackingSide == 0) ? WHITE_BISHOP : BLACK_BISHOP;
                int queen = (attackingSide == 0) ? WHITE_QUEEN : BLACK_QUEEN;
                if (piece == bishop || piece == queen)
                    return true;
                break;
            }
            from += dir;
        }
    }

    // Check rook/queen attacks (straight)
    for (int i = 0; i < 4; i++)
    {
        int dir = ROOK_DIRECTIONS[i];
        int from = square + dir;

        while (!(from & 0x88))
        {
            int piece = board[from];
            if (piece != EMPTY)
            {
                int rook = (attackingSide == 0) ? WHITE_ROOK : BLACK_ROOK;
                int queen = (attackingSide == 0) ? WHITE_QUEEN : BLACK_QUEEN;
                if (piece == rook || piece == queen)
                    return true;
                break;
            }
            from += dir;
        }
    }

    // Check pawn attacks
    // White pawns attack diagonally forward (toward higher ranks)
    // Black pawns attack diagonally forward (toward lower ranks)
    // To find attacking pawns: look for pawns that are behind the target square
    int pawnAttacks[2];
    if (attackingSide == 0)
    {                         // White pawns attacking
        pawnAttacks[0] = -17; // diagonal down-left
        pawnAttacks[1] = -15; // diagonal down-right
    }
    else
    {                        // Black pawns attacking
        pawnAttacks[0] = 15; // diagonal up-left
        pawnAttacks[1] = 17; // diagonal up-right
    }
    int pawn = (attackingSide == 0) ? WHITE_PAWN : BLACK_PAWN;

    for (int i = 0; i < 2; i++)
    {
        int from = square + pawnAttacks[i];
        if (!(from & 0x88) && board[from] == pawn)
        {
            // Additional check: ensure the pawn is in a position to attack forward
            int fromRank = from >> 4;
            int targetRank = square >> 4;

            if (attackingSide == 0)
            { // White pawn
                // White pawn must be on a lower rank than the target square
                if (fromRank < targetRank)
                {
                    return true;
                }
            }
            else
            { // Black pawn
                // Black pawn must be on a higher rank than the target square
                if (fromRank > targetRank)
                {
                    return true;
                }
            }
        }
    }

    // Check king attacks
    int king = (attackingSide == 0) ? WHITE_KING : BLACK_KING;
    for (int i = 0; i < 8; i++)
    {
        int from = square + KING_OFFSETS[i];
        if (!(from & 0x88) && board[from] == king)
        {
            return true;
        }
    }

    return false;
}

int Board::findKing(int side) const
{
    int king = (side == 0) ? WHITE_KING : BLACK_KING;

    for (int sq = 0; sq < 128; sq++)
    {
        if (!(sq & 0x88) && board[sq] == king)
        {
            return sq;
        }
    }

    return -1; // King not found
}


bool Board::makeMove(const Move &move)
{
    // Save state for undo
    MoveInfo info;
    info.move = move;
    info.capturedPiece = board[move.to];
    info.castlingRights = castlingRights;
    info.enPassantSquare = enPassantSquare;
    info.halfMoveClock = halfMoveClock;

    // Move the piece
    board[move.to] = board[move.from];
    board[move.from] = EMPTY;

    // Handle special moves
    if (move.flags & FLAG_CASTLING_KINGSIDE)
    {
        int rookFrom = sideToMove == 0 ? 0x07 : 0x77;
        int rookTo = sideToMove == 0 ? 0x05 : 0x75;
        board[rookTo] = board[rookFrom];
        board[rookFrom] = EMPTY;
    }

    if (move.flags & FLAG_CASTLING_QUEENSIDE)
    {
        int rookFrom = sideToMove == 0 ? 0x00 : 0x70;
        int rookTo = sideToMove == 0 ? 0x03 : 0x73;
        board[rookTo] = board[rookFrom];
        board[rookFrom] = EMPTY;
    }

    if (move.flags & FLAG_EN_PASSANT)
    {
        int capturedPawnSquare = move.to + (sideToMove == 0 ? -16 : 16);
        board[capturedPawnSquare] = EMPTY;
    }

    if (move.flags & FLAG_PROMOTION)
    {
        board[move.to] = move.promotionPiece;
    }

    // Update en passant square
    if (move.flags & FLAG_DOUBLE_PAWN_PUSH)
    {
        int ep = (move.from + move.to) / 2;

        if (enemyPawnCanCaptureEP(move.to))
            enPassantSquare = ep;
        else
            enPassantSquare = -1;
    }
    else
        enPassantSquare = -1;

    // Update castling rights
    updateCastlingRights(move);

    // Update move counters
    if (abs(board[move.from]) == 1 || move.capturedPiece != 0)
    {
        halfMoveClock = 0;
    }
    else
    {
        halfMoveClock++;
    }

    if (sideToMove == 1)
    {
        fullMoveNumber++;
    }

    sideToMove = 1 - sideToMove;
    moveHistory.push_back(info);
    hashUpdate(move);
    return true;
}

void Board::undoMove()
{
    if (moveHistory.empty())
        return;

    MoveInfo info = moveHistory.back();
    hashUpdate(info.move);
    moveHistory.pop_back();

    // Restore side to move
    sideToMove = 1 - sideToMove;
    if (sideToMove == 1)
    {
        fullMoveNumber--;
    }

    // Restore the piece
    board[info.move.from] = board[info.move.to];
    board[info.move.to] = info.capturedPiece;

    // Handle special moves (reverse)
    if (info.move.flags & FLAG_CASTLING_KINGSIDE)
    {
        int rookFrom = sideToMove == 0 ? 0x07 : 0x77;
        int rookTo = sideToMove == 0 ? 0x05 : 0x75;
        board[rookFrom] = board[rookTo];
        board[rookTo] = EMPTY;
    }

    if (info.move.flags & FLAG_CASTLING_QUEENSIDE)
    {
        int rookFrom = sideToMove == 0 ? 0x00 : 0x70;
        int rookTo = sideToMove == 0 ? 0x03 : 0x73;
        board[rookFrom] = board[rookTo];
        board[rookTo] = EMPTY;
    }

    if (info.move.flags & FLAG_EN_PASSANT)
    {
        int capturedPawnSquare = info.move.to + (sideToMove == 0 ? -16 : 16);
        board[capturedPawnSquare] = (sideToMove == 0 ? BLACK_PAWN : WHITE_PAWN);
    }

    if (info.move.flags & FLAG_PROMOTION)
    {
        board[info.move.from] = info.move.piece;
    }

    // Restore game state
    castlingRights = info.castlingRights;
    enPassantSquare = info.enPassantSquare;
    halfMoveClock = info.halfMoveClock;
}

void Board::updateCastlingRights(const Move &move)
{
    // If king moves, lose all castling rights for that side
    if (abs(move.piece) == 6)
    {
        if (sideToMove == 0)
        {
            castlingRights &= ~(CASTLING_WHITE_KINGSIDE | CASTLING_WHITE_QUEENSIDE);
        }
        else
        {
            castlingRights &= ~(CASTLING_BLACK_KINGSIDE | CASTLING_BLACK_QUEENSIDE);
        }
    }

    // If rook moves or is captured, lose castling right on that side
    if (move.from == 0x00 || move.to == 0x00)
    {
        castlingRights &= ~CASTLING_WHITE_QUEENSIDE;
    }
    if (move.from == 0x07 || move.to == 0x07)
    {
        castlingRights &= ~CASTLING_WHITE_KINGSIDE;
    }
    if (move.from == 0x70 || move.to == 0x70)
    {
        castlingRights &= ~CASTLING_BLACK_QUEENSIDE;
    }
    if (move.from == 0x77 || move.to == 0x77)
    {
        castlingRights &= ~CASTLING_BLACK_KINGSIDE;
    }
}

// ==================== GAME STATE QUERIES ====================

bool Board::isCheckmate()
{
    if (!isCheck(sideToMove))
        return false;
    return generateLegalMoves().empty();
}

bool Board::isStalemate()
{
    if (isCheck(sideToMove))
        return false;
    return generateLegalMoves().empty();
}

bool Board::isDraw() const
{
    // 50-move rule
    if (halfMoveClock >= 100)
        return true;

    // Insufficient material
    if (isInsufficientMaterial())
        return true;

    // Threefold repetition
    if (isThreefoldRepetition())
        return true;

    return false;
}

// Helper: detect situations where checkmate is impossible with current material
bool Board::isInsufficientMaterial() const
{
    int pawnCount = 0;
    int majorCount = 0; // rooks or queens
    int minorCount = 0; // knights or bishops
    int bishopSquares[2] = {-1, -1};
    int bishopFound = 0;

    for (int sq = 0; sq < 128; ++sq)
    {
        if (sq & 0x88)
            continue;

        int p = board[sq];
        if (p == EMPTY)
            continue;

        int absP = abs(p);
        if (absP == 1)
            ++pawnCount;
        else if (absP == 2)
            ++minorCount; // knight
        else if (absP == 3)
        {
            ++minorCount; // bishop
            if (bishopFound < 2)
                bishopSquares[bishopFound++] = sq;
        }
        else if (absP == 4 || absP == 5)
            ++majorCount; // rook or queen
    }

    // If there are any pawns or any major pieces, material is sufficient
    if (pawnCount > 0 || majorCount > 0)
        return false;

    // Only kings
    if (minorCount == 0)
        return true;

    // Single minor piece vs lone king (K+B vs K or K+N vs K)
    if (minorCount == 1)
        return true;

    // Two bishops only (one per side or both on same side): draw if bishops are on same color
    if (minorCount == 2 && bishopFound == 2)
    {
        auto squareColor = [](int sq)
        {
            int f = sq & 7;
            int r = sq >> 4;
            return (f + r) & 1; // 0 = dark, 1 = light
        };

        if (squareColor(bishopSquares[0]) == squareColor(bishopSquares[1]))
            return true;
    }

    // Otherwise, consider material sufficient
    return false;
}

// Helper: detect threefold repetition by undoing moves on a copy
bool Board::isThreefoldRepetition() const
{
    Board temp = *this; // make a copy we can undo on
    uint64_t key = temp.hash;
    int occurrences = 1; // current position

    // Walk backwards through history by undoing moves on the temp board
    while (!temp.moveHistory.empty())
    {
        temp.undoMove();
        if (temp.hash == key)
        {
            ++occurrences;
            if (occurrences >= 3)
                return true;
        }
    }

    return false;
}


int Board::getPiece(int square) const
{
    if (square & 0x88)
        return EMPTY;
    return board[square];
}

void Board::setPiece(int square, int piece)
{
    if (!(square & 0x88))
    {
        board[square] = piece;
    }
}

bool Board::isValidSquare(int square) const
{
    return (square & 0x88) == 0;
}

void Board::print() const
{
cout << "\n  +---+---+---+---+---+---+---+---+\n";

    for (int rank = 7; rank >= 0; rank--)
    {
cout << (rank + 1) << " |";

        for (int file = 0; file < 8; file++)
        {
            int square = makeSquare(file, rank);
            int piece = board[square];
            char c = pieceToChar(piece);
cout << " " << c << " |";
        }

cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }

cout << "    a   b   c   d   e   f   g   h\n\n";
cout << "FEN: " << getFEN() << "\n";
cout << "Side to move: " << (sideToMove == 0 ? "White" : "Black") << "\n";
}

// ==================== MOVE STRING FUNCTIONS ====================

string Move::toAlgebraic() const
{
    return squareToAlgebraic(from) + squareToAlgebraic(to);
}

string Move::toUCI() const
{
string uci = squareToAlgebraic(from) + squareToAlgebraic(to);

    if (flags & FLAG_PROMOTION)
    {
        char promoChar = tolower(pieceToChar(promotionPiece));
        uci += promoChar;
    }

    return uci;
}


string squareToAlgebraic(int square88)
{
    if (square88 & 0x88)
        return "-";

    int file = fileOf(square88);
    int rank = rankOf(square88);

string result;
    result += (char)('a' + file);
    result += (char)('1' + rank);

    return result;
}

int algebraicToSquare(const string &algebraic)
{
    if (algebraic.length() < 2)
        return -1;

    int file = algebraic[0] - 'a';
    int rank = algebraic[1] - '1';

    if (file < 0 || file > 7 || rank < 0 || rank > 7)
        return -1;

    return makeSquare(file, rank);
}

char pieceToChar(int piece)
{
    switch (piece)
    {
    case WHITE_PAWN:
        return 'P';
    case WHITE_KNIGHT:
        return 'N';
    case WHITE_BISHOP:
        return 'B';
    case WHITE_ROOK:
        return 'R';
    case WHITE_QUEEN:
        return 'Q';
    case WHITE_KING:
        return 'K';
    case BLACK_PAWN:
        return 'p';
    case BLACK_KNIGHT:
        return 'n';
    case BLACK_BISHOP:
        return 'b';
    case BLACK_ROOK:
        return 'r';
    case BLACK_QUEEN:
        return 'q';
    case BLACK_KING:
        return 'k';
    default:
        return ' ';
    }
}

int charToPiece(char c)
{
    switch (c)
    {
    case 'P':
        return WHITE_PAWN;
    case 'N':
        return WHITE_KNIGHT;
    case 'B':
        return WHITE_BISHOP;
    case 'R':
        return WHITE_ROOK;
    case 'Q':
        return WHITE_QUEEN;
    case 'K':
        return WHITE_KING;
    case 'p':
        return BLACK_PAWN;
    case 'n':
        return BLACK_KNIGHT;
    case 'b':
        return BLACK_BISHOP;
    case 'r':
        return BLACK_ROOK;
    case 'q':
        return BLACK_QUEEN;
    case 'k':
        return BLACK_KING;
    default:
        return EMPTY;
    }
}
