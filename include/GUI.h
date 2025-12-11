#ifndef GUI_H
#define GUI_H

#include "Board.h"
#include "Engine.h"
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <vector>

/**
 * ChessGUI - Graphical User Interface for the chess engine
 * Uses SFML for rendering and handles user interaction
 *
 * Features:
 * - Visual board display with piece rendering
 * - Mouse-based piece selection and movement
 * - Legal move highlighting
 * - Drag and drop piece movement
 * - Move history display
 * - Game status display
 */
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
    std::map<int, sf::Texture> pieceTextures;

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
    /**
     * Constructor
     * @param b - Reference to Board object
     */
    ChessGUI(Board &b, int enginePlay, Engine &e1, Engine &e2);
    /**
     * Main game loop - runs until window is closed
     * Time Complexity: O(∞) - runs continuously
     */
    void run();

private:
    // ==================== INITIALIZATION ====================

    /**
     * Loads textures and resources
     * In a full implementation, would load PNG files
     */
    void loadResources();

    void makeEngineMove(Engine &engine);
    /**
     * Initializes color scheme
     */
    void initializeColors();

    // ==================== EVENT HANDLING ====================

    /**
     * Handles all SFML events (mouse, keyboard, window)
     * Time Complexity: O(n) where n = number of events
     */
    void handleEvents();

    /**
     * Handles mouse button press
     */
    void handleMousePress(int x, int y);

    /**
     * Handles mouse button release
     */
    void handleMouseRelease(int x, int y);

    /**
     * Handles mouse movement (for dragging)
     */
    void handleMouseMove(int x, int y);

    /**
     * Attempts to make a move from selected square to target square
     * @return true if move was legal and made
     */
    bool tryMakeMove(int fromSquare, int toSquare);

    /**
     * Handles pawn promotion - shows promotion dialog
     * @return Promoted piece type
     */
    int selectPromotionPiece();

    // ==================== RENDERING ====================

    /**
     * Renders entire game state
     * Time Complexity: O(1) - constant number of squares
     */
    void render();

    /**
     * Draws the chess board (squares)
     */
    void drawBoard();

    /**
     * Draws all pieces on the board
     */
    void drawPieces();

    /**
     * Draws a single piece at a position
     */
    void drawPiece(int piece, float x, float y, float scale = 1.0f);

    /**
     * Draws the dragged piece following the mouse
     */
    void drawDraggedPiece();

    /**
     * Highlights squares (selected, legal moves, last move)
     */
    void drawHighlights();

    /**
     * Draws legal move indicators for selected piece
     */
    void drawLegalMoves();

    /**
     * Highlights a specific square
     */
    void highlightSquare(int square88, sf::Color color);

    /**
     * Draws the information panel (move history, status, etc.)
     */
    void drawInfoPanel();

    /**
     * Draws move history
     */
    void drawMoveHistory();

    /**
     * Draws game status (check, checkmate, etc.)
     */
    void drawGameStatus();

    /**
     * Draws captured pieces
     */
    void drawCapturedPieces();

    // ==================== COORDINATE CONVERSION ====================

    /**
     * Converts screen coordinates to 0x88 square
     * Time Complexity: O(1)
     * @param x - Screen X coordinate
     * @param y - Screen Y coordinate
     * @return 0x88 square index, or -1 if off board
     */
    int screenToSquare(int x, int y) const;

    /**
     * Converts 0x88 square to screen coordinates
     * Time Complexity: O(1)
     * @param square88 - 0x88 square index
     * @return Screen position (top-left of square)
     */
    sf::Vector2f squareToScreen(int square88) const;

    /**
     * Gets center of square in screen coordinates
     * Time Complexity: O(1)
     */
    sf::Vector2f squareCenter(int square88) const;

    // ==================== UTILITY ====================

    /**
     * Updates game state after a move
     */
    void updateGameState();

    /**
     * Checks if game is over and updates result
     */
    void checkGameOver();

    /**
     * Resets selection state
     */
    void clearSelection();

    /**
     * Handles window resize events
     * @param width - New window width
     * @param height - New window height
     */
    void handleWindowResize(int width, int height);

    /**
     * Shows promotion dialog and handles piece selection
     * @param moves - Available promotion moves
     * @param square - Square where promotion occurs
     */
    void showPromotionDialog(const std::vector<Move> &moves, int square);

    /**
     * Draws the promotion dialog
     */
    void drawPromotionDialog();

    /**
     * Handles promotion dialog mouse clicks
     * @param x - Mouse x coordinate
     * @param y - Mouse y coordinate
     * @return true if a piece was selected
     */
    bool handlePromotionClick(int x, int y);

    /**
     * Converts move to algebraic notation for display
     */
    std::string formatMove(const Move &move, int moveNumber) const;
};

#endif // GUI_H
