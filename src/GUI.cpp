#include "../include/GUI.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// ==================== CONSTRUCTOR ====================

ChessGUI::ChessGUI(Board& b) : board(b) {
    // Create window
    window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), 
                  "Chess Engine - DSA Project",
                  sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    
    // Initialize state
    selectedSquare = -1;
    isDragging = false;
    draggedPiece = EMPTY;
    draggedSquare = -1;
    lastMoveFrom = -1;
    lastMoveTo = -1;
    gameOver = false;
    
    // Initialize colors and resources
    initializeColors();
    loadResources();
}

// ==================== INITIALIZATION ====================

void ChessGUI::initializeColors() {
    // Board colors (classic wooden style)
    lightSquareColor = sf::Color(240, 217, 181);
    darkSquareColor = sf::Color(181, 136, 99);
    
    // Highlight colors
    highlightColor = sf::Color(255, 255, 0, 100);      // Yellow, semi-transparent
    legalMoveColor = sf::Color(100, 200, 100, 120);    // Green, semi-transparent
    lastMoveColor = sf::Color(255, 255, 100, 80);      // Light yellow
}

void ChessGUI::loadResources() {
    // Load piece textures
    std::map<int, std::string> pieceFiles = {
        {WHITE_PAWN, "assets/w-pawn.png"},
        {WHITE_KNIGHT, "assets/w-knight.png"},
        {WHITE_BISHOP, "assets/w-bishop.png"},
        {WHITE_ROOK, "assets/w-rook.png"},
        {WHITE_QUEEN, "assets/w-queen.png"},
        {WHITE_KING, "assets/w-king.png"},
        {BLACK_PAWN, "assets/b-pawn.png"},
        {BLACK_KNIGHT, "assets/b-knight.png"},
        {BLACK_BISHOP, "assets/b-bishop.png"},
        {BLACK_ROOK, "assets/b-rook.png"},
        {BLACK_QUEEN, "assets/b-queen.png"},
        {BLACK_KING, "assets/b-king.png"}
    };
    
    for (const auto& pair : pieceFiles) {
        sf::Texture texture;
        if (texture.loadFromFile(pair.second)) {
            pieceTextures[pair.first] = texture;
            std::cout << "Loaded texture: " << pair.second << std::endl;
        } else {
            std::cout << "Failed to load texture: " << pair.second << std::endl;
        }
    }
    
    // Try to load font (use default if not available)
    if (!font.openFromFile("arial.ttf")) {
        // Font not found - that's okay, we'll render without text
        std::cout << "Note: Font file not found. Text rendering disabled.\n";
    }
}

// ==================== MAIN LOOP ====================

void ChessGUI::run() {
    while (window.isOpen()) {
        handleEvents();
        render();
    }
}

// ==================== EVENT HANDLING ====================

void ChessGUI::handleEvents() {
    while (auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        
        if (auto mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                handleMousePress(mousePressed->position.x, mousePressed->position.y);
            }
        }
        
        if (auto mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseReleased->button == sf::Mouse::Button::Left) {
                handleMouseRelease(mouseReleased->position.x, mouseReleased->position.y);
            }
        }
        
        if (auto mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
            handleMouseMove(mouseMoved->position.x, mouseMoved->position.y);
        }
        
        // Keyboard shortcuts
        if (auto keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::U) {
                // Undo move
                if (!gameOver) {
                    board.undoMove();
                    if (!moveHistory.empty()) {
                        moveHistory.pop_back();
                    }
                    clearSelection();
                    checkGameOver();
                }
            }
            
            if (keyPressed->code == sf::Keyboard::Key::R) {
                // Reset board
                board.setStartingPosition();
                moveHistory.clear();
                clearSelection();
                gameOver = false;
                gameResult = "";
            }
        }
    }
}

