#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

#include "Cell.hpp"

// ColorBonusCell - класс-наследник от BonusCell
class ColorBonusCell : public BonusCell {
   public:
    static constexpr int COLORBONUS_RADIUS = 3;
    ColorBonusCell(Color bonusColor);

    std::string getSymbol() const override { return "C"; }

    // Активирует бонус: перекрашивает себя и до 2 случайных клеток из целей
    void execute(
        const sf::Vector2i& pos,
        std::vector<std::vector<std::shared_ptr<Cell>>>& board) override;

   private:
    // Функция, возвразающая список координат цветных бонусов, которые были
    // сгенерированы
    std::vector<sf::Vector2i> getTargetCells(
        const sf::Vector2i& pos, int width, int height,
        const std::vector<std::vector<std::shared_ptr<Cell>>>& board) const;
};