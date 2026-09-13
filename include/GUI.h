#ifndef GUI_H
#define GUI_H

#include "Board.h"
#include "Engine.h"
#include <map>
#include <string>
#include <vector>
#include <chrono>

#ifdef __has_include
#if __has_include(<SFML/Graphics.hpp>)
#include <SFML/Graphics.hpp>
#else
#error "SFML/Graphics.hpp not found. Please install SFML and make sure the include path is set correctly."
#endif
#else
#include <SFML/Graphics.hpp>
#endif

class ChessGUI
{
private:
    Board &board;
    Engine &engine1;
    Engine &engine2;
    int noEngines = 0;
    int engineTurn = 0;
    sf::RenderWindow window;

    // Visual settings
    static const int SQUARE_SIZE = 80;
    static const int BOARD_SIZE = SQUARE_SIZE * 8;
    static const int INFO_PANEL_WIDTH = 300;
    static const int WINDOW_WIDTH = BOARD_SIZE + INFO_PANEL_WIDTH;
    static const int WINDOW_HEIGHT = BOARD_SIZE;

    // Colors
    sf::Color lightSquareColor;
    sf::Color darkSquareColor;
    sf::Color highlightColor;
    sf::Color legalMoveColor;
    sf::Color lastMoveColor;

    // Piece textures - load actual PNG images
    map<int, sf::Texture> pieceTextures;
    map<int, sf::Texture> spellTextures;

    // Fonts
    sf::Font font;

    // Interaction state
    int selectedSquare;
    std::vector<Move> selectedMoves;
    bool isDragging;
    int draggedPiece;
    int draggedSquare;
    sf::Vector2f dragPosition;

    // Game state
    std::vector<std::string> moveHistory;
    int lastMoveFrom;
    int lastMoveTo;
    bool gameOver;
    std::string gameResult;

    // Clock / timer state
    long long whiteTimeMs;
    long long blackTimeMs;
    long long initialTimeMs;
    int incrementMs;
    std::chrono::steady_clock::time_point lastTimerUpdate;

    // Scaling variables
    float scaleX;
    float scaleY;
    float currentSquareSize;
    float currentBoardSize;
    float currentInfoPanelWidth;
    int currentWindowWidth;
    int currentWindowHeight;

    // Promotion dialog state
    bool showingPromotionDialog;
    int promotionSquare;
    std::vector<Move> pendingPromotionMoves;

public:
    ChessGUI(Board &b, int enginePlay, Engine &e1, Engine &e2, int startTimeMs, int incrementMs);
    void run();

private:
    void loadResources();
    void makeEngineMove(Engine &engine);
    void initializeColors();
    void handleEvents();
    void handleMousePress(int x, int y);
    void handleMouseRelease(int x, int y);
    void handleMouseMove(int x, int y);
    bool tryMakeMove(int fromSquare, int toSquare);
    int selectPromotionPiece();
    void render();
    void drawBoard();
    void drawPieces();
    void drawPiece(int piece, float x, float y, float scale = 1.0f);
    void drawDraggedPiece();
    void drawHighlights();
    void drawLegalMoves();
    void highlightSquare(int square88, sf::Color color);
    void drawInfoPanel();
    void drawClocks();
    void drawMoveHistory();
    void drawGameStatus();
    void drawCapturedPieces();
    int screenToSquare(int x, int y) const;
    sf::Vector2f squareToScreen(int square88) const;
    sf::Vector2f squareCenter(int square88) const;
    void updateGameState();
    void checkGameOver();
    void clearSelection();
    void handleWindowResize(int width, int height);
    void showPromotionDialog(const std::vector<Move> &moves, int square);
    void drawPromotionDialog();
    bool handlePromotionClick(int x, int y);
    std::string formatMove(const Move &move, int moveNumber) const;
    void updateTimers();
    void handleMoveTiming(int movingSide, long long moveDurationMs = 0);
    std::string formatTime(long long ms) const;
};

#endif // GUI_H