void ChessGUI::handleMousePress(int x, int y) {
    if (gameOver) return;
    
    int square = screenToSquare(x, y);
    if (square < 0) return;
    
    int piece = board.getPiece(square);
    
    // Check if clicking on own piece
    if (piece != EMPTY) {
        int side = board.getSideToMove();
        if ((side == 0 && piece > 0) || (side == 1 && piece < 0)) {
            // Start dragging
            isDragging = true;
            draggedPiece = piece;
            draggedSquare = square;
            dragPosition = sf::Vector2f(x, y);
            
            // Select square and get legal moves
            selectedSquare = square;
            selectedMoves = board.getLegalMovesFrom(square);
            return;
        }
    }
    
    // If already have a piece selected, try to move
    if (selectedSquare >= 0) {
        if (tryMakeMove(selectedSquare, square)) {
            clearSelection();
        } else {
            clearSelection();
        }
    }
}

void ChessGUI::handleMouseRelease(int x, int y) {
    if (!isDragging) return;
    
    int targetSquare = screenToSquare(x, y);
    
    if (targetSquare >= 0 && draggedSquare >= 0) {
        if (tryMakeMove(draggedSquare, targetSquare)) {
            clearSelection();
        }
    }
    
    isDragging = false;
    draggedPiece = EMPTY;
    draggedSquare = -1;
    clearSelection();
}

void ChessGUI::handleMouseMove(int x, int y) {
    if (isDragging) {
        dragPosition = sf::Vector2f(x, y);
    }
}

bool ChessGUI::tryMakeMove(int fromSquare, int toSquare) {
    // Get all legal moves from the source square
    std::vector<Move> legalMoves = board.getLegalMovesFrom(fromSquare);
    
    // Find the move that matches from->to
    Move* foundMove = nullptr;
    std::vector<Move> promotionMoves;
    
    for (Move& move : legalMoves) {
        if (move.from == fromSquare && move.to == toSquare) {
            if (move.flags & FLAG_PROMOTION) {
                promotionMoves.push_back(move);
            } else {
                foundMove = &move;
                break;
            }
        }
    }
    
    // Handle promotion
    if (!promotionMoves.empty()) {
        int promotionPiece = selectPromotionPiece();
        for (Move& move : promotionMoves) {
            if (move.promotionPiece == promotionPiece) {
                foundMove = &move;
                break;
            }
        }
    }
    
    if (foundMove) {
        // Store move for history
        std::string moveStr = formatMove(*foundMove, board.getFullMoveNumber());
        
        // Make the move
        lastMoveFrom = fromSquare;
        lastMoveTo = toSquare;
        
        board.makeMove(*foundMove);
        moveHistory.push_back(moveStr);
        
        updateGameState();
        return true;
    }
    
    return false;
}

int ChessGUI::selectPromotionPiece() {
    // Simple promotion selection - default to queen
    // In a full implementation, would show a dialog
    
    int side = board.getSideToMove();
    return (side == 0) ? WHITE_QUEEN : BLACK_QUEEN;
    
    // TODO: Implement graphical promotion dialog
}

// ==================== RENDERING ====================

void ChessGUI::render() {
    window.clear(sf::Color(50, 50, 50));
    
    drawBoard();
    drawHighlights();
    drawPieces();
    drawDraggedPiece();
    drawInfoPanel();
    
    window.display();
}

void ChessGUI::drawBoard() {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
            square.setPosition(sf::Vector2f(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE));
            
            // Checkerboard pattern
            if ((rank + file) % 2 == 0) {
                square.setFillColor(lightSquareColor);
            } else {
                square.setFillColor(darkSquareColor);
            }
            
            window.draw(square);
        }
    }
    
    // Draw coordinate labels if font is loaded
    if (font.getInfo().family != "") {
        // File labels (a-h)
        for (int file = 0; file < 8; file++) {
            sf::Text text(font, std::string(1, 'a' + file), 16);
            text.setFillColor(file % 2 == 1 ? lightSquareColor : darkSquareColor);
            text.setPosition(sf::Vector2f(file * SQUARE_SIZE + 5, BOARD_SIZE - 20));
            window.draw(text);
        }
        
        // Rank labels (1-8)
        for (int rank = 0; rank < 8; rank++) {
            sf::Text text(font, std::to_string(rank + 1), 16);
            text.setFillColor(rank % 2 == 0 ? lightSquareColor : darkSquareColor);
            text.setPosition(sf::Vector2f(BOARD_SIZE - 15, (7 - rank) * SQUARE_SIZE + 5));
            window.draw(text);
        }
    }
}

