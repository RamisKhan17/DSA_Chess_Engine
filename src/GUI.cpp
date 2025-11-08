#include "../include/GUI.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// ==================== CONSTRUCTOR ====================

ChessGUI::ChessGUI(Board &b, Engine &e, int enginePlay) : board(b), engine(e), engineTurn(enginePlay)
{
    // Create window with resize support
    window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
                  "Chess Engine - DSA Project",
                  sf::Style::Default);
    window.setFramerateLimit(60);

    // Initialize state
    selectedSquare = -1;
    isDragging = false;
    draggedPiece = EMPTY;
    draggedSquare = -1;
    lastMoveFrom = -1;
    lastMoveTo = -1;
    gameOver = false;

    // Initialize scaling variables
    currentWindowWidth = WINDOW_WIDTH;
    currentWindowHeight = WINDOW_HEIGHT;
    scaleX = 1.0f;
    scaleY = 1.0f;
    currentSquareSize = SQUARE_SIZE;
    currentBoardSize = BOARD_SIZE;
    currentInfoPanelWidth = INFO_PANEL_WIDTH;

    // Initialize promotion dialog state
    showingPromotionDialog = false;
    promotionSquare = -1;

    // Initialize colors and resources
    initializeColors();
    loadResources();
}

// ==================== INITIALIZATION ====================

void ChessGUI::initializeColors()
{
    // Board colors (classic wooden style)
    lightSquareColor = sf::Color(240, 217, 181);
    darkSquareColor = sf::Color(181, 136, 99);

    // Highlight colors
    highlightColor = sf::Color(255, 255, 0, 100);   // Yellow, semi-transparent
    legalMoveColor = sf::Color(100, 200, 100, 120); // Green, semi-transparent
    lastMoveColor = sf::Color(255, 255, 100, 80);   // Light yellow
}

void ChessGUI::loadResources()
{
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
        {BLACK_KING, "assets/b-king.png"}};

    for (const auto &pair : pieceFiles)
    {
        sf::Texture texture;
        if (texture.loadFromFile(pair.second))
        {
            pieceTextures[pair.first] = texture;
            std::cout << "Loaded texture: " << pair.second << std::endl;
        }
        else
        {
            std::cout << "Failed to load texture: " << pair.second << std::endl;
        }
    }

    // Try to load font (use default if not available)
    if (!font.openFromFile("assets/fonts/ARIAL.ttf"))
    {
        // Font not found - that's okay, we'll render without text
        std::cout << "Note: Font file not found. Text rendering disabled.\n";
    }
}

// ==================== MAIN LOOP ====================

void ChessGUI::run()
{
    while (window.isOpen())
    {
        handleEvents();
        render();
        if (engineTurn > 0)
        {
            makeEngineMove();
            engineTurn = -engineTurn;
        }
    }
}
void ChessGUI::makeEngineMove()
{
    if (gameOver)
        return;
    Move engineMove = engine.getBestMove();
    lastMoveFrom = engineMove.from;
    lastMoveTo = engineMove.to;
    std::string moveStr = formatMove(engineMove, board.getFullMoveNumber());
    board.makeMove(engineMove);
    moveHistory.push_back(moveStr);
    updateGameState();
}

// ==================== EVENT HANDLING ====================

