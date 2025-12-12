#include "../include/GUI.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
using namespace std;
using namespace chrono;

ChessGUI::ChessGUI(Board &b, int enginePlay, Engine &e1, Engine &e2, int startTimeMs, int incrementMs)
    : board(b),
      engine1(e1),
      engine2(e2),
      engineTurn(enginePlay),
      whiteTimeMs(startTimeMs),
      blackTimeMs(startTimeMs),
      initialTimeMs(startTimeMs),
      incrementMs(incrementMs),
      lastTimerUpdate(steady_clock::now())
{
    window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
                  "Chess Engine - DSA Project",
                  sf::Style::Default);
    window.setFramerateLimit(60);

    selectedSquare = -1;
    isDragging = false;
    draggedPiece = EMPTY;
    draggedSquare = -1;
    lastMoveFrom = -1;
    lastMoveTo = -1;
    gameOver = false;

    currentWindowWidth = WINDOW_WIDTH;
    currentWindowHeight = WINDOW_HEIGHT;
    scaleX = 1.0f;
    scaleY = 1.0f;
    currentSquareSize = SQUARE_SIZE;
    currentBoardSize = BOARD_SIZE;
    currentInfoPanelWidth = INFO_PANEL_WIDTH;

    showingPromotionDialog = false;
    promotionSquare = -1;

    initializeColors();
    loadResources();
}

void ChessGUI::initializeColors()
{
    lightSquareColor = sf::Color(240, 217, 181);
    darkSquareColor = sf::Color(181, 136, 99);
    highlightColor = sf::Color(255, 255, 0, 100);
    legalMoveColor = sf::Color(100, 200, 100, 120);
    lastMoveColor = sf::Color(255, 255, 100, 80);
}

void ChessGUI::loadResources()
{
    map<int, string> pieceFiles = {
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
        }
        else
        {
            cout << "Failed to load texture: " << pair.second << endl;
        }
    }

    if (!font.openFromFile("assets/fonts/ARIAL.ttf"))
    {
        cout << "Note: Font file not found. Text rendering disabled.\n";
    }
}

