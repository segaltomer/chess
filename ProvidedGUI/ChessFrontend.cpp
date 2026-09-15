#include "ChessFrontend.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <utility>
#include <vector>

namespace {

sf::Color lightSquare() {
    return sf::Color(238, 220, 185);
}

sf::Color darkSquare() {
    return sf::Color(159, 111, 72);
}

sf::Color selectedColor() {
    return sf::Color(240, 218, 74);
}

sf::Color panelColor() {
    return sf::Color(30, 34, 41);
}

sf::Color buttonColor() {
    return sf::Color(76, 88, 105);
}

sf::Color dangerColor() {
    return sf::Color(158, 57, 57);
}

std::filesystem::path findAsset(const std::string& filename) {
    const std::vector<std::filesystem::path> candidates = {
        std::filesystem::path("ProvidedGUI") / "assets" / filename,
        std::filesystem::path("assets") / filename,
        std::filesystem::path("..") / "ProvidedGUI" / "assets" / filename,
        std::filesystem::path("..") / "assets" / filename,
        std::filesystem::path("..") / ".." / "ProvidedGUI" / "assets" / filename,
        std::filesystem::path("..") / ".." / "assets" / filename
    };

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    return {};
}

} // namespace

ChessFrontend::ChessFrontend(unsigned width, unsigned height)
    : window_(
          sf::VideoMode({width, height}),
          "C++ Chess",
          sf::Style::Titlebar | sf::Style::Close) {

    window_.setFramerateLimit(60);

    board_.fill('.');

    fontLoaded_ = loadSystemFont();
    pieceImagesLoaded_ = loadPieceImages();

    offerDrawButton_ =
        sf::FloatRect({720.f, 500.f}, {245.f, 52.f});

    resignButton_ =
        sf::FloatRect({720.f, 565.f}, {245.f, 58.f});

    closeSummaryButton_ =
        sf::FloatRect({405.f, 445.f}, {240.f, 58.f});

    drawAcceptButton_ =
        sf::FloatRect({347.f, 420.f}, {160.f, 58.f});

    drawDeclineButton_ =
        sf::FloatRect({543.f, 420.f}, {160.f, 58.f});

    resignConfirmButton_ =
        sf::FloatRect({347.f, 420.f}, {160.f, 58.f});

    resignCancelButton_ =
        sf::FloatRect({543.f, 420.f}, {160.f, 58.f});
}

bool ChessFrontend::isOpen() const {
    return window_.isOpen();
}

void ChessFrontend::setBoard(
    const std::array<char, 64>& board) {
    board_ = board;
}

void ChessFrontend::setPlayerNames(
    const std::string& white,
    const std::string& black) {

    whitePlayerName_ = white;
    blackPlayerName_ = black;
}

void ChessFrontend::setTurnText(
    const std::string& text) {
    turnText_ = text;
}

void ChessFrontend::setStatusText(
    const std::string& text) {
    statusText_ = text;
}

void ChessFrontend::setOfferDrawVisible(bool visible) {
    offerDrawVisible_ = visible;
}

void ChessFrontend::setBoardInputEnabled(bool enabled) {
    boardInputEnabled_ = enabled;

    if (!boardInputEnabled_) {
        selected_.reset();
    }
}

void ChessFrontend::showPromotionDialog(
    const std::string& playerText) {

    promotionPlayerText_ = playerText;
    promotionVisible_ = true;
    drawOfferVisible_ = false;
    resignConfirmVisible_ = false;
    selected_.reset();
}

void ChessFrontend::showResignConfirmDialog() {
    resignConfirmVisible_ = true;
    promotionVisible_ = false;
    drawOfferVisible_ = false;
    selected_.reset();
}

void ChessFrontend::showGameOver(
    const std::string& title,
    const std::string& detail) {

    gameOverTitle_ = title;
    gameOverDetail_ = detail;

    gameOverVisible_ = true;
    promotionVisible_ = false;
    drawOfferVisible_ = false;
    resignConfirmVisible_ = false;
    selected_.reset();
}