void ChessGUI::handleEvents()
{
    while (auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window.close();
        }

        if (auto resized = event->getIf<sf::Event::Resized>())
        {
            // Handle window resize - maintain aspect ratio and minimum size
            handleWindowResize(resized->size.x, resized->size.y);
        }

        if (auto mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Left)
            {
                if (showingPromotionDialog)
                {
                    if (handlePromotionClick(mousePressed->position.x, mousePressed->position.y))
                    {
                        // Promotion piece selected, dialog will be closed
                        showingPromotionDialog = false;
                    }
                }
                else
                {
                    handleMousePress(mousePressed->position.x, mousePressed->position.y);
                }
            }
        }

        if (auto mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseReleased->button == sf::Mouse::Button::Left)
            {
                handleMouseRelease(mouseReleased->position.x, mouseReleased->position.y);
            }
        }

        if (auto mouseMoved = event->getIf<sf::Event::MouseMoved>())
        {
            handleMouseMove(mouseMoved->position.x, mouseMoved->position.y);
        }

        // Keyboard shortcuts
        if (auto keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::U)
            {
                // Undo move
                if (!gameOver)
                {
                    board.undoMove();
                    if (!moveHistory.empty())
                    {
                        moveHistory.pop_back();
                    }
                    clearSelection();
                    checkGameOver();
                }
            }

            if (keyPressed->code == sf::Keyboard::Key::R)
            {
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

void ChessGUI::handleMousePress(int x, int y)
{
    if (gameOver)
        return;

    int square = screenToSquare(x, y);
    if (square < 0)
        return;

    int piece = board.getPiece(square);

    // Check if clicking on own piece
    if (piece != EMPTY)
    {
        int side = board.getSideToMove();
        if ((side == 0 && piece > 0) || (side == 1 && piece < 0))
        {
            // If clicking on the same piece that's already selected, deselect it
            if (selectedSquare == square)
            {
                clearSelection();
                return;
            }

            // Start dragging (for visual feedback)
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

    // If clicking on empty square or opponent piece, clear selection
    if (selectedSquare >= 0)
    {
        // If already have a piece selected, try to move
        if (tryMakeMove(selectedSquare, square))
        {
            clearSelection();
        }
        else
        {
            clearSelection();
        }
    }
}

void ChessGUI::handleMouseRelease(int x, int y)
{
    if (!isDragging)
        return;

    int targetSquare = screenToSquare(x, y);

    if (targetSquare >= 0 && draggedSquare >= 0)
    {
        if (tryMakeMove(draggedSquare, targetSquare))
        {
            clearSelection();
        }
    }

    isDragging = false;
    draggedPiece = EMPTY;
    draggedSquare = -1;
    // Don't clear selection here - let it stay visible until next click
}

void ChessGUI::handleMouseMove(int x, int y)
{
    if (isDragging)
    {
        dragPosition = sf::Vector2f(x, y);
    }
}

void ChessGUI::handleWindowResize(int width, int height)
{
    // Set minimum window size to prevent the window from becoming too small
    const int MIN_WIDTH = 640;  // BOARD_SIZE (640) + INFO_PANEL_WIDTH (300) = 940, but use 640 as minimum
    const int MIN_HEIGHT = 640; // BOARD_SIZE (640)

    if (width < MIN_WIDTH || height < MIN_HEIGHT)
    {
        // Resize to minimum size if window is too small
        window.setSize(sf::Vector2u(MIN_WIDTH, MIN_HEIGHT));
        width = MIN_WIDTH;
        height = MIN_HEIGHT;
    }

    // Update current window dimensions
    currentWindowWidth = width;
    currentWindowHeight = height;

    // Calculate scaling factors
    scaleX = static_cast<float>(width) / static_cast<float>(WINDOW_WIDTH);
    scaleY = static_cast<float>(height) / static_cast<float>(WINDOW_HEIGHT);

    // Use the smaller scale to maintain aspect ratio
    float scale = std::min(scaleX, scaleY);

    // Update scaled dimensions
    currentSquareSize = SQUARE_SIZE * scale;
    currentBoardSize = BOARD_SIZE * scale;
    currentInfoPanelWidth = INFO_PANEL_WIDTH * scale;

    // Update the view to maintain proper aspect ratio
    sf::View view = window.getView();
    view.setSize(sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
    view.setCenter(sf::Vector2f(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f));
    window.setView(view);
}

bool ChessGUI::tryMakeMove(int fromSquare, int toSquare)
{
    // Get all legal moves from the source square
    std::vector<Move> legalMoves = board.getLegalMovesFrom(fromSquare);

    // Find the move that matches from->to
    Move *foundMove = nullptr;
    std::vector<Move> promotionMoves;

    for (Move &move : legalMoves)
    {
        if (move.from == fromSquare && move.to == toSquare)
        {
            if (move.flags & FLAG_PROMOTION)
            {
                promotionMoves.push_back(move);
            }
            else
            {
                foundMove = &move;
                break;
            }
        }
    }

    // Handle promotion
    if (!promotionMoves.empty())
    {
        showPromotionDialog(promotionMoves, toSquare);
        return false; // Don't make the move yet, wait for user selection
    }

    if (foundMove)
    {
        // Store move for history
        std::string moveStr = formatMove(*foundMove, board.getFullMoveNumber());

        // Make the move
        lastMoveFrom = fromSquare;
        lastMoveTo = toSquare;

        board.makeMove(*foundMove);
        moveHistory.push_back(moveStr);

        updateGameState();
        engineTurn = -engineTurn;
        return true;
    }

    return false;
}

int ChessGUI::selectPromotionPiece()
{
    // This function is now handled by the promotion dialog
    // Return a default value (should not be called when dialog is active)
    int side = board.getSideToMove();
    return (side == 0) ? WHITE_QUEEN : BLACK_QUEEN;
}

// ==================== RENDERING ====================

void ChessGUI::render()
{
    window.clear(sf::Color(50, 50, 50));

    drawBoard();
    drawHighlights();
    drawPieces();
    drawDraggedPiece();
    drawInfoPanel();

    // Draw promotion dialog if active
    if (showingPromotionDialog)
    {
        drawPromotionDialog();
    }

    window.display();
}

void ChessGUI::drawBoard()
{
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            sf::RectangleShape square(sf::Vector2f(currentSquareSize, currentSquareSize));
            square.setPosition(sf::Vector2f(file * currentSquareSize, (7 - rank) * currentSquareSize));

            // Checkerboard pattern
            if ((rank + file) % 2 == 0)
            {
                square.setFillColor(lightSquareColor);
            }
            else
            {
                square.setFillColor(darkSquareColor);
            }

            window.draw(square);
        }
    }

    // Draw coordinate labels if font is loaded
    if (font.getInfo().family != "")
    {
        float fontSize = 16.0f * std::min(scaleX, scaleY);

        // File labels (a-h)
        for (int file = 0; file < 8; file++)
        {
            sf::Text text(font, std::string(1, 'a' + file), static_cast<unsigned int>(fontSize));
            text.setFillColor(file % 2 == 1 ? lightSquareColor : darkSquareColor);
            text.setPosition(sf::Vector2f(file * currentSquareSize + 5, currentBoardSize - 20));
            window.draw(text);
        }

        // Rank labels (1-8)
        for (int rank = 0; rank < 8; rank++)
        {
            sf::Text text(font, std::to_string(rank + 1), static_cast<unsigned int>(fontSize));
            text.setFillColor(rank % 2 == 0 ? lightSquareColor : darkSquareColor);
            text.setPosition(sf::Vector2f(currentBoardSize - 15, (7 - rank) * currentSquareSize + 5));
            window.draw(text);
        }
    }
}

void ChessGUI::drawPieces()
{
    for (int square88 = 0; square88 < 128; square88++)
    {
        if (square88 & 0x88)
            continue; // Skip off-board squares

        // Don't draw the dragged piece in its original position
        if (isDragging && square88 == draggedSquare)
            continue;

        int piece = board.getPiece(square88);
        if (piece == EMPTY)
            continue;

        sf::Vector2f pos = squareToScreen(square88);
        drawPiece(piece, pos.x, pos.y);
    }
}

void ChessGUI::drawPiece(int piece, float x, float y, float scale)
{
    if (piece == EMPTY)
        return;

    // Check if we have a texture for this piece
    if (pieceTextures.find(piece) != pieceTextures.end())
    {
        // Draw piece using texture
        sf::Sprite sprite(pieceTextures[piece]);

        // Scale the sprite to fit the square
        float pieceSize = currentSquareSize * 0.9f * scale; // 90% of square size
        sf::Vector2u textureSize = pieceTextures[piece].getSize();
        float scaleFactor = pieceSize / std::max(textureSize.x, textureSize.y);
        sprite.setScale(sf::Vector2f(scaleFactor, scaleFactor));

        // Center the piece in the square
        sf::Vector2f spriteSize = sf::Vector2f(
            textureSize.x * scaleFactor,
            textureSize.y * scaleFactor);
        sprite.setPosition(sf::Vector2f(
            x + (currentSquareSize - spriteSize.x) / 2,
            y + (currentSquareSize - spriteSize.y) / 2));

        window.draw(sprite);
    }
    else
    {
        // Fallback: draw piece as a circle with letter if texture not found
        float radius = currentSquareSize * 0.35f * scale;
        sf::CircleShape circle(radius);

        circle.setPosition(sf::Vector2f(x + currentSquareSize / 2 - radius, y + currentSquareSize / 2 - radius));
        circle.setFillColor(piece > 0 ? sf::Color(255, 255, 255) : sf::Color(50, 50, 50));
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color(0, 0, 0, 150));

        window.draw(circle);

        // Draw piece letter if font is available
        if (font.getInfo().family != "")
        {
            float fontSize = 40.0f * scale * std::min(scaleX, scaleY);
            sf::Text text(font, std::string(1, pieceToChar(piece)), static_cast<unsigned int>(fontSize));
            text.setFillColor(piece > 0 ? sf::Color(100, 100, 100) : sf::Color(220, 220, 220));
            text.setStyle(sf::Text::Bold);

            // Center the text (simplified for SFML 3.0)
            text.setPosition(sf::Vector2f(x + currentSquareSize / 2 - 10,
                                          y + currentSquareSize / 2 - 15));

            window.draw(text);
        }
    }
}

void ChessGUI::drawDraggedPiece()
{
    if (isDragging && draggedPiece != EMPTY)
    {
        float x = dragPosition.x - currentSquareSize / 2;
        float y = dragPosition.y - currentSquareSize / 2;
        drawPiece(draggedPiece, x, y, 1.2f);
    }
}

void ChessGUI::drawHighlights()
{
    // Highlight last move
    if (lastMoveFrom >= 0)
    {
        highlightSquare(lastMoveFrom, lastMoveColor);
    }
    if (lastMoveTo >= 0)
    {
        highlightSquare(lastMoveTo, lastMoveColor);
    }

    // Highlight selected square
    if (selectedSquare >= 0 && !isDragging)
    {
        highlightSquare(selectedSquare, highlightColor);
    }

    // Draw legal move indicators
    drawLegalMoves();
}

void ChessGUI::drawLegalMoves()
{
    for (const Move &move : selectedMoves)
    {
        sf::Vector2f center = squareCenter(move.to);

        // Draw circle for legal moves
        float radius = currentSquareSize * 0.15f;
        if (board.getPiece(move.to) != EMPTY)
        {
            radius = currentSquareSize * 0.4f; // Larger circle for captures
        }

        sf::CircleShape indicator(radius);
        indicator.setPosition(sf::Vector2f(center.x - radius, center.y - radius));
        indicator.setFillColor(legalMoveColor);

        window.draw(indicator);
    }
}

void ChessGUI::highlightSquare(int square88, sf::Color color)
{
    if (square88 < 0 || (square88 & 0x88))
        return;

    sf::Vector2f pos = squareToScreen(square88);
    sf::RectangleShape highlight(sf::Vector2f(currentSquareSize, currentSquareSize));
    highlight.setPosition(pos);
    highlight.setFillColor(color);

    window.draw(highlight);
}

void ChessGUI::drawInfoPanel()
{
    // Background for info panel
    sf::RectangleShape panel(sf::Vector2f(currentInfoPanelWidth, currentWindowHeight));
    panel.setPosition(sf::Vector2f(currentBoardSize, 0));
    panel.setFillColor(sf::Color(40, 40, 40));
    window.draw(panel);

    if (font.getInfo().family == "")
        return; // No font loaded

    float yOffset = 20 * std::min(scaleX, scaleY);

    // Title
    float titleSize = 24.0f * std::min(scaleX, scaleY);
    sf::Text title(font, "Chess Engine", static_cast<unsigned int>(titleSize));
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(title);
    yOffset += 40 * std::min(scaleX, scaleY);

    // Current turn
    float turnSize = 18.0f * std::min(scaleX, scaleY);
    sf::Text turnText(font, std::string("Turn: ") + (board.getSideToMove() == 0 ? "White" : "Black"), static_cast<unsigned int>(turnSize));
    turnText.setFillColor(sf::Color::White);
    turnText.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(turnText);
    yOffset += 30 * std::min(scaleX, scaleY);

    // Game status
    drawGameStatus();

    // Move history
    yOffset = 150 * std::min(scaleX, scaleY);
    float historyTitleSize = 18.0f * std::min(scaleX, scaleY);
    sf::Text historyTitle(font, "Move History:", static_cast<unsigned int>(historyTitleSize));
    historyTitle.setFillColor(sf::Color::White);
    historyTitle.setStyle(sf::Text::Bold);
    historyTitle.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(historyTitle);
    yOffset += 30 * std::min(scaleX, scaleY);

    drawMoveHistory();

    // Controls
    yOffset = currentWindowHeight - 100 * std::min(scaleX, scaleY);
    float controlsSize = 14.0f * std::min(scaleX, scaleY);
    sf::Text controls(font, "Controls:\nU - Undo\nR - Reset", static_cast<unsigned int>(controlsSize));
    controls.setFillColor(sf::Color(150, 150, 150));
    controls.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(controls);
}

void ChessGUI::drawMoveHistory()
{
    if (font.getInfo().family == "")
        return;

    float yOffset = 180 * std::min(scaleX, scaleY);
    int displayCount = std::min(15, (int)moveHistory.size());
    int startIndex = std::max(0, (int)moveHistory.size() - displayCount);

    float moveTextSize = 14.0f * std::min(scaleX, scaleY);

    for (int i = startIndex; i < moveHistory.size(); i++)
    {
        sf::Text moveText(font, moveHistory[i], static_cast<unsigned int>(moveTextSize));
        moveText.setFillColor(sf::Color(200, 200, 200));
        moveText.setPosition(sf::Vector2f(currentBoardSize + 25, yOffset));
        window.draw(moveText);
        yOffset += 20 * std::min(scaleX, scaleY);
    }
}

void ChessGUI::drawGameStatus()
{
    if (font.getInfo().family == "")
        return;

    float yOffset = 100 * std::min(scaleX, scaleY);
    float statusSize = 16.0f * std::min(scaleX, scaleY);
    sf::Text statusText(font, "", static_cast<unsigned int>(statusSize));
    statusText.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));

    if (gameOver)
    {
        statusText.setString(gameResult);
        statusText.setFillColor(sf::Color::Red);
        statusText.setStyle(sf::Text::Bold);
    }
    else if (board.isCheck(board.getSideToMove()))
    {
        statusText.setString("CHECK!");
        statusText.setFillColor(sf::Color::Red);
        statusText.setStyle(sf::Text::Bold);
    }
    else
    {
        statusText.setString("In progress");
        statusText.setFillColor(sf::Color::Green);
    }

    window.draw(statusText);
}

