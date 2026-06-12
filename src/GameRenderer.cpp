#include "GameRenderer.hpp"

#include "GemsGame.hpp"

GameRenderer::GameRenderer(int cellSize) : cellSize(cellSize) {
    font.loadFromFile(PATH_TO_DATA "arial.ttf");
}

void GameRenderer::draw(sf::RenderWindow& window, const GemsGame& game) {
    // Получаем данные из класса GemsGame
    const auto& board = game.getBoard();
    int height = game.getHeight();
    int width = game.getWidth();

    // Отрисовываем все клетки
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (board[y][x]) {
                bool isSelected = game.isCellSelected(x, y);
                drawCell(window, board[y][x], x, y, isSelected);
            }
        }
    }

    // Если экран паузы:
    if (game.isPausedState()) {
        drawPauseOverlay(window, width, height);
    }
}

void GameRenderer::drawCell(sf::RenderWindow& window,
                            const std::shared_ptr<Cell>& cell, int x, int y,
                            bool isSelected) {
    // Создаём прямоугольники так, чтобы между ними было по 2 единицы расстояния
    sf::RectangleShape rect(sf::Vector2f(cellSize - 2, cellSize - 2));
    rect.setPosition(x * cellSize + 1, y * cellSize + 1);
    rect.setFillColor(cell->getFillColor());
    rect.setOutlineColor(sf::Color::Black);
    rect.setOutlineThickness(-5);  // Граница внутрь
    window.draw(rect);
    // Если первая выбранная клетка есть, и это - она, то затемнить
    if (isSelected) {
        sf::RectangleShape selectMask(rect.getSize());
        selectMask.setPosition(rect.getPosition());
        // Чёрный полупрозрачный прямоугольник
        selectMask.setFillColor(sf::Color(0, 0, 0, 100));
        window.draw(selectMask);
    }
    // Для бонусов нарисуем символы
    std::string symbol = cell->getSymbol();
    if (!symbol.empty()) {
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(cellSize / 2.0f);
        text.setString(symbol);
        text.setFillColor(sf::Color::Black);
        sf::FloatRect textBounds = text.getLocalBounds();
        // Ставим origin по центру boundingBox
        text.setOrigin(textBounds.left + textBounds.width / 2,
                       textBounds.top + textBounds.height / 2);

        text.setPosition(x * cellSize + cellSize / 2,
                         y * cellSize + cellSize / 2);
        window.draw(text);
    }
}

void GameRenderer::drawPauseOverlay(sf::RenderWindow& window, int width,
                                    int height) {
    sf::RectangleShape pauseRect(
        sf::Vector2f(width * cellSize, height * cellSize));
    pauseRect.setPosition(0.0f, 0.0f);
    pauseRect.setFillColor(sf::Color(0, 0, 0, 127));
    window.draw(pauseRect);

    sf::Text pauseText;
    pauseText.setFont(font);
    pauseText.setCharacterSize(cellSize / 2.0f);
    pauseText.setString("Paused");
    pauseText.setFillColor(sf::Color::White);

    sf::FloatRect textBounds = pauseText.getLocalBounds();
    // Ставим origin по центру boundingBox
    pauseText.setOrigin(textBounds.left + textBounds.width / 2,
                        textBounds.top + textBounds.height / 2);
    // Устанавливаем по центру
    pauseText.setPosition((width * cellSize) / 2.0f,
                          (height * cellSize) / 2.0f);

    window.draw(pauseText);
}