void ChessFrontend::showDrawOfferDialog(
    const std::string& message) {

    drawOfferMessage_ = message;
    drawOfferVisible_ = true;
    promotionVisible_ = false;
    resignConfirmVisible_ = false;
    selected_.reset();
}

void ChessFrontend::hideGameOver() {
    gameOverVisible_ = false;
}

bool ChessFrontend::isInside(
    const sf::FloatRect& rect,
    sf::Vector2f point) {

    return rect.contains(point);
}

std::optional<ChessFrontend::Square>
ChessFrontend::mouseToSquare(
    sf::Vector2f mouse) const {

    const float boardSize =
        squareSize_ * BOARD_N;

    if (mouse.x < boardX_ ||
        mouse.y < boardY_ ||
        mouse.x >= boardX_ + boardSize ||
        mouse.y >= boardY_ + boardSize) {
        return std::nullopt;
    }

    Square square;

    square.col =
        static_cast<int>(
            (mouse.x - boardX_) / squareSize_);

    square.row =
        static_cast<int>(
            (mouse.y - boardY_) / squareSize_);

    return square;
}

ChessFrontend::UiEvent
ChessFrontend::pollEvent() {

    while (const auto event = window_.pollEvent()) {

        if (event->is<sf::Event::Closed>()) {
            window_.close();

            UiEvent out;
            out.type = UiEventType::CloseRequested;
            return out;
        }

        const auto* mousePressed =
            event->getIf<
                sf::Event::MouseButtonPressed>();

        if (!mousePressed ||
            mousePressed->button !=
                sf::Mouse::Button::Left) {
            continue;
        }

        const sf::Vector2f mouse{
            static_cast<float>(
                mousePressed->position.x),
            static_cast<float>(
                mousePressed->position.y)
        };

        // Game-over overlay.
        if (gameOverVisible_) {
            if (isInside(
                    closeSummaryButton_,
                    mouse)) {
                gameOverVisible_ = false;
            }

            continue;
        }

        // Incoming draw-offer overlay.
        if (drawOfferVisible_) {

            if (isInside(drawAcceptButton_, mouse)) {
                drawOfferVisible_ = false;

                UiEvent out;
                out.type = UiEventType::DrawAccepted;
                return out;
            }

            if (isInside(drawDeclineButton_, mouse)) {
                drawOfferVisible_ = false;

                UiEvent out;
                out.type = UiEventType::DrawDeclined;
                return out;
            }

            continue;
        }

        // Resign confirmation overlay.
        if (resignConfirmVisible_) {

            if (isInside(resignConfirmButton_, mouse)) {
                resignConfirmVisible_ = false;

                UiEvent out;
                out.type = UiEventType::ResignConfirmed;
                return out;
            }

            if (isInside(resignCancelButton_, mouse)) {
                resignConfirmVisible_ = false;

                UiEvent out;
                out.type = UiEventType::ResignCancelled;
                return out;
            }

            continue;
        }

        // Promotion overlay.
        if (promotionVisible_) {

            const float startX = 333.f;
            const float y = 385.f;
            const float w = 90.f;
            const float h = 70.f;
            const float gap = 12.f;

            const char choices[4] = {
                'Q', 'R', 'B', 'N'
            };

            for (int i = 0; i < 4; ++i) {

                const sf::FloatRect rect(
                    {
                        startX +
                            i * (w + gap),
                        y
                    },
                    {w, h}
                );

                if (isInside(rect, mouse)) {

                    promotionVisible_ = false;

                    UiEvent out;
                    out.type =
                        UiEventType::
                            PromotionSelected;

                    out.promotion =
                        choices[i];

                    return out;
                }
            }

            continue;
        }

        // Offer draw button.
        if (offerDrawVisible_ &&
            isInside(offerDrawButton_, mouse)) {

            selected_.reset();

            UiEvent out;
            out.type =
                UiEventType::DrawOfferRequested;

            return out;
        }

        // Resign button. The GUI reports the request only.
        // The application decides whether to show a confirmation dialog.
        if (isInside(resignButton_, mouse)) {

            selected_.reset();

            UiEvent out;
            out.type = UiEventType::ResignRequested;
            return out;
        }

        // Board input can be temporarily disabled by the application.
        if (!boardInputEnabled_) {
            continue;
        }

        // Board click.
        const auto square =
            mouseToSquare(mouse);

        if (!square.has_value()) {
            continue;
        }

        // First click selects the source.
        if (!selected_.has_value()) {
            selected_ = square;
            continue;
        }

        // Same square = cancel selection.
        if (selected_->row == square->row &&
            selected_->col == square->col) {

            selected_.reset();
            continue;
        }

        // Second click = move request.
        UiEvent out;

        out.type =
            UiEventType::MoveRequested;

        out.from = *selected_;
        out.to = *square;

        selected_.reset();

        return out;
    }

    return {};
}