void ChessGUI::drawPieces() {
    for (int square88 = 0; square88 < 128; square88++) {
        if (square88 & 0x88) continue;  // Skip off-board squares
        
        // Don't draw the dragged piece in its original position
        if (isDragging && square88 == draggedSquare) continue;
        
        int piece = board.getPiece(square88);
        if (piece == EMPTY) continue;
        
        sf::Vector2f pos = squareToScreen(square88);
        drawPiece(piece, pos.x, pos.y);
    }
}

void ChessGUI::drawPiece(int piece, float x, float y, float scale) {
    if (piece == EMPTY) return;
    
    // Check if we have a texture for this piece
    if (pieceTextures.find(piece) != pieceTextures.end()) {
        // Draw piece using texture
        sf::Sprite sprite(pieceTextures[piece]);
        
        // Scale the sprite to fit the square
        float pieceSize = SQUARE_SIZE * 0.9f * scale;  // 90% of square size
        sf::Vector2u textureSize = pieceTextures[piece].getSize();
        float scaleFactor = pieceSize / std::max(textureSize.x, textureSize.y);
        sprite.setScale(sf::Vector2f(scaleFactor, scaleFactor));
        
        // Center the piece in the square
        sf::Vector2f spriteSize = sf::Vector2f(
            textureSize.x * scaleFactor,
            textureSize.y * scaleFactor
        );
        sprite.setPosition(sf::Vector2f(
            x + (SQUARE_SIZE - spriteSize.x) / 2,
            y + (SQUARE_SIZE - spriteSize.y) / 2
        ));
        
        window.draw(sprite);
    } else {
        // Fallback: draw piece as a circle with letter if texture not found
        float radius = SQUARE_SIZE * 0.35f * scale;
        sf::CircleShape circle(radius);
        
        circle.setPosition(sf::Vector2f(x + SQUARE_SIZE / 2 - radius, y + SQUARE_SIZE / 2 - radius));
        circle.setFillColor(piece > 0 ? sf::Color(255, 255, 255) : sf::Color(50, 50, 50));
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color(0, 0, 0, 150));
        
        window.draw(circle);
        
        // Draw piece letter if font is available
        if (font.getInfo().family != "") {
            sf::Text text(font, std::string(1, pieceToChar(piece)), static_cast<unsigned int>(40 * scale));
            text.setFillColor(piece > 0 ? sf::Color(100, 100, 100) : sf::Color(220, 220, 220));
            text.setStyle(sf::Text::Bold);
            
            // Center the text (simplified for SFML 3.0)
            text.setPosition(sf::Vector2f(x + SQUARE_SIZE / 2 - 10,
                            y + SQUARE_SIZE / 2 - 15));
            
            window.draw(text);
        }
    }
}

void ChessGUI::drawDraggedPiece() {
    if (isDragging && draggedPiece != EMPTY) {
        float x = dragPosition.x - SQUARE_SIZE / 2;
        float y = dragPosition.y - SQUARE_SIZE / 2;
        drawPiece(draggedPiece, x, y, 1.2f);
    }
}

void ChessGUI::drawHighlights() {
    // Highlight last move
    if (lastMoveFrom >= 0) {
        highlightSquare(lastMoveFrom, lastMoveColor);
    }
    if (lastMoveTo >= 0) {
        highlightSquare(lastMoveTo, lastMoveColor);
    }
    
    // Highlight selected square
    if (selectedSquare >= 0 && !isDragging) {
        highlightSquare(selectedSquare, highlightColor);
    }
    
    // Draw legal move indicators
    drawLegalMoves();
}

