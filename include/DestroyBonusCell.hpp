#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

#include "Cell.hpp"

class DestroyBonusCell : public BonusCell {
   public:
    DestroyBonusCell();

    virtual std::string getSymbol() const override { return "B"; }
    // Активирует бонус: уничтожает себя и до 4 случайных клеток на поле
    void execute(
        const sf::Vector2i& pos,
        std::vector<std::vector<std::shared_ptr<Cell>>>& board) override;

   private:
    // Функция, возвразающая список координат бомб-бонусов, которые были
    // сгенерированы
    std::vector<sf::Vector2i> getTargetCells(
        int width, int height,
        const std::vector<std::vector<std::shared_ptr<Cell>>>& board) const;
};