void ChessFrontend::render() {

    if (!window_.isOpen()) {
        return;
    }

    window_.clear(
        sf::Color(21, 24, 29));

    drawBoard();
    drawSidePanel();

    if (promotionVisible_) {
        drawPromotionDialog();
    }

    if (drawOfferVisible_) {
        drawDrawOfferDialog();
    }

    if (resignConfirmVisible_) {
        drawResignConfirmDialog();
    }

    if (gameOverVisible_) {
        drawGameOver();
    }

    window_.display();
}

void ChessFrontend::drawBoard() {

    const float boardCenterX =
        boardX_ + (squareSize_ * BOARD_N) / 2.f;

    // The board orientation is fixed: Black is shown above the board
    // and White below it.
    drawCenteredText(
        "Black: " + blackPlayerName_,
        23,
        boardCenterX,
        35.f,
        sf::Color(225, 225, 225),
        true
    );

    drawCenteredText(
        "White: " + whitePlayerName_,
        23,
        boardCenterX,
        boardY_ + squareSize_ * BOARD_N + 43.f,
        sf::Color::White,
        true
    );

    for (int row = 0;
         row < BOARD_N;
         ++row) {

        for (int col = 0;
             col < BOARD_N;
             ++col) {

            sf::RectangleShape square(
                {squareSize_, squareSize_});

            square.setPosition({
                boardX_ +
                    col * squareSize_,
                boardY_ +
                    row * squareSize_
            });

            square.setFillColor(
                ((row + col) % 2 == 0)
                    ? lightSquare()
                    : darkSquare()
            );

            if (selected_.has_value() &&
                selected_->row == row &&
                selected_->col == col) {

                square.setFillColor(
                    selectedColor());
            }

            window_.draw(square);

            const char piece =
                board_[row * BOARD_N + col];

            if (piece == '.' ||
                piece == '#') {
                continue;
            }

            if (pieceImagesLoaded_ &&
                pieceTextures_.find(piece) != pieceTextures_.end()) {

                drawPieceImage(
                    piece, row, col);
            }
            else {
                drawFallbackPiece(
                    piece, row, col);
            }
        }
    }

    // Files: a-h.
    for (int col = 0;
         col < BOARD_N;
         ++col) {

        const std::string file(
            1,
            static_cast<char>('a' + col));

        drawText(
            file,
            20,
            {
                boardX_ +
                    col * squareSize_ +
                    squareSize_ / 2.f -
                    6.f,

                boardY_ +
                    BOARD_N * squareSize_ +
                    8.f
            },
            sf::Color(220, 220, 220)
        );
    }

    // Ranks: 8-1.
    for (int row = 0;
         row < BOARD_N;
         ++row) {

        const std::string rank =
            std::to_string(8 - row);

        drawText(
            rank,
            20,
            {
                boardX_ - 27.f,

                boardY_ +
                    row * squareSize_ +
                    squareSize_ / 2.f -
                    12.f
            },
            sf::Color(220, 220, 220)
        );
    }
}

