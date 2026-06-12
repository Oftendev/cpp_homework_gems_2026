#pragma once
#include <memory>
#include <vector>

#include "Cell.hpp"

class BonusManager {
   public:
    static constexpr int BONUS_PROBABILITY = 5;
    BonusManager() = default;

    // Генерация бонуса на поле
    void placeBonus(const sf::Vector2i& pos, Color bonusColor,
                    std::vector<std::vector<std::shared_ptr<Cell>>>& board,
                    int width, int height,
                    std::vector<std::pair<sf::Vector2i, std::shared_ptr<Cell>>>&
                        bonusesToActivate);

    // Активация одного бонуса
    void executeBonus(const sf::Vector2i& pos, std::shared_ptr<Cell> bonus,
                      std::vector<std::vector<std::shared_ptr<Cell>>>& board,
                      int width, int height);
};