void ChessGUI::showPromotionDialog(const std::vector<Move> &moves, int square)
{
    showingPromotionDialog = true;
    promotionSquare = square;
    pendingPromotionMoves = moves;
}

void ChessGUI::drawPromotionDialog()
{
    if (pendingPromotionMoves.empty())
        return;

    // Calculate dialog position (center of the board)
    float dialogWidth = currentSquareSize * 6.0f;
    float dialogHeight = currentSquareSize * 2.5f;
    float dialogX = (currentBoardSize - dialogWidth) / 2.0f;
    float dialogY = (currentBoardSize - dialogHeight) / 2.0f;

    // Draw dialog background
    sf::RectangleShape dialog(sf::Vector2f(dialogWidth, dialogHeight));
    dialog.setPosition(sf::Vector2f(dialogX, dialogY));
    dialog.setFillColor(sf::Color(50, 50, 50, 220));
    dialog.setOutlineThickness(3);
    dialog.setOutlineColor(sf::Color::White);
    window.draw(dialog);

    // Draw title
    if (font.getInfo().family != "")
    {
        float titleSize = 20.0f * std::min(scaleX, scaleY);
        sf::Text title(font, "Choose Promotion Piece", static_cast<unsigned int>(titleSize));
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold);
        title.setPosition(sf::Vector2f(dialogX + 10, dialogY + 10));
        window.draw(title);
    }

    // Draw promotion piece options
    float squareSize = currentSquareSize;
    float pieceSpacing = currentSquareSize * 1.2f;

    // Calculate the total width of all squares including spacing
    size_t numPieces = std::min(pendingPromotionMoves.size(), size_t(4));
    float totalWidth = (numPieces * squareSize) + ((numPieces - 1) * (pieceSpacing - squareSize));

    // Center the entire row of squares horizontally
    float startX = dialogX + (dialogWidth - totalWidth) / 2.0f;
    float squareY = dialogY + dialogHeight * 0.4f;

    for (size_t i = 0; i < numPieces; i++)
    {
        // Calculate square position
        float squareX = startX + i * pieceSpacing;

        // Draw piece background square
        sf::RectangleShape pieceBg(sf::Vector2f(squareSize, squareSize));
        pieceBg.setPosition(sf::Vector2f(squareX, squareY));
        pieceBg.setFillColor(sf::Color(240, 240, 240, 200));
        pieceBg.setOutlineThickness(2);
        pieceBg.setOutlineColor(sf::Color::Black);
        window.draw(pieceBg);

        // Draw piece at the same position as the square
        int piece = pendingPromotionMoves[i].promotionPiece;
        drawPiece(piece, squareX, squareY, 1.0f);
    }
}