void ChessFrontend::drawPieceImage(
    char piece,
    int row,
    int col) {

    const auto iterator =
        pieceTextures_.find(piece);

    if (iterator == pieceTextures_.end()) {
        drawFallbackPiece(piece, row, col);
        return;
    }

    const sf::Texture& texture =
        iterator->second;

    sf::Sprite sprite(texture);

    const sf::Vector2u textureSize =
        texture.getSize();

    if (textureSize.x == 0 ||
        textureSize.y == 0) {
        return;
    }

    const float targetSize =
        squareSize_ * 0.90f;

    const float scale =
        targetSize /
        static_cast<float>(
            std::max(
                textureSize.x,
                textureSize.y));

    sprite.setScale({scale, scale});

    const float drawnWidth =
        textureSize.x * scale;

    const float drawnHeight =
        textureSize.y * scale;

    sprite.setPosition({
        boardX_ +
            col * squareSize_ +
            (squareSize_ - drawnWidth) /
                2.f,

        boardY_ +
            row * squareSize_ +
            (squareSize_ - drawnHeight) /
                2.f
    });

    window_.draw(sprite);
}

void ChessFrontend::drawFallbackPiece(
    char piece,
    int row,
    int col) {

    sf::CircleShape token(
        squareSize_ * 0.33f);

    token.setOrigin({
        token.getRadius(),
        token.getRadius()
    });

    token.setPosition({
        boardX_ +
            col * squareSize_ +
            squareSize_ / 2.f,

        boardY_ +
            row * squareSize_ +
            squareSize_ / 2.f
    });

    if (isWhitePiece(piece)) {

        token.setFillColor(
            sf::Color(242, 242, 242));

        token.setOutlineColor(
            sf::Color(40, 40, 40));
    }
    else {

        token.setFillColor(
            sf::Color(45, 45, 48));

        token.setOutlineColor(
            sf::Color(232, 232, 232));
    }

    token.setOutlineThickness(2.f);

    window_.draw(token);

    if (!fontLoaded_) {
        return;
    }

    sf::Text text(
        font_,
        pieceLabel(piece),
        28
    );

    text.setStyle(sf::Text::Bold);

    text.setFillColor(
        isWhitePiece(piece)
            ? sf::Color(25, 25, 25)
            : sf::Color::White
    );

    const sf::FloatRect bounds =
        text.getLocalBounds();

    text.setOrigin({
        bounds.position.x +
            bounds.size.x / 2.f,

        bounds.position.y +
            bounds.size.y / 2.f
    });

    text.setPosition({
        token.getPosition().x,
        token.getPosition().y - 1.f
    });

    window_.draw(text);
}

void ChessFrontend::drawSidePanel() {

    sf::RectangleShape panel(
        {290.f, 575.f});

    panel.setPosition(
        {690.f, 80.f});

    panel.setFillColor(panelColor());

    panel.setOutlineColor(
        sf::Color(70, 78, 90));

    panel.setOutlineThickness(1.f);

    window_.draw(panel);

    drawText(
        "CHESS",
        36,
        {740.f, 115.f},
        sf::Color::White,
        true
    );

    drawText(
        "Turn",
        18,
        {720.f, 205.f},
        sf::Color(160, 170, 185)
    );

    drawText(
        turnText_,
        27,
        {720.f, 236.f},
        sf::Color::White,
        true
    );

    drawText(
        "Game status",
        18,
        {720.f, 325.f},
        sf::Color(160, 170, 185)
    );

    const std::string visibleStatus =
        statusText_.empty()
            ? "Game in progress"
            : statusText_;

    const sf::Color statusColor =
        (visibleStatus == "Check" ||
         visibleStatus == "CHECK")
            ? sf::Color(255, 202, 58)
            : sf::Color::White;

    drawText(
        visibleStatus,
        24,
        {720.f, 358.f},
        statusColor,
        true
    );

    if (offerDrawVisible_) {
        sf::RectangleShape offerDraw(
            offerDrawButton_.size);

        offerDraw.setPosition(
            offerDrawButton_.position);

        offerDraw.setFillColor(
            sf::Color(74, 105, 128));

        offerDraw.setOutlineColor(
            sf::Color(130, 158, 178));

        offerDraw.setOutlineThickness(1.f);

        window_.draw(offerDraw);

        drawText(
            "Offer Draw",
            23,
            {
                offerDrawButton_.position.x +
                    57.f,

                offerDrawButton_.position.y +
                    11.f
            },
            sf::Color::White,
            true
        );
    }

    sf::RectangleShape resign(
        resignButton_.size);

    resign.setPosition(
        resignButton_.position);

    resign.setFillColor(
        dangerColor());

    resign.setOutlineColor(
        sf::Color(210, 115, 115));

    resign.setOutlineThickness(1.f);

    window_.draw(resign);

    drawText(
        "Resign",
        25,
        {
            resignButton_.position.x +
                81.f,

            resignButton_.position.y +
                13.f
        },
        sf::Color::White,
        true
    );
}