void ChessGUI::run()
{
    while (window.isOpen())
    {
        updateTimers();
        if (engineTurn != 2)
        {
            handleEvents();
            render();
            updateTimers();
            if (!gameOver && !board.isCheckmate() && !board.isDraw() && engineTurn > 0)
            {
                int movingSide = board.getSideToMove();
                auto start = steady_clock::now();

                makeEngineMove(engine1);
                auto end = steady_clock::now();
                auto duration = duration_cast<milliseconds>(end - start);

                handleMoveTiming(movingSide, duration.count());
                cout << "Time taken: " << duration.count() << " ms\n";
                engineTurn = -engineTurn;
            }
        }
        else
        {
            render();
            updateTimers();

            if (!gameOver && !board.isCheckmate() && !board.isDraw())
            {
                int movingSide = board.getSideToMove();
                auto start = steady_clock::now();

                makeEngineMove(engine1);
                auto end = steady_clock::now();
                auto duration = duration_cast<milliseconds>(end - start);

                handleMoveTiming(movingSide, duration.count());
                cout << "Time taken: " << duration.count() << " ms\n";

                render();
                updateTimers();

                movingSide = board.getSideToMove();
                start = steady_clock::now();

                makeEngineMove(engine2);
                end = steady_clock::now();
                duration = duration_cast<milliseconds>(end - start);

                handleMoveTiming(movingSide, duration.count());
                cout << "Time taken: " << duration.count() << " ms\n";
            }
            else
            {
                this_thread::sleep_for(std::chrono::seconds(2));
                window.close();
            }
        }
    }
}
void ChessGUI::makeEngineMove(Engine &engine)
{
    if (gameOver)
        return;
    Move engineMove = engine.getBestMove();
    lastMoveFrom = engineMove.from;
    lastMoveTo = engineMove.to;
    string moveStr = formatMove(engineMove, board.getFullMoveNumber());
    board.makeMove(engineMove);
    moveHistory.push_back(moveStr);
    updateGameState();
}

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

        if (auto keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::U)
            {
                if (!gameOver)
                {
                    board.undoMove();
                    if (!moveHistory.empty())
                    {
                        moveHistory.pop_back();
                    }
                    clearSelection();
                    checkGameOver();
                    lastTimerUpdate = steady_clock::now();
                }
            }

            if (keyPressed->code == sf::Keyboard::Key::R)
            {
                board.setStartingPosition();
                moveHistory.clear();
                clearSelection();
                gameOver = false;
                gameResult = "";
                whiteTimeMs = initialTimeMs;
                blackTimeMs = initialTimeMs;
                lastTimerUpdate = steady_clock::now();
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

    if (piece != EMPTY)
    {
        int side = board.getSideToMove();
        if ((side == 0 && piece > 0) || (side == 1 && piece < 0))
        {
            if (selectedSquare == square)
            {
                clearSelection();
                return;
            }

            isDragging = true;
            draggedPiece = piece;
            draggedSquare = square;
            dragPosition = sf::Vector2f(x, y);
            selectedSquare = square;
            selectedMoves = board.getLegalMovesFrom(square);
            return;
        }
    }

    if (selectedSquare >= 0)
    {
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
    const int MIN_WIDTH = 640;
    const int MIN_HEIGHT = 640;

    if (width < MIN_WIDTH || height < MIN_HEIGHT)
    {
        window.setSize(sf::Vector2u(MIN_WIDTH, MIN_HEIGHT));
        width = MIN_WIDTH;
        height = MIN_HEIGHT;
    }

    currentWindowWidth = width;
    currentWindowHeight = height;
    scaleX = static_cast<float>(width) / static_cast<float>(WINDOW_WIDTH);
    scaleY = static_cast<float>(height) / static_cast<float>(WINDOW_HEIGHT);
    float scale = min(scaleX, scaleY);
    currentSquareSize = SQUARE_SIZE * scale;
    currentBoardSize = BOARD_SIZE * scale;
    currentInfoPanelWidth = INFO_PANEL_WIDTH * scale;

    sf::View view = window.getView();
    view.setSize(sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
    view.setCenter(sf::Vector2f(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f));
    window.setView(view);
}

bool ChessGUI::tryMakeMove(int fromSquare, int toSquare)
{
    vector<Move> legalMoves = board.getLegalMovesFrom(fromSquare);
    Move *foundMove = nullptr;
    vector<Move> promotionMoves;

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

    if (!promotionMoves.empty())
    {
        showPromotionDialog(promotionMoves, toSquare);
        return false;
    }

    if (foundMove)
    {
        int movingSide = board.getSideToMove();
        updateTimers();
        string moveStr = formatMove(*foundMove, board.getFullMoveNumber());
        lastMoveFrom = fromSquare;
        lastMoveTo = toSquare;
        cout << foundMove->toAlgebraic() << " ";
        board.makeMove(*foundMove);
        moveHistory.push_back(moveStr);
        updateGameState();
        handleMoveTiming(movingSide);
        engineTurn = -engineTurn;
        return true;
    }

    return false;
}

int ChessGUI::selectPromotionPiece()
{
    int side = board.getSideToMove();
    return (side == 0) ? WHITE_QUEEN : BLACK_QUEEN;
}

void ChessGUI::render()
{
    window.clear(sf::Color(50, 50, 50));
    drawBoard();
    drawHighlights();
    drawPieces();
    drawDraggedPiece();
    drawInfoPanel();

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

    if (font.getInfo().family != "")
    {
        float fontSize = 16.0f * min(scaleX, scaleY);

        for (int file = 0; file < 8; file++)
        {
            sf::Text text(font, string(1, 'a' + file), static_cast<unsigned int>(fontSize));
            text.setFillColor(file % 2 == 1 ? lightSquareColor : darkSquareColor);
            text.setPosition(sf::Vector2f(file * currentSquareSize + 5, currentBoardSize - 20));
            window.draw(text);
        }

        for (int rank = 0; rank < 8; rank++)
        {
            sf::Text text(font, to_string(rank + 1), static_cast<unsigned int>(fontSize));
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
            continue;

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

    if (pieceTextures.find(piece) != pieceTextures.end())
    {
        sf::Sprite sprite(pieceTextures[piece]);
        float pieceSize = currentSquareSize * 0.9f * scale;
        sf::Vector2u textureSize = pieceTextures[piece].getSize();
        float scaleFactor = pieceSize / max(textureSize.x, textureSize.y);
        sprite.setScale(sf::Vector2f(scaleFactor, scaleFactor));
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
        float radius = currentSquareSize * 0.35f * scale;
        sf::CircleShape circle(radius);
        circle.setPosition(sf::Vector2f(x + currentSquareSize / 2 - radius, y + currentSquareSize / 2 - radius));
        circle.setFillColor(piece > 0 ? sf::Color(255, 255, 255) : sf::Color(50, 50, 50));
        circle.setOutlineThickness(2);
        circle.setOutlineColor(sf::Color(0, 0, 0, 150));
        window.draw(circle);

        if (font.getInfo().family != "")
        {
            float fontSize = 40.0f * scale * min(scaleX, scaleY);
            sf::Text text(font, string(1, pieceToChar(piece)), static_cast<unsigned int>(fontSize));
            text.setFillColor(piece > 0 ? sf::Color(100, 100, 100) : sf::Color(220, 220, 220));
            text.setStyle(sf::Text::Bold);
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
    if (lastMoveFrom >= 0)
    {
        highlightSquare(lastMoveFrom, lastMoveColor);
    }
    if (lastMoveTo >= 0)
    {
        highlightSquare(lastMoveTo, lastMoveColor);
    }

    if (selectedSquare >= 0 && !isDragging)
    {
        highlightSquare(selectedSquare, highlightColor);
    }

    drawLegalMoves();
}

void ChessGUI::drawLegalMoves()
{
    for (const Move &move : selectedMoves)
    {
        sf::Vector2f center = squareCenter(move.to);
        float radius = currentSquareSize * 0.15f;
        if (board.getPiece(move.to) != EMPTY)
        {
            radius = currentSquareSize * 0.4f;
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

    float yOffset = 20 * min(scaleX, scaleY);

    // Title
    float titleSize = 24.0f * min(scaleX, scaleY);
    sf::Text title(font, "Chess Engine", static_cast<unsigned int>(titleSize));
    title.setFillColor(sf::Color::White);
    title.setStyle(sf::Text::Bold);
    title.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(title);
    yOffset += 40 * min(scaleX, scaleY);

    // Current turn
    float turnSize = 18.0f * min(scaleX, scaleY);
    sf::Text turnText(font, string("Turn: ") + (board.getSideToMove() == 0 ? "White" : "Black"), static_cast<unsigned int>(turnSize));
    turnText.setFillColor(sf::Color::White);
    turnText.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(turnText);
    yOffset += 30 * min(scaleX, scaleY);

    // Player clocks
    drawClocks();

    // Game status
    drawGameStatus();

    // Move history
    yOffset = 190 * min(scaleX, scaleY);
    float historyTitleSize = 18.0f * min(scaleX, scaleY);
    sf::Text historyTitle(font, "Move History:", static_cast<unsigned int>(historyTitleSize));
    historyTitle.setFillColor(sf::Color::White);
    historyTitle.setStyle(sf::Text::Bold);
    historyTitle.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(historyTitle);
    yOffset += 30 * min(scaleX, scaleY);

    drawMoveHistory();

    // Controls
    yOffset = currentWindowHeight - 100 * min(scaleX, scaleY);
    float controlsSize = 14.0f * min(scaleX, scaleY);
    sf::Text controls(font, "Controls:\nU - Undo\nR - Reset", static_cast<unsigned int>(controlsSize));
    controls.setFillColor(sf::Color(150, 150, 150));
    controls.setPosition(sf::Vector2f(currentBoardSize + 20, yOffset));
    window.draw(controls);
}

void ChessGUI::drawClocks()
{
    if (font.getInfo().family == "")
        return;

    float scale = min(scaleX, scaleY);
    float baseY = 80.0f * scale;
    float lineHeight = 22.0f * scale;
    float clockSize = 18.0f * scale;

    sf::Color activeColor(180, 255, 180);
    sf::Color inactiveColor = sf::Color::White;

    sf::Text whiteClock(font, "White: " + formatTime(whiteTimeMs), static_cast<unsigned int>(clockSize));
    whiteClock.setFillColor(board.getSideToMove() == 0 ? activeColor : inactiveColor);
    whiteClock.setPosition(sf::Vector2f(currentBoardSize + 20, baseY));
    window.draw(whiteClock);

    sf::Text blackClock(font, "Black: " + formatTime(blackTimeMs), static_cast<unsigned int>(clockSize));
    blackClock.setFillColor(board.getSideToMove() == 1 ? activeColor : inactiveColor);
    blackClock.setPosition(sf::Vector2f(currentBoardSize + 20, baseY + lineHeight));
    window.draw(blackClock);
}

void ChessGUI::drawMoveHistory()
{
    if (font.getInfo().family == "")
        return;

    float yOffset = 180 * min(scaleX, scaleY);
    int displayCount = min(15, (int)moveHistory.size());
    int startIndex = max(0, (int)moveHistory.size() - displayCount);

    float moveTextSize = 14.0f * min(scaleX, scaleY);

    for (int i = startIndex; i < moveHistory.size(); i++)
    {
        sf::Text moveText(font, moveHistory[i], static_cast<unsigned int>(moveTextSize));
        moveText.setFillColor(sf::Color(200, 200, 200));
        moveText.setPosition(sf::Vector2f(currentBoardSize + 25, yOffset));
        window.draw(moveText);
        yOffset += 20 * min(scaleX, scaleY);
    }
}

void ChessGUI::drawGameStatus()
{
    if (font.getInfo().family == "")
        return;

    float yOffset = 140 * min(scaleX, scaleY);
    float statusSize = 16.0f * min(scaleX, scaleY);
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

void ChessGUI::showPromotionDialog(const vector<Move> &moves, int square)
{
    showingPromotionDialog = true;
    promotionSquare = square;
    pendingPromotionMoves = moves;
}

void ChessGUI::drawPromotionDialog()
{
    if (pendingPromotionMoves.empty())
        return;

    float dialogWidth = currentSquareSize * 6.0f;
    float dialogHeight = currentSquareSize * 2.5f;
    float dialogX = (currentBoardSize - dialogWidth) / 2.0f;
    float dialogY = (currentBoardSize - dialogHeight) / 2.0f;

    sf::RectangleShape dialog(sf::Vector2f(dialogWidth, dialogHeight));
    dialog.setPosition(sf::Vector2f(dialogX, dialogY));
    dialog.setFillColor(sf::Color(50, 50, 50, 220));
    dialog.setOutlineThickness(3);
    dialog.setOutlineColor(sf::Color::White);
    window.draw(dialog);

    if (font.getInfo().family != "")
    {
        float titleSize = 20.0f * min(scaleX, scaleY);
        sf::Text title(font, "Choose Promotion Piece", static_cast<unsigned int>(titleSize));
        title.setFillColor(sf::Color::White);
        title.setStyle(sf::Text::Bold);
        title.setPosition(sf::Vector2f(dialogX + 10, dialogY + 10));
        window.draw(title);
    }

    float squareSize = currentSquareSize;
    float pieceSpacing = currentSquareSize * 1.2f;
    size_t numPieces = min(pendingPromotionMoves.size(), size_t(4));
    float totalWidth = (numPieces * squareSize) + ((numPieces - 1) * (pieceSpacing - squareSize));
    float startX = dialogX + (dialogWidth - totalWidth) / 2.0f;
    float squareY = dialogY + dialogHeight * 0.4f;

    for (size_t i = 0; i < numPieces; i++)
    {
        float squareX = startX + i * pieceSpacing;
        sf::RectangleShape pieceBg(sf::Vector2f(squareSize, squareSize));
        pieceBg.setPosition(sf::Vector2f(squareX, squareY));
        pieceBg.setFillColor(sf::Color(240, 240, 240, 200));
        pieceBg.setOutlineThickness(2);
        pieceBg.setOutlineColor(sf::Color::Black);
        window.draw(pieceBg);

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
        int movingSide = board.getSideToMove();
        updateTimers();
        // Find the selected move and execute it
        Move selectedMove = pendingPromotionMoves[pieceIndex];

        // Store move for history
        string moveStr = formatMove(selectedMove, board.getFullMoveNumber());

        // Make the move
        lastMoveFrom = selectedMove.from;
        lastMoveTo = selectedMove.to;

        board.makeMove(selectedMove);
        moveHistory.push_back(moveStr);

        updateGameState();
        handleMoveTiming(movingSide);
        engineTurn = -engineTurn;

        // Clear promotion state
        pendingPromotionMoves.clear();
        return true;
    }

    return false;
}

void ChessGUI::drawCapturedPieces()
{
}

int ChessGUI::screenToSquare(int x, int y) const
{
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

string ChessGUI::formatMove(const Move &move, int moveNumber) const
{
    ostringstream oss;

    // Move number for white moves
    if (board.getSideToMove() == 0)
    {
        oss << moveNumber << ". ";
    }
    else if (moveHistory.empty() || moveHistory.back().find('.') != string::npos)
    {
        oss << moveNumber << "... ";
    }

    // Get piece character
    char pieceChar = pieceToChar(move.piece);
    if (abs(move.piece) != 1)
    { // Not a pawn
        oss << (char)toupper(pieceChar);
    }

    if (move.capturedPiece != 0)
    {
        if (abs(move.piece) == 1)
        {
            oss << (char)('a' + fileOf(move.from));
        }
        oss << "x";
    }

    oss << squareToAlgebraic(move.to);

    if (move.flags & FLAG_PROMOTION)
    {
        oss << "=" << (char)toupper(pieceToChar(move.promotionPiece));
    }
    if (move.flags & FLAG_CASTLING_KINGSIDE)
    {
        return (board.getSideToMove() == 0 ? to_string(moveNumber) + ". " : "") + "O-O";
    }
    if (move.flags & FLAG_CASTLING_QUEENSIDE)
    {
        return (board.getSideToMove() == 0 ? to_string(moveNumber) + ". " : "") + "O-O-O";
    }

    return oss.str();
}

void ChessGUI::updateTimers()
{
    if (gameOver)
    {
        lastTimerUpdate = steady_clock::now();
        return;
    }

    auto now = steady_clock::now();
    long long elapsed = duration_cast<milliseconds>(now - lastTimerUpdate).count();
    if (elapsed <= 0)
        return;

    int activeSide = board.getSideToMove();
    long long &timeRef = (activeSide == 0) ? whiteTimeMs : blackTimeMs;
    timeRef = max(0LL, timeRef - elapsed);

    // Check if time has expired for either player
    if (whiteTimeMs <= 0)
    {
        gameOver = true;
        gameResult = "Black Wins!\nTime expired";
    }
    else if (blackTimeMs <= 0)
    {
        gameOver = true;
        gameResult = "White Wins!\nTime expired";
    }

    lastTimerUpdate = now;
}

void ChessGUI::handleMoveTiming(int movingSide, long long moveDurationMs)
{
    long long &timeRef = (movingSide == 0) ? whiteTimeMs : blackTimeMs;

    if (moveDurationMs > 0)
    {
        timeRef = max(0LL, timeRef - moveDurationMs);
    }

    timeRef = max(0LL, timeRef + static_cast<long long>(incrementMs));
    
    // Check if time has expired for either player after move timing
    if (whiteTimeMs <= 0)
    {
        gameOver = true;
        gameResult = "Black Wins!\nTime expired";
    }
    else if (blackTimeMs <= 0)
    {
        gameOver = true;
        gameResult = "White Wins!\nTime expired";
    }
    
    lastTimerUpdate = steady_clock::now();
}

string ChessGUI::formatTime(long long ms) const
{
    if (ms < 0)
        ms = 0;
    long long totalSeconds = ms / 1000;
    long long minutes = totalSeconds / 60;
    long long seconds = totalSeconds % 60;

    ostringstream oss;
    oss << setw(2) << setfill('0') << minutes
        << ":" << setw(2) << setfill('0') << seconds;
    return oss.str();
}
