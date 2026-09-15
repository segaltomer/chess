#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <map>
#include <optional>
#include <string>

class ChessFrontend {
public:
    enum class UiEventType {
        None,
        MoveRequested,
        ResignRequested,
        ResignConfirmed,
        ResignCancelled,
        PromotionSelected,
        DrawOfferRequested,
        DrawAccepted,
        DrawDeclined,
        CloseRequested
    };

    struct Square {
        int row = -1; // 0 = rank 8, 7 = rank 1
        int col = -1; // 0 = file a, 7 = file h
    };

    struct UiEvent {
        UiEventType type = UiEventType::None;
        Square from{};
        Square to{};
        char promotion = '\0'; // Q / R / B / N
    };

    ChessFrontend(unsigned width = 1050, unsigned height = 760);

    bool isOpen() const;

    // Board format:
    // K Q R B N P = white pieces
    // k q r b n p = black pieces
    // '.' or '#'   = empty square
    void setBoard(const std::array<char, 64>& board);

    // Names shown around the board. The black player is displayed above
    // the board and the white player below it.
    void setPlayerNames(const std::string& white,
                        const std::string& black);

    void setTurnText(const std::string& text);
    void setStatusText(const std::string& text);

    // Useful when the current game mode does not support draw offers
    // (for example, Player vs Bot).
    void setOfferDrawVisible(bool visible);

    // Disables/enables board-square input without changing the board display.
    // Useful while waiting for the opponent, the server, or the Bot.
    void setBoardInputEnabled(bool enabled);

    // YOUR chess engine decides when promotion is required.
    void showPromotionDialog(const std::string& playerText);

    // Call this if YOUR application decides to ask the player
    // to confirm a resignation after receiving ResignRequested.
    void showResignConfirmDialog();

    // Examples:
    // showGameOver("White wins!", "Checkmate");
    // showGameOver("Black wins!", "White resigned");
    // showGameOver("Draw", "Stalemate");
    void showGameOver(const std::string& title,
                      const std::string& detail);

    // Call this when YOUR application receives a draw offer
    // from the other player.
    void showDrawOfferDialog(const std::string& message);

    void hideGameOver();

    // The GUI reports user actions only.
    // It never decides whether a chess move is legal.
    UiEvent pollEvent();

    void render();

private:
    static constexpr int BOARD_N = 8;

    sf::RenderWindow window_;

    sf::Font font_;
    bool fontLoaded_ = false;

    std::map<char, sf::Texture> pieceTextures_;
    bool pieceImagesLoaded_ = false;

    std::array<char, 64> board_{};

    std::string whitePlayerName_ = "White Player";
    std::string blackPlayerName_ = "Black Player";

    std::string turnText_ = "White to move";
    std::string statusText_;

    std::optional<Square> selected_;

    bool promotionVisible_ = false;
    std::string promotionPlayerText_;

    bool gameOverVisible_ = false;
    std::string gameOverTitle_;
    std::string gameOverDetail_;

    bool drawOfferVisible_ = false;
    std::string drawOfferMessage_;

    bool resignConfirmVisible_ = false;

    bool offerDrawVisible_ = true;
    bool boardInputEnabled_ = true;

    float boardX_ = 55.f;
    float boardY_ = 80.f;
    float squareSize_ = 72.f;

    sf::FloatRect resignButton_{};
    sf::FloatRect offerDrawButton_{};
    sf::FloatRect closeSummaryButton_{};

    sf::FloatRect drawAcceptButton_{};
    sf::FloatRect drawDeclineButton_{};

    sf::FloatRect resignConfirmButton_{};
    sf::FloatRect resignCancelButton_{};

    bool loadSystemFont();
    bool loadPieceImages();

    static bool isInside(const sf::FloatRect& rect,
                         sf::Vector2f point);

    static std::string pieceLabel(char piece);
    static bool isWhitePiece(char piece);

    std::optional<Square>
    mouseToSquare(sf::Vector2f mouse) const;

    void drawBoard();
    void drawPieceImage(char piece, int row, int col);
    void drawFallbackPiece(char piece, int row, int col);

    void drawSidePanel();
    void drawPromotionDialog();
    void drawDrawOfferDialog();
    void drawResignConfirmDialog();
    void drawGameOver();

    void drawCenteredText(const std::string& text,
                          unsigned size,
                          float centerX,
                          float y,
                          sf::Color color = sf::Color::White,
                          bool bold = false);

    void drawText(const std::string& text,
                  unsigned size,
                  sf::Vector2f position,
                  sf::Color color = sf::Color::White,
                  bool bold = false);
};