void ChessFrontend::drawPromotionDialog() {

    sf::RectangleShape shade({
        static_cast<float>(
            window_.getSize().x),

        static_cast<float>(
            window_.getSize().y)
    });

    shade.setFillColor(
        sf::Color(0, 0, 0, 160));

    window_.draw(shade);

    sf::RectangleShape dialog(
        {500.f, 250.f});

    dialog.setPosition(
        {275.f, 255.f});

    dialog.setFillColor(
        sf::Color(38, 43, 51));

    dialog.setOutlineColor(
        sf::Color(110, 120, 135));

    dialog.setOutlineThickness(2.f);

    window_.draw(dialog);

    drawText(
        "Promotion",
        32,
        {430.f, 282.f},
        sf::Color::White,
        true
    );

    drawText(
        promotionPlayerText_ +
            ": choose a piece",
        20,
        {360.f, 332.f},
        sf::Color(210, 215, 225)
    );

    const float startX = 333.f;
    const float y = 385.f;
    const float w = 90.f;
    const float h = 70.f;
    const float gap = 12.f;

    const char choices[4] = {
        'Q', 'R', 'B', 'N'
    };

    for (int i = 0; i < 4; ++i) {

        sf::RectangleShape button(
            {w, h});

        button.setPosition({
            startX + i * (w + gap),
            y
        });

        button.setFillColor(
            buttonColor());

        button.setOutlineColor(
            sf::Color(130, 140, 155));

        button.setOutlineThickness(1.f);

        window_.draw(button);

        // Use the actual white-piece image here.
        const char piece =
            choices[i];

        if (pieceTextures_.find(piece) != pieceTextures_.end()) {

            const sf::Texture& texture =
                pieceTextures_.at(piece);

            sf::Sprite sprite(texture);

            const sf::Vector2u texSize =
                texture.getSize();

            const float target = 57.f;

            const float scale =
                target /
                static_cast<float>(
                    std::max(
                        texSize.x,
                        texSize.y));

            sprite.setScale(
                {scale, scale});

            const float dw =
                texSize.x * scale;

            const float dh =
                texSize.y * scale;

            sprite.setPosition({
                startX +
                    i * (w + gap) +
                    (w - dw) / 2.f,

                y +
                    (h - dh) / 2.f
            });

            window_.draw(sprite);
        }
        else {
            drawText(
                std::string(1, piece),
                30,
                {
                    startX +
                        i * (w + gap) +
                        34.f,
                    y + 16.f
                },
                sf::Color::White,
                true
            );
        }
    }
}