bool ChessGUI::handlePromotionClick(int x, int y)
{
    if (pendingPromotionMoves.empty())
        return false;

    // Calculate dialog position
    float dialogWidth = currentSquareSize * 4.5f;
    float dialogHeight = currentSquareSize * 1.5f;
    float dialogX = (currentBoardSize - dialogWidth) / 2.0f;
    float dialogY = (currentBoardSize - dialogHeight) / 2.0f;

    // Check if click is within dialog
    if (x < dialogX || x > dialogX + dialogWidth ||
        y < dialogY || y > dialogY + dialogHeight)
    {
        return false;
    }

    // Calculate which piece was clicked
    float pieceSize = currentSquareSize * 0.8f;
    float pieceSpacing = currentSquareSize * 1.1f;
    float startX = dialogX + (dialogWidth - pieceSpacing * 3) / 2.0f;
    float pieceY = dialogY + dialogHeight * 0.4f;

    float relativeX = x - startX;
    int pieceIndex = static_cast<int>(relativeX / pieceSpacing);

    if (pieceIndex >= 0 && pieceIndex < static_cast<int>(pendingPromotionMoves.size()))
    {
        // Find the selected move and execute it
        Move selectedMove = pendingPromotionMoves[pieceIndex];

        // Store move for history
        std::string moveStr = formatMove(selectedMove, board.getFullMoveNumber());

        // Make the move
        lastMoveFrom = selectedMove.from;
        lastMoveTo = selectedMove.to;

        board.makeMove(selectedMove);
        moveHistory.push_back(moveStr);

        updateGameState();

        // Clear promotion state
        pendingPromotionMoves.clear();
        return true;
    }

    return false;
}