void ChessGUI::drawLegalMoves() {
    for (const Move& move : selectedMoves) {
        sf::Vector2f center = squareCenter(move.to);
        
        // Draw circle for legal moves
        float radius = SQUARE_SIZE * 0.15f;
        if (board.getPiece(move.to) != EMPTY) {
            radius = SQUARE_SIZE * 0.4f;  // Larger circle for captures
        }
        
        sf::CircleShape indicator(radius);
        indicator.setPosition(sf::Vector2f(center.x - radius, center.y - radius));
        indicator.setFillColor(legalMoveColor);
        
        window.draw(indicator);
    }
}

void ChessGUI::highlightSquare(int square88, sf::Color color) {
    if (square88 < 0 || (square88 & 0x88)) return;
    
    sf::Vector2f pos = squareToScreen(square88);
    sf::RectangleShape highlight(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    highlight.setPosition(pos);
    highlight.setFillColor(color);
    
    window.draw(highlight);
}

void ChessGUI::drawInfoPanel() {
    // Background for info panel
    sf::RectangleShape panel(sf::Vector2f(INFO_PANEL_WIDTH, WINDOW_HEIGHT));
    panel.setPosition(sf::Vector2f(BOARD_SIZE, 0));
    panel.setFillColor(sf::Color(40, 40, 40));
    window.draw(panel);
    
    if (font.getInfo().family == "") return;  // No font loaded
    
    float yOffset = 20;
    
    // Title
    sf::Text title(font, "Chess Engine", 24);
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(BOARD_SIZE + 20, yOffset));
    window.draw(title);
    yOffset += 40;
    
    // Current turn
    sf::Text turnText(font, std::string("Turn: ") + 
                      (board.getSideToMove() == 0 ? "White" : "Black"), 18);
    turnText.setFillColor(sf::Color::White);
    turnText.setPosition(sf::Vector2f(BOARD_SIZE + 20, yOffset));
    window.draw(turnText);
    yOffset += 30;
    
    // Game status
    drawGameStatus();
    
    // Move history
    yOffset = 150;
    sf::Text historyTitle(font, "Move History:", 18);
    historyTitle.setFillColor(sf::Color::White);
    historyTitle.setStyle(sf::Text::Bold);
    historyTitle.setPosition(sf::Vector2f(BOARD_SIZE + 20, yOffset));
    window.draw(historyTitle);
    yOffset += 30;
    
    drawMoveHistory();
    
    // Controls
    yOffset = WINDOW_HEIGHT - 100;
    sf::Text controls(font, "Controls:\nU - Undo\nR - Reset", 14);
    controls.setFillColor(sf::Color(150, 150, 150));
    controls.setPosition(sf::Vector2f(BOARD_SIZE + 20, yOffset));
    window.draw(controls);
}

void ChessGUI::drawMoveHistory() {
    if (font.getInfo().family == "") return;
    
    float yOffset = 180;
    int displayCount = std::min(15, (int)moveHistory.size());
    int startIndex = std::max(0, (int)moveHistory.size() - displayCount);
    
    for (int i = startIndex; i < moveHistory.size(); i++) {
        sf::Text moveText(font, moveHistory[i], 14);
        moveText.setFillColor(sf::Color(200, 200, 200));
        moveText.setPosition(sf::Vector2f(BOARD_SIZE + 25, yOffset));
        window.draw(moveText);
        yOffset += 20;
    }
}