void ChessFrontend::drawDrawOfferDialog() {

    sf::RectangleShape shade({
        static_cast<float>(
            window_.getSize().x),

        static_cast<float>(
            window_.getSize().y)
    });

    shade.setFillColor(
        sf::Color(0, 0, 0, 180));

    window_.draw(shade);

    sf::RectangleShape dialog(
        {520.f, 290.f});

    dialog.setPosition(
        {265.f, 225.f});

    dialog.setFillColor(
        sf::Color(37, 42, 49));

    dialog.setOutlineColor(
        sf::Color(130, 140, 155));

    dialog.setOutlineThickness(2.f);

    window_.draw(dialog);

    drawText(
        "Draw Offer",
        32,
        {430.f, 260.f},
        sf::Color::White,
        true
    );

    const std::string message =
        drawOfferMessage_.empty()
            ? "Your opponent offers a draw."
            : drawOfferMessage_;

    drawText(
        message,
        21,
        {330.f, 330.f},
        sf::Color(215, 220, 228)
    );

    sf::RectangleShape accept(
        drawAcceptButton_.size);

    accept.setPosition(
        drawAcceptButton_.position);

    accept.setFillColor(
        sf::Color(69, 128, 81));

    window_.draw(accept);

    drawText(
        "Accept",
        23,
        {
            drawAcceptButton_.position.x + 42.f,
            drawAcceptButton_.position.y + 13.f
        },
        sf::Color::White,
        true
    );

    sf::RectangleShape decline(
        drawDeclineButton_.size);

    decline.setPosition(
        drawDeclineButton_.position);

    decline.setFillColor(
        sf::Color(130, 72, 72));

    window_.draw(decline);

    drawText(
        "Decline",
        23,
        {
            drawDeclineButton_.position.x + 38.f,
            drawDeclineButton_.position.y + 13.f
        },
        sf::Color::White,
        true
    );
}

void ChessFrontend::drawResignConfirmDialog() {

    sf::RectangleShape shade({
        static_cast<float>(window_.getSize().x),
        static_cast<float>(window_.getSize().y)
    });

    shade.setFillColor(sf::Color(0, 0, 0, 180));
    window_.draw(shade);

    sf::RectangleShape dialog({520.f, 290.f});
    dialog.setPosition({265.f, 225.f});
    dialog.setFillColor(sf::Color(37, 42, 49));
    dialog.setOutlineColor(sf::Color(130, 140, 155));
    dialog.setOutlineThickness(2.f);
    window_.draw(dialog);

    drawText(
        "Confirm Resignation",
        32,
        {355.f, 260.f},
        sf::Color::White,
        true
    );

    drawText(
        "Are you sure you want to resign?",
        21,
        {337.f, 330.f},
        sf::Color(215, 220, 228)
    );

    sf::RectangleShape confirm(resignConfirmButton_.size);
    confirm.setPosition(resignConfirmButton_.position);
    confirm.setFillColor(dangerColor());
    window_.draw(confirm);

    drawText(
        "Confirm",
        23,
        {
            resignConfirmButton_.position.x + 34.f,
            resignConfirmButton_.position.y + 13.f
        },
        sf::Color::White,
        true
    );

    sf::RectangleShape cancel(resignCancelButton_.size);
    cancel.setPosition(resignCancelButton_.position);
    cancel.setFillColor(buttonColor());
    window_.draw(cancel);

    drawText(
        "No",
        23,
        {
            resignCancelButton_.position.x + 64.f,
            resignCancelButton_.position.y + 13.f
        },
        sf::Color::White,
        true
    );
}