void ChessGUI::drawCapturedPieces()
{
    // TODO: Implement captured pieces display
}

// ==================== COORDINATE CONVERSION ====================

int ChessGUI::screenToSquare(int x, int y) const
{
    // Check if click is on the board
    if (x < 0 || x >= currentBoardSize || y < 0 || y >= currentBoardSize)
    {
        return -1;
    }

    int file = static_cast<int>(x / currentSquareSize);
    int rank = 7 - static_cast<int>(y / currentSquareSize);

    if (file < 0 || file > 7 || rank < 0 || rank > 7)
    {
        return -1;
    }

    return makeSquare(file, rank);
}

sf::Vector2f ChessGUI::squareToScreen(int square88) const
{
    if (square88 & 0x88)
        return sf::Vector2f(-1, -1);

    int file = fileOf(square88);
    int rank = rankOf(square88);

    float x = file * currentSquareSize;
    float y = (7 - rank) * currentSquareSize;

    return sf::Vector2f(x, y);
}

sf::Vector2f ChessGUI::squareCenter(int square88) const
{
    sf::Vector2f topLeft = squareToScreen(square88);
    return sf::Vector2f(topLeft.x + currentSquareSize / 2, topLeft.y + currentSquareSize / 2);
}

// ==================== UTILITY ====================

