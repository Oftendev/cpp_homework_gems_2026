#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "Cell.hpp"

class GemsGame;

class GameRenderer {
   public:
    GameRenderer(int cellSize);
    void draw(sf::RenderWindow& window, const GemsGame& game);

   private:
    int cellSize;
    sf::Font font;
    void drawCell(sf::RenderWindow& window, const std::shared_ptr<Cell>& cell,
                  int x, int y, bool isSelected);
    void drawPauseOverlay(sf::RenderWindow& window, int width, int height);
};