void ChessFrontend::drawGameOver() {

    sf::RectangleShape shade({
        static_cast<float>(
            window_.getSize().x),

        static_cast<float>(
            window_.getSize().y)
    });

    shade.setFillColor(
        sf::Color(0, 0, 0, 185));

    window_.draw(shade);

    sf::RectangleShape dialog(
        {560.f, 320.f});

    dialog.setPosition(
        {245.f, 200.f});

    dialog.setFillColor(
        sf::Color(37, 42, 49));

    dialog.setOutlineColor(
        sf::Color(130, 140, 155));

    dialog.setOutlineThickness(2.f);

    window_.draw(dialog);

    drawText(
        "Game Over",
        24,
        {452.f, 235.f},
        sf::Color(175, 185, 198)
    );

    drawText(
        gameOverTitle_,
        38,
        {350.f, 292.f},
        sf::Color::White,
        true
    );

    if (!gameOverDetail_.empty()) {

        drawText(
            gameOverDetail_,
            23,
            {400.f, 355.f},
            sf::Color(215, 220, 228)
        );
    }

    sf::RectangleShape close(
        closeSummaryButton_.size);

    close.setPosition(
        closeSummaryButton_.position);

    close.setFillColor(
        buttonColor());

    window_.draw(close);

    drawText(
        "Close",
        24,
        {
            closeSummaryButton_.position.x +
                86.f,

            closeSummaryButton_.position.y +
                14.f
        },
        sf::Color::White,
        true
    );
}

void ChessFrontend::drawCenteredText(
    const std::string& text,
    unsigned size,
    float centerX,
    float y,
    sf::Color color,
    bool bold) {

    if (!fontLoaded_) {
        return;
    }

    sf::Text drawable(
        font_,
        text,
        size
    );

    drawable.setFillColor(color);

    if (bold) {
        drawable.setStyle(sf::Text::Bold);
    }

    const sf::FloatRect bounds =
        drawable.getLocalBounds();

    drawable.setPosition({
        centerX - bounds.size.x / 2.f - bounds.position.x,
        y
    });

    window_.draw(drawable);
}

void ChessFrontend::drawText(
    const std::string& text,
    unsigned size,
    sf::Vector2f position,
    sf::Color color,
    bool bold) {

    if (!fontLoaded_) {
        return;
    }

    sf::Text drawable(
        font_,
        text,
        size
    );

    drawable.setFillColor(color);
    drawable.setPosition(position);

    if (bold) {
        drawable.setStyle(
            sf::Text::Bold);
    }

    window_.draw(drawable);
}

std::string
ChessFrontend::pieceLabel(char piece) {

    const char upper =
        static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(
                    piece)));

    switch (upper) {
        case 'K': return "K";
        case 'Q': return "Q";
        case 'R': return "R";
        case 'B': return "B";
        case 'N': return "N";
        case 'P': return "P";
        default:  return "?";
    }
}

bool ChessFrontend::isWhitePiece(
    char piece) {

    return std::isupper(
        static_cast<unsigned char>(
            piece)) != 0;
}

bool ChessFrontend::loadPieceImages() {

    const std::map<char, std::string> files = {
        {'K', "white_king.png"},
        {'Q', "white_queen.png"},
        {'R', "white_rook.png"},
        {'B', "white_bishop.png"},
        {'N', "white_knight.png"},
        {'P', "white_pawn.png"},

        {'k', "black_king.png"},
        {'q', "black_queen.png"},
        {'r', "black_rook.png"},
        {'b', "black_bishop.png"},
        {'n', "black_knight.png"},
        {'p', "black_pawn.png"}
    };

    bool allLoaded = true;

    for (const auto& [piece, filename] : files) {

        const auto path =
            findAsset(filename);

        if (path.empty()) {
            allLoaded = false;
            continue;
        }

        sf::Texture texture;

        if (!texture.loadFromFile(path)) {
            allLoaded = false;
            continue;
        }

        texture.setSmooth(true);

        pieceTextures_.emplace(
            piece,
            std::move(texture)
        );
    }

    return allLoaded &&
           pieceTextures_.size() == 12;
}

bool ChessFrontend::loadSystemFont() {

    const std::vector<std::string>
        candidates = {

        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",

        "/usr/share/fonts/truetype/"
        "dejavu/DejaVuSans.ttf",

        "/System/Library/Fonts/"
        "Supplemental/Arial.ttf"
    };

    for (const auto& path : candidates) {

        if (std::filesystem::exists(path) &&
            font_.openFromFile(path)) {

            return true;
        }
    }

    return false;
}