void ChessGUI::drawGameStatus() {
    if (font.getInfo().family == "") return;
    
    float yOffset = 100;
    sf::Text statusText(font, "", 16);
    statusText.setPosition(sf::Vector2f(BOARD_SIZE + 20, yOffset));
    
    if (gameOver) {
        statusText.setString(gameResult);
        statusText.setFillColor(sf::Color::Red);
        statusText.setStyle(sf::Text::Bold);
    } else if (board.isCheck(board.getSideToMove())) {
        statusText.setString("CHECK!");
        statusText.setFillColor(sf::Color::Red);
        statusText.setStyle(sf::Text::Bold);
    } else {
        statusText.setString("In progress");
        statusText.setFillColor(sf::Color::Green);
    }
    
    window.draw(statusText);
}

void ChessGUI::drawCapturedPieces() {
    // TODO: Implement captured pieces display
}

// ==================== COORDINATE CONVERSION ====================

int ChessGUI::screenToSquare(int x, int y) const {
    // Check if click is on the board
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return -1;
    }
    
    int file = x / SQUARE_SIZE;
    int rank = 7 - (y / SQUARE_SIZE);
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) {
        return -1;
    }
    
    return makeSquare(file, rank);
}

sf::Vector2f ChessGUI::squareToScreen(int square88) const {
    if (square88 & 0x88) return sf::Vector2f(-1, -1);
    
    int file = fileOf(square88);
    int rank = rankOf(square88);
    
    float x = file * SQUARE_SIZE;
    float y = (7 - rank) * SQUARE_SIZE;
    
    return sf::Vector2f(x, y);
}

sf::Vector2f ChessGUI::squareCenter(int square88) const {
    sf::Vector2f topLeft = squareToScreen(square88);
    return sf::Vector2f(topLeft.x + SQUARE_SIZE / 2, topLeft.y + SQUARE_SIZE / 2);
}

// ==================== UTILITY ====================

void ChessGUI::updateGameState() {
    checkGameOver();
}

void ChessGUI::checkGameOver() {
    if (board.isCheckmate()) {
        gameOver = true;
        gameResult = (board.getSideToMove() == 0) ? "Black Wins!" : "White Wins!";
        gameResult += "\nCheckmate!";
    } else if (board.isStalemate()) {
        gameOver = true;
        gameResult = "Draw!\nStalemate";
    } else if (board.isDraw()) {
        gameOver = true;
        gameResult = "Draw!";
    }
}

void ChessGUI::clearSelection() {
    selectedSquare = -1;
    selectedMoves.clear();
}

std::string ChessGUI::formatMove(const Move& move, int moveNumber) const {
    std::ostringstream oss;
    
    // Move number for white moves
    if (board.getSideToMove() == 0) {
        oss << moveNumber << ". ";
    } else if (moveHistory.empty() || moveHistory.back().find('.') != std::string::npos) {
        oss << moveNumber << "... ";
    }
    
    // Get piece character
    char pieceChar = pieceToChar(move.piece);
    if (std::abs(move.piece) != 1) {  // Not a pawn
        oss << (char)std::toupper(pieceChar);
    }
    
    // Origin square for disambiguation (simplified - full implementation would be more complex)
    // oss << squareToAlgebraic(move.from);
    
    // Capture notation
    if (move.capturedPiece != 0) {
        if (std::abs(move.piece) == 1) {  // Pawn capture
            oss << (char)('a' + fileOf(move.from));
        }
        oss << "x";
    }
    
    // Destination square
    oss << squareToAlgebraic(move.to);
    
    // Promotion
    if (move.flags & FLAG_PROMOTION) {
        oss << "=" << (char)std::toupper(pieceToChar(move.promotionPiece));
    }
    
    // Castling
    if (move.flags & FLAG_CASTLING_KINGSIDE) {
        return (board.getSideToMove() == 0 ? std::to_string(moveNumber) + ". " : "") + "O-O";
    }
    if (move.flags & FLAG_CASTLING_QUEENSIDE) {
        return (board.getSideToMove() == 0 ? std::to_string(moveNumber) + ". " : "") + "O-O-O";
    }
    
    return oss.str();
}