void ChessGUI::updateGameState()
{
    checkGameOver();
}

void ChessGUI::checkGameOver()
{
    if (board.isCheckmate())
    {
        gameOver = true;
        gameResult = (board.getSideToMove() == 0) ? "Black Wins!" : "White Wins!";
        gameResult += "\nCheckmate!";
    }
    else if (board.isStalemate())
    {
        gameOver = true;
        gameResult = "Draw!\nStalemate";
    }
    else if (board.isDraw())
    {
        gameOver = true;
        gameResult = "Draw!";
    }
}

void ChessGUI::clearSelection()
{
    selectedSquare = -1;
    selectedMoves.clear();
}

std::string ChessGUI::formatMove(const Move &move, int moveNumber) const
{
    std::ostringstream oss;

    // Move number for white moves
    if (board.getSideToMove() == 0)
    {
        oss << moveNumber << ". ";
    }
    else if (moveHistory.empty() || moveHistory.back().find('.') != std::string::npos)
    {
        oss << moveNumber << "... ";
    }

    // Get piece character
    char pieceChar = pieceToChar(move.piece);
    if (std::abs(move.piece) != 1)
    { // Not a pawn
        oss << (char)std::toupper(pieceChar);
    }

    // Origin square for disambiguation (simplified - full implementation would be more complex)
    // oss << squareToAlgebraic(move.from);

    // Capture notation
    if (move.capturedPiece != 0)
    {
        if (std::abs(move.piece) == 1)
        { // Pawn capture
            oss << (char)('a' + fileOf(move.from));
        }
        oss << "x";
    }

    // Destination square
    oss << squareToAlgebraic(move.to);

    // Promotion
    if (move.flags & FLAG_PROMOTION)
    {
        oss << "=" << (char)std::toupper(pieceToChar(move.promotionPiece));
    }

    // Castling
    if (move.flags & FLAG_CASTLING_KINGSIDE)
    {
        return (board.getSideToMove() == 0 ? std::to_string(moveNumber) + ". " : "") + "O-O";
    }
    if (move.flags & FLAG_CASTLING_QUEENSIDE)
    {
        return (board.getSideToMove() == 0 ? std::to_string(moveNumber) + ". " : "") + "O-O-O";
    }

    return oss.str();
}
