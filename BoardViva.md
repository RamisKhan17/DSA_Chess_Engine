## Board.cpp Viva Prep

Use this as a quick, confident walkthrough of the `Board` class. Assume the professor will probe both “what” and “why”.

### High-Level Idea
- Implements an 0x88 chess board (1D array of 128 squares) to simplify move bounds checks.
- Encapsulates full game state: pieces on squares, side to move, castling rights, en passant target, half-move clock, full-move number, and move history for undo.
- Provides FEN load/save, move generation (pseudo-legal + legality filtering), move execution/undo, and basic draw/checkmate/stalemate detection.

### Core Data Structures
- `board[128]`: 0x88 indexed squares; off-board squares have bit 0x88 set.
- Piece encoding: positive = White, negative = Black; magnitude denotes type (1 pawn … 6 king).
- Castling flags: `CASTLING_WHITE_KINGSIDE`, etc., bitwise combined.
- `sideToMove` (0 white / 1 black), `enPassantSquare` (square index or -1), `halfMoveClock`, `fullMoveNumber`.
- `Move`: stores from/to, moving piece, captured piece, promotion piece, and flags (promotion, en passant, castling, double pawn push).
- `MoveInfo`: snapshot of state for undo (move, captured piece, castling rights, en passant, half-move).

### Board Lifecycle
- `Board()`: zeroes board and state.
- `setStartingPosition()`: lays out standard start position, initializes castling rights, clears history.
- `setFEN(...)` / `getFEN()`: load/save full position (pieces, side, castling, en passant, clocks).

### Move Generation
1) `generatePseudoLegalMoves()`: enumerates moves by piece type without king-safety filtering.
   - Pawn: single/double pushes, captures, en passant, promotions.
   - Knight: fixed offsets.
   - Bishop/Rook/Queen: sliding along direction vectors until blocked.
   - King: 1-step moves + calls `generateCastlingMoves`.
2) `generateLegalMoves()`: filters pseudo-legal moves by:
   - Make move on board, apply special rules (en passant capture, rook moves in castling, promotion).
   - Check `isCheck(sideToMove)` to ensure own king not attacked after the move.
   - Undo to restore state.
3) `getLegalMovesFrom(square)`: helper that filters legal moves originating at a square.

### Attack / Check Detection
- `isCheck(side)`: finds king then calls `isSquareAttacked`.
- `isSquareAttacked(square, attackingSide)`: tests:
  - Knight offsets.
  - Sliding bishop/queen (diagonals) and rook/queen (files/ranks).
  - Pawn attack directions by side.
  - Adjacent king squares.

### Castling Logic
- `generateCastlingMoves`: requires empty path and no attacked squares the king crosses/lands on.
- `updateCastlingRights`: clears side-specific rights when king moves or when rooks move/are captured on their home squares.

### Move Execution / Undo
- `makeMove(...)`:
  - Snapshot `MoveInfo` for undo.
  - Move piece; handle castling rook moves, en passant capture removal, promotion replacement.
  - Set en passant target after double push.
  - Update castling rights and 50-move clock (resets on pawn move or capture).
  - Increment fullmove on Black’s move; flip `sideToMove`.
- `undoMove()`:
  - Revert side/fullmove.
  - Restore pieces and special-move consequences (rooks, en passant pawn, promotion).
  - Restore castling rights, en passant square, half-move clock from history.

### Game State Queries
- `isCheckmate()`: side is in check and has no legal moves.
- `isStalemate()`: side is not in check and has no legal moves.
- `isDraw()`: currently only 50-move rule implemented; TODOs for insufficient material and threefold repetition.

### Helper Concepts
- 0x88 utilities: `makeSquare(file, rank)`, `fileOf`, `rankOf` (likely defined in header), `isValidSquare`.
- Algebraic helpers: `squareToAlgebraic`, `algebraicToSquare`.
- Piece ↔ char mapping for FEN/printing.
- `print()`: ASCII board + FEN + side to move.

### Key Implementation Choices (good to mention)
- 0x88 board simplifies boundary checks: any square with bit 0x88 set is off-board.
- Separation of pseudo-legal vs. legal generation keeps logic clean and reusable.
- State snapshot (`MoveInfo`) enables reliable undo for search.
- Bit flags compactly encode move specials and castling rights.

### Potential Professor Questions & Short Answers
- Q: Why 0x88?  
  A: O(1) boundary detection; off-board squares share bit 0x88, so `(sq & 0x88)` guards loops.
- Q: Difference between pseudo-legal and legal moves?  
  A: Pseudo ignores king safety; legal filters out moves leaving own king in check.
- Q: How is en passant handled?  
  A: Special flag; capture square is behind the target; enPassantSquare set after a double push.
- Q: When are castling rights lost?  
  A: If king moves, or if rook moves/is captured on its original square.
- Q: What draw rules are implemented?  
  A: 50-move rule only; TODOs for insufficient material and threefold repetition.
- Q: How do you ensure undo correctness?  
  A: Store prior state (castling, en passant, half-move, captured piece) in `MoveInfo` and reverse special moves.

### If Asked to Improve (show initiative)
- Add insufficient material detection (e.g., K vs K; K+N vs K; K+B vs K; K+B vs K+B with same-color bishops).
- Add threefold repetition via hashing (Zobrist) or move/state history.
- Add perft/self-check tests for move generator.
- Add move ordering and evaluation to build a search engine later.

### Quick Walkthrough Order (for the viva)
1) Data representation: 0x88, piece encoding, state fields.
2) Setup: `setStartingPosition`, FEN parsing.
3) Move generation: pseudo-legal by piece, then legality filtering.
4) Special rules: castling, en passant, promotion.
5) Check/checkmate/stalemate detection.
6) Move make/undo flow and why snapshots are needed.
7) Current limitations and planned improvements (draw rules TODOs).

### Additional Tips
- Speak to *why* choices were made (simplicity, speed, correctness).
- Mention 0x88 tradeoff: easy bounds checking; array sparsity is acceptable for clarity.
- Emphasize separation of concerns (generation vs. validation vs. execution).
- Keep examples ready: describe a castling legality check or an en passant capture.
- If stuck, outline the relevant function and its role instead of guessing details.

